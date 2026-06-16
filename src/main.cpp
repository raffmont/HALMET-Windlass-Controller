// HALMET Windlass Controller firmware.
//
// This keeps the HALMET example firmware programming model: SensESP owns
// onboarding, configuration, Signal K outputs/listeners, and the cooperative
// event loop. Windlass-specific safety, event detection, and state handling are
// adapted from raffmont/halmet-windlass-controller-new.

#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <N2kMessages.h>
#include <NMEA2000_esp32.h>
#include <Preferences.h>
#include <climits>
#include <cmath>
#include <memory>

#include "anchor_watch.h"
#include "config.h"
#include "geo_utils.h"
#include "gps_manager.h"
#include "halmet_const.h"
#include "halmet_digital.h"
#include "halmet_display.h"
#include "halmet_serial.h"
#include "sensesp/net/http_server.h"
#include "sensesp/net/networking.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/signalk/signalk_listener.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/signalk/signalk_value_listener.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/transforms/lambda_transform.h"
#include "sensesp/ui/config_item.h"
#include "sensesp_app_builder.h"
#include "windlass_config.h"
#include "windlass_state.h"

#define BUILDER_CLASS SensESPAppBuilder

using namespace sensesp;
using namespace halmet;

namespace {

TwoWire* i2c;
Adafruit_SSD1306* display;
tNMEA2000* nmea2000;
Preferences prefs;
ChainCounterState state;
std::shared_ptr<WindlassRuntimeConfig> runtime_config;
std::unique_ptr<GpsManager> gps_manager;
std::unique_ptr<AnchorWatch> anchor_watch;
portMUX_TYPE pulse_mux = portMUX_INITIALIZER_UNLOCKED;

volatile uint32_t last_chain_pulse_isr_ms = 0;
volatile uint32_t chain_pulse_debounce_ms = DEFAULT_CHAIN_PULSE_DEBOUNCE_MS;

SKOutputFloat* rode_length_output = nullptr;
SKOutputFloat* rode_speed_output = nullptr;
SKOutputString* direction_output = nullptr;
SKOutputString* state_output = nullptr;
SKOutputString* command_status_output = nullptr;
SKOutputInt* pulse_output = nullptr;
SKOutputFloat* meters_per_pulse_output = nullptr;
SKOutputInt* fault_output = nullptr;
SKOutputString* mode_output = nullptr;
SKOutputBool* free_fall_output = nullptr;
SKOutputBool* anchor_detected_output = nullptr;
SKOutputBool* seafloor_detected_output = nullptr;
SKOutputString* event_output = nullptr;
SKOutputString* notification_output = nullptr;
SKOutputBool* anchor_watch_enabled_output = nullptr;
SKOutputBool* anchor_watch_auto_arm_output = nullptr;
SKOutputString* anchor_watch_state_output = nullptr;
SKOutputFloat* anchor_watch_radius_output = nullptr;
SKOutputFloat* anchor_watch_distance_output = nullptr;
SKOutputFloat* anchor_watch_margin_output = nullptr;
SKOutputFloat* anchor_watch_lat_output = nullptr;
SKOutputFloat* anchor_watch_lon_output = nullptr;
SKOutputFloat* anchor_watch_rode_at_arm_output = nullptr;
SKOutputBool* anchor_watch_gnss_present_output = nullptr;
SKOutputString* anchor_watch_gnss_interface_output = nullptr;
SKOutputBool* anchor_watch_gnss_fix_valid_output = nullptr;
SKOutputFloat* anchor_watch_gnss_hdop_output = nullptr;
SKOutputInt* anchor_watch_gnss_sats_output = nullptr;
SKOutputFloat* anchor_watch_gnss_lat_output = nullptr;
SKOutputFloat* anchor_watch_gnss_lon_output = nullptr;
SKOutputString* anchor_watch_notification_output = nullptr;

const uint8_t kWindlassUpRelayPin = WINDLASS_UP_RELAY_PIN;
const uint8_t kWindlassDownRelayPin = WINDLASS_DOWN_RELAY_PIN;

const adsGain_t kADS1115Gain = GAIN_ONE;

void setFault(uint32_t fault) { state.fault_flags |= fault; }
void clearFault(uint32_t fault) { state.fault_flags &= ~fault; }

int64_t getPulseCount() {
  portENTER_CRITICAL(&pulse_mux);
  int64_t pulses = state.pulses_total;
  portEXIT_CRITICAL(&pulse_mux);
  return pulses;
}

void setPulseCount(int64_t pulses) {
  portENTER_CRITICAL(&pulse_mux);
  state.pulses_total = pulses;
  portEXIT_CRITICAL(&pulse_mux);
  prefs.putLong64("pulses", pulses);
}

void IRAM_ATTR onChainPulse() {
  const uint32_t now = millis();
  if (now - last_chain_pulse_isr_ms < chain_pulse_debounce_ms) return;
  last_chain_pulse_isr_ms = now;

  portENTER_CRITICAL_ISR(&pulse_mux);
  if (state.direction == WindlassDirection::Retrieving) {
    state.pulses_total--;
  } else {
    state.pulses_total++;
  }
  state.last_pulse_ms = now;
  portEXIT_CRITICAL_ISR(&pulse_mux);
}

bool relayActiveHigh() {
  return runtime_config ? runtime_config->relay_active_high
                        : (DEFAULT_RELAY_ACTIVE_HIGH != 0);
}

void writeRelay(uint8_t pin, bool on) {
  const bool active_high = relayActiveHigh();
  const uint8_t active_level = active_high ? HIGH : LOW;
  const uint8_t inactive_level = active_high ? LOW : HIGH;
  digitalWrite(pin, on ? active_level : inactive_level);
}

void stopWindlass(uint32_t fault = FaultNone) {
  writeRelay(kWindlassUpRelayPin, false);
  writeRelay(kWindlassDownRelayPin, false);
  state.active_command = WindlassCommand::Stop;
  if (!state.manual_up && !state.manual_down) {
    state.direction = WindlassDirection::Stopped;
  }
  if (fault != FaultNone) setFault(fault);
}

bool commandWouldViolateLimits(WindlassCommand command) {
  const double min_length = runtime_config ? runtime_config->min_safe_length_m
                                           : DEFAULT_MIN_SAFE_LENGTH_M;
  return command == WindlassCommand::Up && state.rode_length_m <= min_length;
}

bool applyCommand(WindlassCommand command, bool refresh_only = false) {
  const uint32_t now = millis();
  if (command == WindlassCommand::Stop) {
    stopWindlass();
    state.command_refreshed_ms = now;
    return true;
  }
  if (commandWouldViolateLimits(command)) {
    stopWindlass(FaultNearZeroLimit);
    return false;
  }
  if (state.manual_up || state.manual_down) {
    stopWindlass(FaultMutuallyExclusiveCommand);
    return false;
  }

  writeRelay(kWindlassUpRelayPin, false);
  writeRelay(kWindlassDownRelayPin, false);
  delay(20);

  if (command == WindlassCommand::Up) {
    writeRelay(kWindlassUpRelayPin, true);
    state.direction = WindlassDirection::Retrieving;
  } else {
    writeRelay(kWindlassDownRelayPin, true);
    state.direction = WindlassDirection::Deploying;
  }

  if (!refresh_only || state.active_command != command) {
    state.command_started_ms = now;
  }
  state.command_refreshed_ms = now;
  state.active_command = command;
  return true;
}

void updateManualSense() {
#if SENSE_ACTIVE_HIGH
  const bool up = digitalRead(kDigitalInputPin2) == HIGH;
  const bool down = digitalRead(kDigitalInputPin3) == HIGH;
#else
  const bool up = digitalRead(kDigitalInputPin2) == LOW;
  const bool down = digitalRead(kDigitalInputPin3) == LOW;
#endif
  state.manual_up = up;
  state.manual_down = down;
  if (up && down) {
    state.direction = WindlassDirection::Unknown;
    stopWindlass(FaultMutuallyExclusiveCommand);
  } else if (up) {
    state.direction = WindlassDirection::Retrieving;
  } else if (down) {
    state.direction = WindlassDirection::Deploying;
  } else if (state.active_command == WindlassCommand::Stop) {
    state.direction = WindlassDirection::Stopped;
  }
}

bool isDeployingCommandedOrManual() {
  return state.active_command == WindlassCommand::Down || state.manual_down;
}

bool isRetrievingCommandedOrManual() {
  return state.active_command == WindlassCommand::Up || state.manual_up;
}

void markEvent(WindlassEvent event) {
  state.last_event = event;
  state.last_event_ms = millis();
  state.last_event_pulses = getPulseCount();
  state.event_dirty = true;
  debugI("Windlass event: %s length=%.2f m speed=%.2f m/s",
         event_to_string(event), state.rode_length_m, state.rode_speed_m_s);
}

void updateDerivedValues() {
  const int64_t pulses = getPulseCount();
  const double rode_length = pulses * state.meters_per_pulse;
  state.rode_length_m = rode_length > 0.0 ? rode_length : 0.0;

  const uint32_t now = millis();
  if (state.last_speed_ms == 0) {
    state.last_speed_ms = now;
    state.last_speed_pulses = pulses;
    return;
  }

  const uint32_t dt = now - state.last_speed_ms;
  if (dt >= SPEED_WINDOW_MS) {
    const int64_t dp = pulses - state.last_speed_pulses;
    state.rode_speed_m_s = (dp * state.meters_per_pulse) / (dt / 1000.0);
    if (state.direction == WindlassDirection::Retrieving) {
      state.rode_speed_m_s = -fabs(state.rode_speed_m_s);
    }
    if (state.direction == WindlassDirection::Deploying) {
      state.rode_speed_m_s = fabs(state.rode_speed_m_s);
    }
    state.last_speed_ms = now;
    state.last_speed_pulses = pulses;
  }
}

void updateEventDetection() {
  if (!runtime_config) return;
  const uint32_t now = millis();
  const int64_t pulses = getPulseCount();
  const bool has_recent_pulse =
      state.last_pulse_ms != 0 &&
      (now - state.last_pulse_ms) <= runtime_config->seafloor_no_pulse_ms;
  const bool deploying_motion =
      state.rode_speed_m_s > 0.01 ||
      (has_recent_pulse && !isRetrievingCommandedOrManual());

  if (state.rode_length_m <= runtime_config->min_safe_length_m &&
      !deploying_motion && !isDeployingCommandedOrManual()) {
    state.anchor_detected = false;
    state.seafloor_detected = false;
    state.anchor_alarm_suggested = false;
    state.free_fall_latched = false;
    state.free_fall_active = false;
    state.moving_without_command_started_ms = 0;
  }

  if (!state.anchor_detected &&
      state.rode_length_m >= runtime_config->anchor_detected_length_m &&
      deploying_motion) {
    state.anchor_detected = true;
    state.deployment_started_ms = now;
    state.deployment_started_pulses = pulses;
    markEvent(WindlassEvent::AnchorDetected);
  }

  const bool uncommanded_deploy_pulses =
      !isDeployingCommandedOrManual() && !isRetrievingCommandedOrManual() &&
      deploying_motion;
  if (runtime_config->free_fall_detection_enabled &&
      uncommanded_deploy_pulses) {
    if (state.moving_without_command_started_ms == 0) {
      state.moving_without_command_started_ms = now;
    }
    const int64_t freefall_pulses = pulses - state.deployment_started_pulses;
    if (!state.free_fall_latched &&
        freefall_pulses >=
            static_cast<int64_t>(runtime_config->free_fall_min_pulses) &&
        state.rode_speed_m_s >= runtime_config->free_fall_min_speed_m_s) {
      state.free_fall_active = true;
      state.free_fall_latched = true;
      state.direction = WindlassDirection::Deploying;
      state.mode = WindlassMode::FreeFall;
      setFault(FaultFreeFallDetected);
      markEvent(WindlassEvent::FreeFallDetected);
    }
  } else if (!has_recent_pulse) {
    state.free_fall_active = false;
    state.moving_without_command_started_ms = 0;
  }

  const bool deployment_context =
      state.anchor_detected &&
      (isDeployingCommandedOrManual() || state.free_fall_latched ||
       state.direction == WindlassDirection::Deploying);
  const bool stopped_after_deploy =
      deployment_context &&
      state.rode_length_m >= runtime_config->seafloor_min_length_m &&
      state.last_pulse_ms != 0 &&
      (now - state.last_pulse_ms) >= runtime_config->seafloor_no_pulse_ms &&
      state.active_command != WindlassCommand::Up;

  if (runtime_config->seafloor_detection_enabled &&
      !state.seafloor_detected && stopped_after_deploy) {
    state.seafloor_detected = true;
    state.free_fall_active = false;
    state.mode = WindlassMode::Seafloor;
    markEvent(WindlassEvent::SeafloorDetected);
  }

  if (state.seafloor_detected && !state.anchor_alarm_suggested) {
    state.anchor_alarm_suggested = true;
    markEvent(WindlassEvent::AnchorAlarmSuggested);
  }

  if (state.fault_flags != FaultNone &&
      state.active_command != WindlassCommand::Stop) {
    state.mode = WindlassMode::Fault;
  } else if (state.free_fall_active) {
    state.mode = WindlassMode::FreeFall;
  } else if (state.seafloor_detected) {
    state.mode = WindlassMode::Seafloor;
  } else if (isRetrievingCommandedOrManual()) {
    state.mode = WindlassMode::Retrieving;
  } else if (isDeployingCommandedOrManual() ||
             state.direction == WindlassDirection::Deploying) {
    state.mode = WindlassMode::Deploying;
  } else {
    state.mode = WindlassMode::Idle;
  }
}

void enforceSafety() {
  const uint32_t now = millis();
  if (state.active_command == WindlassCommand::Stop) return;

  const uint32_t command_deadman =
      runtime_config ? runtime_config->command_deadman_ms
                     : DEFAULT_COMMAND_DEADMAN_MS;
  const uint32_t stall_detect =
      runtime_config ? runtime_config->stall_detect_ms : DEFAULT_STALL_DETECT_MS;

  if (now - state.command_refreshed_ms > command_deadman) {
    stopWindlass(FaultCommandTimeout);
  } else if (now - state.command_started_ms > MAX_RELAY_ON_MS) {
    stopWindlass(FaultCommandTimeout);
  } else if (state.last_pulse_ms != 0 && now - state.last_pulse_ms > stall_detect) {
    stopWindlass(FaultNoPulsesWhileRunning);
  } else if (commandWouldViolateLimits(state.active_command)) {
    stopWindlass(FaultNearZeroLimit);
  }
}

void applyRuntimeConfig() {
  if (!runtime_config) return;
  if (runtime_config->meters_per_pulse >= 0.01 &&
      runtime_config->meters_per_pulse <= 2.0) {
    state.meters_per_pulse = runtime_config->meters_per_pulse;
  }
  if (runtime_config->chain_pulse_debounce_ms >= 5 &&
      runtime_config->chain_pulse_debounce_ms <= 1000) {
    chain_pulse_debounce_ms = runtime_config->chain_pulse_debounce_ms;
  }
}

void saveRuntimeCountersThrottled() {
  static uint32_t last_save_ms = 0;
  static int64_t last_saved_pulses = LLONG_MIN;
  const uint32_t now = millis();
  const int64_t pulses = getPulseCount();
  if (pulses != last_saved_pulses && now - last_save_ms > 5000) {
    prefs.putLong64("pulses", pulses);
    last_saved_pulses = pulses;
    last_save_ms = now;
  }
}

void publishState() {
  rode_length_output->emit(state.rode_length_m);
  rode_speed_output->emit(state.rode_speed_m_s);
  direction_output->emit(direction_to_string(state.direction));
  state_output->emit(state.active_command == WindlassCommand::Stop
                         ? direction_to_string(state.direction)
                         : command_to_string(state.active_command));
  command_status_output->emit(command_to_string(state.active_command));
  pulse_output->emit(static_cast<int>(getPulseCount()));
  meters_per_pulse_output->emit(state.meters_per_pulse);
  fault_output->emit(static_cast<int>(state.fault_flags));
  mode_output->emit(mode_to_string(state.mode));
  free_fall_output->emit(state.free_fall_latched);
  anchor_detected_output->emit(state.anchor_detected);
  seafloor_detected_output->emit(state.seafloor_detected);
  event_output->emit(event_to_string(state.last_event));

  if (state.event_dirty) {
    if (state.last_event == WindlassEvent::SeafloorDetected ||
        state.last_event == WindlassEvent::AnchorAlarmSuggested) {
      notification_output->emit(
          "Anchor reached seafloor; consider setting or updating the anchor "
          "alarm.");
    } else if (state.last_event == WindlassEvent::FreeFallDetected) {
      notification_output->emit(
          "Windlass free-fall detected: chain is moving without a DOWN "
          "command.");
    } else {
      notification_output->emit(event_to_string(state.last_event));
    }
    state.event_dirty = false;
  }

  if (anchor_watch && gps_manager) {
    const auto& snapshot = anchor_watch->snapshot();
    const auto& fix = gps_manager->fix();
    anchor_watch_enabled_output->emit(runtime_config->anchor_watch.enabled);
    anchor_watch_auto_arm_output->emit(runtime_config->anchor_watch.auto_arm);
    anchor_watch_state_output->emit(anchor_watch->stateString());
    anchor_watch_radius_output->emit(snapshot.radius_m);
    anchor_watch_distance_output->emit(snapshot.distance_m);
    anchor_watch_margin_output->emit(snapshot.margin_m);
    anchor_watch_lat_output->emit(snapshot.anchor_latitude);
    anchor_watch_lon_output->emit(snapshot.anchor_longitude);
    anchor_watch_rode_at_arm_output->emit(snapshot.rode_length_at_arm_m);
    anchor_watch_gnss_present_output->emit(fix.present);
    anchor_watch_gnss_interface_output->emit(fix.interface);
    anchor_watch_gnss_fix_valid_output->emit(gps_manager->hasUsableFix());
    anchor_watch_gnss_hdop_output->emit(fix.hdop);
    anchor_watch_gnss_sats_output->emit(fix.satellites);
    anchor_watch_gnss_lat_output->emit(fix.latitude);
    anchor_watch_gnss_lon_output->emit(fix.longitude);
    if (snapshot.notification_message.length() > 0) {
      String payload = "{\"path\":\"" + snapshot.notification_path +
                       "\",\"value\":{\"state\":\"" +
                       snapshot.notification_state +
                       "\",\"method\":[\"visual\",\"sound\"],\"message\":\"" +
                       snapshot.notification_message + "\"}}";
      anchor_watch_notification_output->emit(payload);
    }
  }

  if (display) {
    ClearRow(display, 2);
    ClearRow(display, 3);
    ClearRow(display, 4);
    PrintValue(display, 2, "Mode:", mode_to_string(state.mode));
    PrintValue(display, 3, "Chain (m):", state.rode_length_m);
    PrintValue(display, 4, "Speed:", state.rode_speed_m_s);
  }
}

void sendWindlassOperatingStatus() {
  tN2kMsg msg;
  msg.SetPGN(128777UL);
  msg.Priority = 3;
  msg.AddByte(DEVICE_INSTANCE);
  msg.AddByte(static_cast<uint8_t>(state.direction));
  msg.Add4ByteDouble(state.rode_length_m, 0.01);
  msg.Add2ByteDouble(fabs(state.rode_speed_m_s), 0.01);
  msg.AddByte(static_cast<uint8_t>(state.active_command));
  msg.Add4ByteUInt(state.fault_flags);
  nmea2000->SendMsg(msg);
}

void sendWindlassControlStatus() {
  tN2kMsg msg;
  msg.SetPGN(128776UL);
  msg.Priority = 3;
  msg.AddByte(DEVICE_INSTANCE);
  msg.AddByte(static_cast<uint8_t>(state.active_command));
  msg.AddByte(static_cast<uint8_t>(state.manual_up ? 1 : 0));
  msg.AddByte(static_cast<uint8_t>(state.manual_down ? 1 : 0));
  msg.Add2ByteUDouble(MAX_RELAY_ON_MS / 1000.0, 0.1);
  msg.Add4ByteUInt(state.fault_flags);
  nmea2000->SendMsg(msg);
}

void sendWindlassMonitoringStatus() {
  tN2kMsg msg;
  msg.SetPGN(128778UL);
  msg.Priority = 3;
  msg.AddByte(DEVICE_INSTANCE);
  msg.Add4ByteUDouble(state.rode_length_m, 0.01);
  msg.Add2ByteDouble(state.rode_speed_m_s, 0.01);
  msg.Add4ByteDouble(static_cast<double>(getPulseCount()), 1.0);
  msg.Add4ByteUInt(state.fault_flags);
  nmea2000->SendMsg(msg);
}

void sendGpsRapidUpdates() {
  if (!gps_manager || !runtime_config->anchor_watch.n2k_publish_gnss ||
      !gps_manager->hasUsableFix()) {
    return;
  }
  static uint8_t sid = 0;
  const auto& fix = gps_manager->fix();

  tN2kMsg position_msg;
  SetN2kLatLonRapid(position_msg, fix.latitude, fix.longitude);
  nmea2000->SendMsg(position_msg);

  tN2kMsg cog_sog_msg;
  SetN2kCOGSOGRapid(cog_sog_msg, sid++, N2khr_true,
                    degreesToRadians(fix.cog_deg), fix.sog_m_s);
  nmea2000->SendMsg(cog_sog_msg);
}

void publishN2K() {
  if (!state.n2k_started) return;
  sendWindlassOperatingStatus();
  sendWindlassControlStatus();
  sendWindlassMonitoringStatus();
  sendGpsRapidUpdates();
}

void updateController() {
  applyRuntimeConfig();
  if (gps_manager) gps_manager->update();
  updateManualSense();
  updateDerivedValues();
  updateEventDetection();
  enforceSafety();
  if (anchor_watch) anchor_watch->update(state);
  saveRuntimeCountersThrottled();
}

void setupRuntimeConfig() {
  runtime_config = std::make_shared<WindlassRuntimeConfig>();
  auto config_item = ConfigItem(runtime_config);
  config_item
      ->set_title("HALMET Windlass Controller")
      ->set_description(
          "Configure meters-per-pulse calibration, debouncing, safety timing, "
          "event detection, relay polarity, and Signal K output paths.")
      ->set_sort_order(100);
  config_item->load();

  state.meters_per_pulse = runtime_config->meters_per_pulse;
  chain_pulse_debounce_ms = runtime_config->chain_pulse_debounce_ms;
}

SKMetadata* makeMetadata(const char* units, const char* description,
                         const char* display_name,
                         const char* short_name) {
  auto metadata = new SKMetadata();
  metadata->units_ = units;
  metadata->description_ = description;
  metadata->display_name_ = display_name;
  metadata->short_name_ = short_name;
  return metadata;
}

void setupSignalK() {
  rode_length_output = new SKOutputFloat(
      runtime_config->sk_rode_length, "/Windlass/Signal K/Rode Length",
      makeMetadata("m", "Anchor rode deployed", "Rode Deployed", "Rode"));
  rode_speed_output = new SKOutputFloat(
      runtime_config->sk_rode_speed, "/Windlass/Signal K/Rode Speed",
      makeMetadata("m/s", "Windlass rode speed", "Windlass Speed", "Speed"));
  direction_output = new SKOutputString(runtime_config->sk_direction,
                                        "/Windlass/Signal K/Direction");
  state_output = new SKOutputString(runtime_config->sk_state,
                                    "/Windlass/Signal K/State");
  command_status_output = new SKOutputString(
      runtime_config->sk_command_status, "/Windlass/Signal K/Command Status");
  pulse_output = new SKOutputInt(runtime_config->sk_pulses,
                                 "/Windlass/Signal K/Pulses");
  meters_per_pulse_output = new SKOutputFloat(
      runtime_config->sk_meters_per_pulse,
      "/Windlass/Signal K/Meters Per Pulse");
  fault_output = new SKOutputInt(runtime_config->sk_faults,
                                 "/Windlass/Signal K/Faults");
  mode_output =
      new SKOutputString(runtime_config->sk_mode, "/Windlass/Signal K/Mode");
  free_fall_output = new SKOutputBool(runtime_config->sk_freefall_detected,
                                      "/Windlass/Signal K/Free Fall");
  anchor_detected_output = new SKOutputBool(
      runtime_config->sk_anchor_detected, "/Windlass/Signal K/Anchor Detected");
  seafloor_detected_output = new SKOutputBool(
      runtime_config->sk_seafloor_detected,
      "/Windlass/Signal K/Seafloor Detected");
  event_output =
      new SKOutputString(runtime_config->sk_event, "/Windlass/Signal K/Event");
  notification_output = new SKOutputString(
      runtime_config->sk_notification, "/Windlass/Signal K/Notification");
  anchor_watch_enabled_output = new SKOutputBool(
      DEFAULT_SK_ANCHOR_WATCH_ENABLED, "/Anchor Watch/Signal K/Enabled");
  anchor_watch_auto_arm_output = new SKOutputBool(
      DEFAULT_SK_ANCHOR_WATCH_AUTO_ARM, "/Anchor Watch/Signal K/Auto Arm");
  anchor_watch_state_output = new SKOutputString(
      DEFAULT_SK_ANCHOR_WATCH_STATE, "/Anchor Watch/Signal K/State");
  anchor_watch_radius_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_RADIUS, "/Anchor Watch/Signal K/Radius",
      makeMetadata("m", "Anchor watch alarm radius", "Anchor Watch Radius",
                   "Radius"));
  anchor_watch_distance_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_DISTANCE, "/Anchor Watch/Signal K/Distance",
      makeMetadata("m", "Distance from anchor watch centre",
                   "Anchor Watch Distance", "Distance"));
  anchor_watch_margin_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_MARGIN, "/Anchor Watch/Signal K/Margin",
      makeMetadata("m", "Distance minus anchor watch radius",
                   "Anchor Watch Margin", "Margin"));
  anchor_watch_lat_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_POSITION_LAT,
      "/Anchor Watch/Signal K/Anchor Latitude");
  anchor_watch_lon_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_POSITION_LON,
      "/Anchor Watch/Signal K/Anchor Longitude");
  anchor_watch_rode_at_arm_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_RODE_AT_ARM,
      "/Anchor Watch/Signal K/Rode At Arm");
  anchor_watch_gnss_present_output = new SKOutputBool(
      DEFAULT_SK_ANCHOR_WATCH_GNSS_PRESENT,
      "/Anchor Watch/Signal K/GNSS Present");
  anchor_watch_gnss_interface_output = new SKOutputString(
      DEFAULT_SK_ANCHOR_WATCH_GNSS_INTERFACE,
      "/Anchor Watch/Signal K/GNSS Interface");
  anchor_watch_gnss_fix_valid_output = new SKOutputBool(
      DEFAULT_SK_ANCHOR_WATCH_GNSS_FIX_VALID,
      "/Anchor Watch/Signal K/GNSS Fix Valid");
  anchor_watch_gnss_hdop_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_GNSS_HDOP, "/Anchor Watch/Signal K/GNSS HDOP");
  anchor_watch_gnss_sats_output = new SKOutputInt(
      DEFAULT_SK_ANCHOR_WATCH_GNSS_SATS,
      "/Anchor Watch/Signal K/GNSS Satellites");
  anchor_watch_gnss_lat_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_GNSS_LAT,
      "/Anchor Watch/Signal K/GNSS Latitude");
  anchor_watch_gnss_lon_output = new SKOutputFloat(
      DEFAULT_SK_ANCHOR_WATCH_GNSS_LON,
      "/Anchor Watch/Signal K/GNSS Longitude");
  anchor_watch_notification_output = new SKOutputString(
      DEFAULT_SK_ANCHOR_WATCH_NOTIFICATION,
      "/Anchor Watch/Signal K/Notification");

  auto command_listener =
      new StringSKListener(runtime_config->sk_command_request);
  command_listener->connect_to(new LambdaConsumer<String>([](String input) {
    input.trim();
    if (input == "reset" || input == "zero") {
      setPulseCount(0);
      clearFault(FaultNearZeroLimit);
      markEvent(WindlassEvent::None);
    } else if (input == "faults clear") {
      state.fault_flags = FaultNone;
    } else if (!applyCommand(command_from_string(input), true)) {
      setFault(FaultInvalidCommand);
    }
  }));
}

void setupPins() {
  pinMode(kDigitalInputPin1, INPUT_PULLUP);
  pinMode(kDigitalInputPin2, INPUT);
  pinMode(kDigitalInputPin3, INPUT);
  pinMode(kDigitalInputPin4, INPUT_PULLUP);
  writeRelay(kWindlassUpRelayPin, false);
  writeRelay(kWindlassDownRelayPin, false);
  pinMode(kWindlassUpRelayPin, OUTPUT);
  pinMode(kWindlassDownRelayPin, OUTPUT);
  writeRelay(kWindlassUpRelayPin, false);
  writeRelay(kWindlassDownRelayPin, false);
  attachInterrupt(digitalPinToInterrupt(kDigitalInputPin1), onChainPulse,
                  FALLING);
}

void setupN2K() {
  nmea2000 = new tNMEA2000_esp32(kCANTxPin, kCANRxPin);
  nmea2000->SetN2kCANSendFrameBufSize(250);
  nmea2000->SetN2kCANReceiveFrameBufSize(250);
  nmea2000->SetProductInformation("000001", 100,
                                  "HALMET Windlass Controller",
                                  HALMET_WINDLASS_VERSION, "1");
  nmea2000->SetDeviceInformation(GetBoardSerialNumber(), 140, 35, 2046);
  nmea2000->SetMode(tNMEA2000::N2km_NodeOnly, DEVICE_INSTANCE);
  nmea2000->EnableForward(false);
  state.n2k_started = nmea2000->Open();
  if (!state.n2k_started) setFault(FaultNmea2000Error);
}

void setupResetInput() {
  auto reset_sensor = ConnectAlarmSender(kDigitalInputPin4, "D4");
  reset_sensor->connect_to(new LambdaConsumer<int>([](int input) {
    if (input == HIGH) {
      setPulseCount(0);
      clearFault(FaultNearZeroLimit);
    }
  }));
}

}  // namespace

void setup() {
  SetupLogging(ESP_LOG_DEBUG);
  Serial.begin(115200);
  Serial.println("HALMET Windlass Controller " HALMET_WINDLASS_VERSION);

  BUILDER_CLASS builder;
  sensesp_app = (&builder)
                    ->set_hostname("halmet-windlass-controller")
                    ->enable_ota("halmet")
                    ->get_app();

  setupRuntimeConfig();

  i2c = new TwoWire(0);
  i2c->begin(kSDAPin, kSCLPin);

  auto ads1115 = new Adafruit_ADS1115();
  ads1115->setGain(kADS1115Gain);
  bool ads_initialized = ads1115->begin(kADS1115Address, i2c);
  debugD("ADS1115 initialized: %d", ads_initialized);

  bool display_present = InitializeSSD1306(sensesp_app->get(), &display, i2c);
  if (!display_present) display = nullptr;

  prefs.begin("windlass", false);
  state.pulses_total = prefs.getLong64("pulses", 0);
  state.last_pulse_ms = millis();

  gps_manager = std::make_unique<GpsManager>(&runtime_config->gps);
  gps_manager->begin();
  anchor_watch = std::make_unique<AnchorWatch>(&runtime_config->anchor_watch,
                                               gps_manager.get(), &prefs);
  anchor_watch->begin();

  setupPins();
  setupResetInput();
  setupSignalK();
  setupN2K();

  event_loop()->onRepeat(100, updateController);
  event_loop()->onRepeat(SIGNALK_PUBLISH_PERIOD_MS, publishState);
  event_loop()->onRepeat(1, []() {
    if (state.n2k_started) nmea2000->ParseMessages();
  });
  event_loop()->onRepeat(N2K_PUBLISH_PERIOD_MS, publishN2K);
  if (display_present) {
    event_loop()->onRepeat(1000, []() {
      ClearRow(display, 1);
      PrintValue(display, 1, "IPx:", WiFi.localIP().toString());
    });
  }

  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }
