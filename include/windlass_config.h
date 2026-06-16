#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "anchor_watch_config.h"
#include "gps_config.h"
#include "sensesp/system/saveable.h"
#include "signalk_paths.h"

struct WindlassRuntimeConfig : public sensesp::FileSystemSaveable {
  String config_path = "/Windlass/Configuration";

  WindlassRuntimeConfig() : sensesp::FileSystemSaveable("/Windlass/Configuration") {}

  double meters_per_pulse = DEFAULT_METERS_PER_PULSE;
  uint32_t chain_pulse_debounce_ms = DEFAULT_CHAIN_PULSE_DEBOUNCE_MS;
  uint32_t command_deadman_ms = DEFAULT_COMMAND_DEADMAN_MS;
  uint32_t stall_detect_ms = DEFAULT_STALL_DETECT_MS;
  double min_safe_length_m = DEFAULT_MIN_SAFE_LENGTH_M;
  bool relay_active_high = DEFAULT_RELAY_ACTIVE_HIGH;
  bool free_fall_detection_enabled = true;
  bool seafloor_detection_enabled = true;
  double free_fall_min_speed_m_s = DEFAULT_FREEFALL_MIN_SPEED_M_S;
  uint32_t free_fall_min_pulses = DEFAULT_FREEFALL_MIN_PULSES;
  uint32_t seafloor_no_pulse_ms = DEFAULT_SEAFLOOR_NO_PULSE_MS;
  double seafloor_min_length_m = DEFAULT_SEAFLOOR_MIN_LENGTH_M;
  double anchor_detected_length_m = DEFAULT_ANCHOR_DETECTED_LENGTH_M;
  GpsConfig gps;
  AnchorWatchConfig anchor_watch;

  String sk_rode_length = DEFAULT_SK_RODE_LENGTH;
  String sk_rode_speed = DEFAULT_SK_RODE_SPEED;
  String sk_direction = DEFAULT_SK_DIRECTION;
  String sk_state = DEFAULT_SK_STATE;
  String sk_command_status = DEFAULT_SK_COMMAND_STATUS;
  String sk_command_request = DEFAULT_SK_COMMAND_REQUEST;
  String sk_pulses = DEFAULT_SK_PULSES;
  String sk_meters_per_pulse = DEFAULT_SK_METERS_PER_PULSE;
  String sk_faults = DEFAULT_SK_FAULTS;
  String sk_mode = DEFAULT_SK_MODE;
  String sk_freefall_detected = DEFAULT_SK_FREEFALL_DETECTED;
  String sk_anchor_detected = DEFAULT_SK_ANCHOR_DETECTED;
  String sk_seafloor_detected = DEFAULT_SK_SEAFLOOR_DETECTED;
  String sk_event = DEFAULT_SK_EVENT;
  String sk_notification = DEFAULT_SK_NOTIFICATION;

  static bool readStringValue(JsonVariantConst value, String& target) {
    if (value.is<const char*>()) {
      target = value.as<String>();
      return true;
    }
    if (value.is<JsonArrayConst>()) {
      JsonArrayConst values = value.as<JsonArrayConst>();
      if (values.size() > 0 && values[0].is<const char*>()) {
        target = values[0].as<String>();
        return true;
      }
    }
    return false;
  }

  bool to_json(JsonObject& root) override {
    root["meters_per_pulse"] = meters_per_pulse;
    root["chain_pulse_debounce_ms"] = chain_pulse_debounce_ms;
    root["command_deadman_ms"] = command_deadman_ms;
    root["stall_detect_ms"] = stall_detect_ms;
    root["min_safe_length_m"] = min_safe_length_m;
    root["relay_active_high"] = relay_active_high;
    root["free_fall_detection_enabled"] = free_fall_detection_enabled;
    root["seafloor_detection_enabled"] = seafloor_detection_enabled;
    root["free_fall_min_speed_m_s"] = free_fall_min_speed_m_s;
    root["free_fall_min_pulses"] = free_fall_min_pulses;
    root["seafloor_no_pulse_ms"] = seafloor_no_pulse_ms;
    root["seafloor_min_length_m"] = seafloor_min_length_m;
    root["anchor_detected_length_m"] = anchor_detected_length_m;
    root["gps_position_source"] = gps.position_source;
    root["gps_mode"] = gps.mode;
    root["gps_rx_pin"] = gps.rx_pin;
    root["gps_tx_pin"] = gps.tx_pin;
    root["gps_baud"] = gps.baud;
    root["gps_sk_position_path"] = gps.sk_position_path;
    root["gps_publish_navigation_position"] = gps.publish_navigation_position;
    root["gps_min_satellites"] = gps.min_satellites;
    root["gps_max_hdop"] = gps.max_hdop;
    root["gps_max_fix_age_ms"] = gps.max_fix_age_ms;
    root["gps_stable_samples"] = gps.stable_samples;

    root["anchor_watch_enabled"] = anchor_watch.enabled;
    root["anchor_watch_auto_arm"] = anchor_watch.auto_arm;
    root["anchor_watch_deploy_threshold_m"] = anchor_watch.deploy_threshold_m;
    root["anchor_watch_onboard_threshold_m"] = anchor_watch.onboard_threshold_m;
    root["anchor_watch_manual_radius_m"] = anchor_watch.manual_radius_m;
    root["anchor_watch_automatic_radius"] = anchor_watch.automatic_radius;
    root["anchor_watch_scope_multiplier"] = anchor_watch.scope_multiplier;
    root["anchor_watch_boat_length_m"] = anchor_watch.boat_length_m;
    root["anchor_watch_bow_offset_m"] = anchor_watch.bow_offset_m;
    root["anchor_watch_gps_error_margin_m"] = anchor_watch.gps_error_margin_m;
    root["anchor_watch_min_radius_m"] = anchor_watch.min_radius_m;
    root["anchor_watch_arming_delay_ms"] = anchor_watch.arming_delay_ms;
    root["anchor_watch_alarm_delay_ms"] = anchor_watch.alarm_delay_ms;
    root["anchor_watch_clear_delay_ms"] = anchor_watch.clear_delay_ms;
    root["anchor_watch_hysteresis_m"] = anchor_watch.hysteresis_m;
    root["anchor_watch_waypoint_enabled"] = anchor_watch.waypoint_enabled;
    root["anchor_watch_waypoint_delete_on_disarm"] =
        anchor_watch.waypoint_delete_on_disarm;
    root["anchor_watch_n2k_publish_gnss"] = anchor_watch.n2k_publish_gnss;
    root["anchor_watch_n2k_anchor_watch_as_active_waypoint"] =
        anchor_watch.n2k_anchor_watch_as_active_waypoint;
    root["anchor_watch_anchor_position_strategy"] =
        anchor_watch.anchor_position_strategy;
    root["anchor_watch_waypoint_id"] = anchor_watch.waypoint_id;

    root["sk_rode_length"] = sk_rode_length;
    root["sk_rode_speed"] = sk_rode_speed;
    root["sk_direction"] = sk_direction;
    root["sk_state"] = sk_state;
    root["sk_command_status"] = sk_command_status;
    root["sk_command_request"] = sk_command_request;
    root["sk_pulses"] = sk_pulses;
    root["sk_meters_per_pulse"] = sk_meters_per_pulse;
    root["sk_faults"] = sk_faults;
    root["sk_mode"] = sk_mode;
    root["sk_freefall_detected"] = sk_freefall_detected;
    root["sk_anchor_detected"] = sk_anchor_detected;
    root["sk_seafloor_detected"] = sk_seafloor_detected;
    root["sk_event"] = sk_event;
    root["sk_notification"] = sk_notification;
    return true;
  }

  bool from_json(const JsonObject& root) override {
    if (root["meters_per_pulse"].is<double>()) {
      double v = root["meters_per_pulse"];
      if (v >= 0.01 && v <= 2.0) meters_per_pulse = v;
    }
    if (root["chain_pulse_debounce_ms"].is<uint32_t>()) {
      uint32_t v = root["chain_pulse_debounce_ms"];
      if (v >= 5 && v <= 1000) chain_pulse_debounce_ms = v;
    }
    if (root["command_deadman_ms"].is<uint32_t>()) {
      uint32_t v = root["command_deadman_ms"];
      if (v >= 250 && v <= 10000) command_deadman_ms = v;
    }
    if (root["stall_detect_ms"].is<uint32_t>()) {
      uint32_t v = root["stall_detect_ms"];
      if (v >= 500 && v <= 30000) stall_detect_ms = v;
    }
    if (root["min_safe_length_m"].is<double>()) {
      double v = root["min_safe_length_m"];
      if (v >= 0.0 && v <= 5.0) min_safe_length_m = v;
    }
    if (root["relay_active_high"].is<bool>()) relay_active_high = root["relay_active_high"];
    if (root["free_fall_detection_enabled"].is<bool>()) free_fall_detection_enabled = root["free_fall_detection_enabled"];
    if (root["seafloor_detection_enabled"].is<bool>()) seafloor_detection_enabled = root["seafloor_detection_enabled"];
    if (root["free_fall_min_speed_m_s"].is<double>()) {
      double v = root["free_fall_min_speed_m_s"];
      if (v >= 0.01 && v <= 3.0) free_fall_min_speed_m_s = v;
    }
    if (root["free_fall_min_pulses"].is<uint32_t>()) {
      uint32_t v = root["free_fall_min_pulses"];
      if (v >= 1 && v <= 50) free_fall_min_pulses = v;
    }
    if (root["seafloor_no_pulse_ms"].is<uint32_t>()) {
      uint32_t v = root["seafloor_no_pulse_ms"];
      if (v >= 250 && v <= 10000) seafloor_no_pulse_ms = v;
    }
    if (root["seafloor_min_length_m"].is<double>()) {
      double v = root["seafloor_min_length_m"];
      if (v >= 0.0 && v <= 20.0) seafloor_min_length_m = v;
    }
    if (root["anchor_detected_length_m"].is<double>()) {
      double v = root["anchor_detected_length_m"];
      if (v >= 0.0 && v <= 10.0) anchor_detected_length_m = v;
    }

    readStringValue(root["gps_position_source"], gps.position_source);
    readStringValue(root["gps_mode"], gps.mode);
    if (root["gps_rx_pin"].is<int>()) gps.rx_pin = root["gps_rx_pin"];
    if (root["gps_tx_pin"].is<int>()) gps.tx_pin = root["gps_tx_pin"];
    if (root["gps_baud"].is<uint32_t>()) gps.baud = root["gps_baud"];
    if (root["gps_sk_position_path"].is<const char*>()) gps.sk_position_path = root["gps_sk_position_path"].as<String>();
    if (root["gps_publish_navigation_position"].is<bool>()) gps.publish_navigation_position = root["gps_publish_navigation_position"];
    if (root["gps_min_satellites"].is<uint8_t>()) gps.min_satellites = root["gps_min_satellites"];
    if (root["gps_max_hdop"].is<float>()) gps.max_hdop = root["gps_max_hdop"];
    if (root["gps_max_fix_age_ms"].is<uint32_t>()) gps.max_fix_age_ms = root["gps_max_fix_age_ms"];
    if (root["gps_stable_samples"].is<uint8_t>()) gps.stable_samples = root["gps_stable_samples"];

    JsonObjectConst gps_config = root["gps"];
    if (!gps_config.isNull()) {
      if (gps_config["position_source"].is<const char*>()) gps.position_source = gps_config["position_source"].as<String>();
      if (gps_config["mode"].is<const char*>()) gps.mode = gps_config["mode"].as<String>();
      if (gps_config["rx_pin"].is<int>()) gps.rx_pin = gps_config["rx_pin"];
      if (gps_config["tx_pin"].is<int>()) gps.tx_pin = gps_config["tx_pin"];
      if (gps_config["baud"].is<uint32_t>()) gps.baud = gps_config["baud"];
      if (gps_config["sk_position_path"].is<const char*>()) gps.sk_position_path = gps_config["sk_position_path"].as<String>();
      if (gps_config["publish_navigation_position"].is<bool>()) gps.publish_navigation_position = gps_config["publish_navigation_position"];
      if (gps_config["min_satellites"].is<uint8_t>()) gps.min_satellites = gps_config["min_satellites"];
      if (gps_config["max_hdop"].is<float>()) gps.max_hdop = gps_config["max_hdop"];
      if (gps_config["max_fix_age_ms"].is<uint32_t>()) gps.max_fix_age_ms = gps_config["max_fix_age_ms"];
      if (gps_config["stable_samples"].is<uint8_t>()) gps.stable_samples = gps_config["stable_samples"];
    }

    if (root["anchor_watch_enabled"].is<bool>()) anchor_watch.enabled = root["anchor_watch_enabled"];
    if (root["anchor_watch_auto_arm"].is<bool>()) anchor_watch.auto_arm = root["anchor_watch_auto_arm"];
    if (root["anchor_watch_deploy_threshold_m"].is<float>()) anchor_watch.deploy_threshold_m = root["anchor_watch_deploy_threshold_m"];
    if (root["anchor_watch_onboard_threshold_m"].is<float>()) anchor_watch.onboard_threshold_m = root["anchor_watch_onboard_threshold_m"];
    if (root["anchor_watch_manual_radius_m"].is<float>()) anchor_watch.manual_radius_m = root["anchor_watch_manual_radius_m"];
    if (root["anchor_watch_automatic_radius"].is<bool>()) anchor_watch.automatic_radius = root["anchor_watch_automatic_radius"];
    if (root["anchor_watch_scope_multiplier"].is<float>()) anchor_watch.scope_multiplier = root["anchor_watch_scope_multiplier"];
    if (root["anchor_watch_boat_length_m"].is<float>()) anchor_watch.boat_length_m = root["anchor_watch_boat_length_m"];
    if (root["anchor_watch_bow_offset_m"].is<float>()) anchor_watch.bow_offset_m = root["anchor_watch_bow_offset_m"];
    if (root["anchor_watch_gps_error_margin_m"].is<float>()) anchor_watch.gps_error_margin_m = root["anchor_watch_gps_error_margin_m"];
    if (root["anchor_watch_min_radius_m"].is<float>()) anchor_watch.min_radius_m = root["anchor_watch_min_radius_m"];
    if (root["anchor_watch_arming_delay_ms"].is<uint32_t>()) anchor_watch.arming_delay_ms = root["anchor_watch_arming_delay_ms"];
    if (root["anchor_watch_alarm_delay_ms"].is<uint32_t>()) anchor_watch.alarm_delay_ms = root["anchor_watch_alarm_delay_ms"];
    if (root["anchor_watch_clear_delay_ms"].is<uint32_t>()) anchor_watch.clear_delay_ms = root["anchor_watch_clear_delay_ms"];
    if (root["anchor_watch_hysteresis_m"].is<float>()) anchor_watch.hysteresis_m = root["anchor_watch_hysteresis_m"];
    if (root["anchor_watch_waypoint_enabled"].is<bool>()) anchor_watch.waypoint_enabled = root["anchor_watch_waypoint_enabled"];
    if (root["anchor_watch_waypoint_delete_on_disarm"].is<bool>()) anchor_watch.waypoint_delete_on_disarm = root["anchor_watch_waypoint_delete_on_disarm"];
    if (root["anchor_watch_n2k_publish_gnss"].is<bool>()) anchor_watch.n2k_publish_gnss = root["anchor_watch_n2k_publish_gnss"];
    if (root["anchor_watch_n2k_anchor_watch_as_active_waypoint"].is<bool>()) anchor_watch.n2k_anchor_watch_as_active_waypoint = root["anchor_watch_n2k_anchor_watch_as_active_waypoint"];
    readStringValue(root["anchor_watch_anchor_position_strategy"],
                    anchor_watch.anchor_position_strategy);
    if (root["anchor_watch_waypoint_id"].is<const char*>()) anchor_watch.waypoint_id = root["anchor_watch_waypoint_id"].as<String>();

    JsonObjectConst aw = root["anchor_watch"];
    if (!aw.isNull()) {
      if (aw["enabled"].is<bool>()) anchor_watch.enabled = aw["enabled"];
      if (aw["auto_arm"].is<bool>()) anchor_watch.auto_arm = aw["auto_arm"];
      if (aw["deploy_threshold_m"].is<float>()) anchor_watch.deploy_threshold_m = aw["deploy_threshold_m"];
      if (aw["onboard_threshold_m"].is<float>()) anchor_watch.onboard_threshold_m = aw["onboard_threshold_m"];
      if (aw["manual_radius_m"].is<float>()) anchor_watch.manual_radius_m = aw["manual_radius_m"];
      if (aw["automatic_radius"].is<bool>()) anchor_watch.automatic_radius = aw["automatic_radius"];
      if (aw["scope_multiplier"].is<float>()) anchor_watch.scope_multiplier = aw["scope_multiplier"];
      if (aw["boat_length_m"].is<float>()) anchor_watch.boat_length_m = aw["boat_length_m"];
      if (aw["bow_offset_m"].is<float>()) anchor_watch.bow_offset_m = aw["bow_offset_m"];
      if (aw["gps_error_margin_m"].is<float>()) anchor_watch.gps_error_margin_m = aw["gps_error_margin_m"];
      if (aw["min_radius_m"].is<float>()) anchor_watch.min_radius_m = aw["min_radius_m"];
      if (aw["arming_delay_ms"].is<uint32_t>()) anchor_watch.arming_delay_ms = aw["arming_delay_ms"];
      if (aw["alarm_delay_ms"].is<uint32_t>()) anchor_watch.alarm_delay_ms = aw["alarm_delay_ms"];
      if (aw["clear_delay_ms"].is<uint32_t>()) anchor_watch.clear_delay_ms = aw["clear_delay_ms"];
      if (aw["hysteresis_m"].is<float>()) anchor_watch.hysteresis_m = aw["hysteresis_m"];
      if (aw["waypoint_enabled"].is<bool>()) anchor_watch.waypoint_enabled = aw["waypoint_enabled"];
      if (aw["waypoint_delete_on_disarm"].is<bool>()) anchor_watch.waypoint_delete_on_disarm = aw["waypoint_delete_on_disarm"];
      if (aw["n2k_publish_gnss"].is<bool>()) anchor_watch.n2k_publish_gnss = aw["n2k_publish_gnss"];
      if (aw["n2k_anchor_watch_as_active_waypoint"].is<bool>()) anchor_watch.n2k_anchor_watch_as_active_waypoint = aw["n2k_anchor_watch_as_active_waypoint"];
      if (aw["anchor_position_strategy"].is<const char*>()) anchor_watch.anchor_position_strategy = aw["anchor_position_strategy"].as<String>();
      if (aw["waypoint_id"].is<const char*>()) anchor_watch.waypoint_id = aw["waypoint_id"].as<String>();
    }

    if (root["sk_rode_length"].is<const char*>()) sk_rode_length = root["sk_rode_length"].as<String>();
    if (root["sk_rode_speed"].is<const char*>()) sk_rode_speed = root["sk_rode_speed"].as<String>();
    if (root["sk_direction"].is<const char*>()) sk_direction = root["sk_direction"].as<String>();
    if (root["sk_state"].is<const char*>()) sk_state = root["sk_state"].as<String>();
    if (root["sk_command_status"].is<const char*>()) sk_command_status = root["sk_command_status"].as<String>();
    if (root["sk_command_request"].is<const char*>()) sk_command_request = root["sk_command_request"].as<String>();
    if (root["sk_pulses"].is<const char*>()) sk_pulses = root["sk_pulses"].as<String>();
    if (root["sk_meters_per_pulse"].is<const char*>()) sk_meters_per_pulse = root["sk_meters_per_pulse"].as<String>();
    if (root["sk_faults"].is<const char*>()) sk_faults = root["sk_faults"].as<String>();
    if (root["sk_mode"].is<const char*>()) sk_mode = root["sk_mode"].as<String>();
    if (root["sk_freefall_detected"].is<const char*>()) sk_freefall_detected = root["sk_freefall_detected"].as<String>();
    if (root["sk_anchor_detected"].is<const char*>()) sk_anchor_detected = root["sk_anchor_detected"].as<String>();
    if (root["sk_seafloor_detected"].is<const char*>()) sk_seafloor_detected = root["sk_seafloor_detected"].as<String>();
    if (root["sk_event"].is<const char*>()) sk_event = root["sk_event"].as<String>();
    if (root["sk_notification"].is<const char*>()) sk_notification = root["sk_notification"].as<String>();

    JsonObjectConst paths = root["signalk_paths"];
    if (!paths.isNull()) {
      if (paths["rode_length"].is<const char*>()) sk_rode_length = paths["rode_length"].as<String>();
      if (paths["rode_speed"].is<const char*>()) sk_rode_speed = paths["rode_speed"].as<String>();
      if (paths["direction"].is<const char*>()) sk_direction = paths["direction"].as<String>();
      if (paths["state"].is<const char*>()) sk_state = paths["state"].as<String>();
      if (paths["command_status"].is<const char*>()) sk_command_status = paths["command_status"].as<String>();
      if (paths["command_request"].is<const char*>()) sk_command_request = paths["command_request"].as<String>();
      if (paths["pulses"].is<const char*>()) sk_pulses = paths["pulses"].as<String>();
      if (paths["meters_per_pulse"].is<const char*>()) sk_meters_per_pulse = paths["meters_per_pulse"].as<String>();
      if (paths["faults"].is<const char*>()) sk_faults = paths["faults"].as<String>();
      if (paths["mode"].is<const char*>()) sk_mode = paths["mode"].as<String>();
      if (paths["freefall_detected"].is<const char*>()) sk_freefall_detected = paths["freefall_detected"].as<String>();
      if (paths["anchor_detected"].is<const char*>()) sk_anchor_detected = paths["anchor_detected"].as<String>();
      if (paths["seafloor_detected"].is<const char*>()) sk_seafloor_detected = paths["seafloor_detected"].as<String>();
      if (paths["event"].is<const char*>()) sk_event = paths["event"].as<String>();
      if (paths["notification"].is<const char*>()) sk_notification = paths["notification"].as<String>();
    }
    return true;
  }
};

namespace sensesp {

inline const String ConfigSchema(const WindlassRuntimeConfig& obj) {
  (void)obj;
  return R"###({
    "type": "object",
    "properties": {
      "meters_per_pulse": { "title": "Meters per pulse", "type": "number", "minimum": 0.01, "maximum": 2.0, "multipleOf": 0.001 },
      "chain_pulse_debounce_ms": { "title": "Pulse debounce, ms", "type": "integer", "minimum": 5, "maximum": 1000 },
      "command_deadman_ms": { "title": "Remote command dead-man timeout, ms", "type": "integer", "minimum": 250, "maximum": 10000 },
      "stall_detect_ms": { "title": "No-pulse stall timeout, ms", "type": "integer", "minimum": 500, "maximum": 30000 },
      "min_safe_length_m": { "title": "Minimum safe rode length, m", "type": "number", "minimum": 0.0, "maximum": 5.0, "multipleOf": 0.01 },
      "relay_active_high": { "title": "Relay module active-high inputs", "type": "boolean" },
      "free_fall_detection_enabled": { "title": "Enable free-fall detection", "type": "boolean" },
      "seafloor_detection_enabled": { "title": "Enable seafloor detection", "type": "boolean" },
      "free_fall_min_speed_m_s": { "title": "Free-fall minimum speed, m/s", "type": "number", "minimum": 0.01, "maximum": 3.0, "multipleOf": 0.01 },
      "free_fall_min_pulses": { "title": "Free-fall minimum pulses", "type": "integer", "minimum": 1, "maximum": 50 },
      "seafloor_no_pulse_ms": { "title": "Seafloor no-pulse time, ms", "type": "integer", "minimum": 250, "maximum": 10000 },
      "seafloor_min_length_m": { "title": "Seafloor minimum deployed length, m", "type": "number", "minimum": 0.0, "maximum": 20.0, "multipleOf": 0.1 },
      "anchor_detected_length_m": { "title": "Anchor detected length, m", "type": "number", "minimum": 0.0, "maximum": 10.0, "multipleOf": 0.01 },
      "gps_position_source": { "title": "GPS position source", "type": "array", "format": "select", "uniqueItems": true, "items": { "type": "string", "enum": ["local", "signalk", "nmea2000"] } },
      "gps_mode": { "title": "GPS mode", "type": "array", "format": "select", "uniqueItems": true, "items": { "type": "string", "enum": ["auto", "uart", "i2c", "disabled"] } },
      "gps_rx_pin": { "title": "GPS UART RX pin (-1 disables)", "type": "integer", "minimum": -1, "maximum": 39 },
      "gps_tx_pin": { "title": "GPS UART TX pin (-1 receive only)", "type": "integer", "minimum": -1, "maximum": 39 },
      "gps_baud": { "title": "GPS baud (0 scans common bauds)", "type": "integer" },
      "gps_sk_position_path": { "title": "Signal K position path", "type": "string" },
      "gps_publish_navigation_position": { "title": "Reserved: publish local GPS to navigation.*", "type": "boolean" },
      "gps_min_satellites": { "title": "GPS minimum satellites", "type": "integer", "minimum": 1, "maximum": 16 },
      "gps_max_hdop": { "title": "GPS maximum HDOP", "type": "number", "minimum": 0.5, "maximum": 10.0 },
      "gps_max_fix_age_ms": { "title": "GPS maximum fix age, ms", "type": "integer", "minimum": 1000, "maximum": 60000 },
      "gps_stable_samples": { "title": "GPS stable samples before arming", "type": "integer", "minimum": 1, "maximum": 20 },
      "anchor_watch_enabled": { "title": "Enable anchor watch", "type": "boolean" },
      "anchor_watch_auto_arm": { "title": "Anchor watch auto-arm after deployment", "type": "boolean" },
      "anchor_watch_deploy_threshold_m": { "title": "Anchor watch deploy threshold, m", "type": "number" },
      "anchor_watch_onboard_threshold_m": { "title": "Anchor watch on-board threshold, m", "type": "number" },
      "anchor_watch_manual_radius_m": { "title": "Anchor watch manual radius, m", "type": "number" },
      "anchor_watch_automatic_radius": { "title": "Anchor watch automatic radius", "type": "boolean" },
      "anchor_watch_scope_multiplier": { "title": "Anchor watch scope multiplier", "type": "number" },
      "anchor_watch_boat_length_m": { "title": "Anchor watch boat length, m", "type": "number" },
      "anchor_watch_bow_offset_m": { "title": "Anchor watch bow GPS offset, m", "type": "number" },
      "anchor_watch_gps_error_margin_m": { "title": "Anchor watch GPS error margin, m", "type": "number" },
      "anchor_watch_min_radius_m": { "title": "Anchor watch minimum automatic radius, m", "type": "number" },
      "anchor_watch_arming_delay_ms": { "title": "Anchor watch arming delay, ms", "type": "integer" },
      "anchor_watch_alarm_delay_ms": { "title": "Anchor watch alarm delay, ms", "type": "integer" },
      "anchor_watch_clear_delay_ms": { "title": "Anchor watch clear delay, ms", "type": "integer" },
      "anchor_watch_hysteresis_m": { "title": "Anchor watch clear hysteresis, m", "type": "number" },
      "anchor_watch_waypoint_enabled": { "title": "Anchor watch save waypoint locally", "type": "boolean" },
      "anchor_watch_waypoint_delete_on_disarm": { "title": "Anchor watch delete waypoint on disarm", "type": "boolean" },
      "anchor_watch_n2k_publish_gnss": { "title": "Anchor watch publish position PGNs", "type": "boolean" },
      "anchor_watch_n2k_anchor_watch_as_active_waypoint": { "title": "Anchor watch publish anchor as active N2K waypoint", "type": "boolean" },
      "anchor_watch_anchor_position_strategy": { "title": "Anchor watch anchor position strategy", "type": "array", "format": "select", "uniqueItems": true, "items": { "type": "string", "enum": ["weighted_set_fix", "current_fix"] } },
      "anchor_watch_waypoint_id": { "title": "Anchor watch waypoint ID", "type": "string" },
      "sk_rode_length": { "title": "Signal K rode length path", "type": "string" },
      "sk_rode_speed": { "title": "Signal K rode speed path", "type": "string" },
      "sk_direction": { "title": "Signal K direction path", "type": "string" },
      "sk_state": { "title": "Signal K state path", "type": "string" },
      "sk_command_status": { "title": "Signal K command status path", "type": "string" },
      "sk_command_request": { "title": "Signal K command request path", "type": "string" },
      "sk_pulses": { "title": "Signal K pulses path", "type": "string" },
      "sk_meters_per_pulse": { "title": "Signal K meters per pulse path", "type": "string" },
      "sk_faults": { "title": "Signal K faults path", "type": "string" },
      "sk_mode": { "title": "Signal K mode path", "type": "string" },
      "sk_freefall_detected": { "title": "Signal K free-fall detected path", "type": "string" },
      "sk_anchor_detected": { "title": "Signal K anchor detected path", "type": "string" },
      "sk_seafloor_detected": { "title": "Signal K seafloor detected path", "type": "string" },
      "sk_event": { "title": "Signal K event path", "type": "string" },
      "sk_notification": { "title": "Signal K notification path", "type": "string" }
    }
  })###";
}

}  // namespace sensesp
