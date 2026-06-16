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
    JsonObject gps_config = root["gps"].to<JsonObject>();
    gps_config["mode"] = gps.mode;
    gps_config["rx_pin"] = gps.rx_pin;
    gps_config["tx_pin"] = gps.tx_pin;
    gps_config["baud"] = gps.baud;
    gps_config["publish_navigation_position"] = gps.publish_navigation_position;
    gps_config["min_satellites"] = gps.min_satellites;
    gps_config["max_hdop"] = gps.max_hdop;
    gps_config["max_fix_age_ms"] = gps.max_fix_age_ms;
    gps_config["stable_samples"] = gps.stable_samples;

    JsonObject aw = root["anchor_watch"].to<JsonObject>();
    aw["enabled"] = anchor_watch.enabled;
    aw["auto_arm"] = anchor_watch.auto_arm;
    aw["deploy_threshold_m"] = anchor_watch.deploy_threshold_m;
    aw["onboard_threshold_m"] = anchor_watch.onboard_threshold_m;
    aw["manual_radius_m"] = anchor_watch.manual_radius_m;
    aw["automatic_radius"] = anchor_watch.automatic_radius;
    aw["scope_multiplier"] = anchor_watch.scope_multiplier;
    aw["boat_length_m"] = anchor_watch.boat_length_m;
    aw["bow_offset_m"] = anchor_watch.bow_offset_m;
    aw["gps_error_margin_m"] = anchor_watch.gps_error_margin_m;
    aw["min_radius_m"] = anchor_watch.min_radius_m;
    aw["arming_delay_ms"] = anchor_watch.arming_delay_ms;
    aw["alarm_delay_ms"] = anchor_watch.alarm_delay_ms;
    aw["clear_delay_ms"] = anchor_watch.clear_delay_ms;
    aw["hysteresis_m"] = anchor_watch.hysteresis_m;
    aw["waypoint_enabled"] = anchor_watch.waypoint_enabled;
    aw["waypoint_delete_on_disarm"] = anchor_watch.waypoint_delete_on_disarm;
    aw["n2k_publish_gnss"] = anchor_watch.n2k_publish_gnss;
    aw["n2k_anchor_watch_as_active_waypoint"] =
        anchor_watch.n2k_anchor_watch_as_active_waypoint;
    aw["anchor_position_strategy"] = anchor_watch.anchor_position_strategy;
    aw["waypoint_id"] = anchor_watch.waypoint_id;

    JsonObject paths = root["signalk_paths"].to<JsonObject>();
    paths["rode_length"] = sk_rode_length;
    paths["rode_speed"] = sk_rode_speed;
    paths["direction"] = sk_direction;
    paths["state"] = sk_state;
    paths["command_status"] = sk_command_status;
    paths["command_request"] = sk_command_request;
    paths["pulses"] = sk_pulses;
    paths["meters_per_pulse"] = sk_meters_per_pulse;
    paths["faults"] = sk_faults;
    paths["mode"] = sk_mode;
    paths["freefall_detected"] = sk_freefall_detected;
    paths["anchor_detected"] = sk_anchor_detected;
    paths["seafloor_detected"] = sk_seafloor_detected;
    paths["event"] = sk_event;
    paths["notification"] = sk_notification;
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

    JsonObjectConst gps_config = root["gps"];
    if (!gps_config.isNull()) {
      if (gps_config["mode"].is<const char*>()) gps.mode = gps_config["mode"].as<String>();
      if (gps_config["rx_pin"].is<int>()) gps.rx_pin = gps_config["rx_pin"];
      if (gps_config["tx_pin"].is<int>()) gps.tx_pin = gps_config["tx_pin"];
      if (gps_config["baud"].is<uint32_t>()) gps.baud = gps_config["baud"];
      if (gps_config["publish_navigation_position"].is<bool>()) gps.publish_navigation_position = gps_config["publish_navigation_position"];
      if (gps_config["min_satellites"].is<uint8_t>()) gps.min_satellites = gps_config["min_satellites"];
      if (gps_config["max_hdop"].is<float>()) gps.max_hdop = gps_config["max_hdop"];
      if (gps_config["max_fix_age_ms"].is<uint32_t>()) gps.max_fix_age_ms = gps_config["max_fix_age_ms"];
      if (gps_config["stable_samples"].is<uint8_t>()) gps.stable_samples = gps_config["stable_samples"];
    }

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
      "gps": {
        "title": "GPS/GNSS",
        "type": "object",
        "properties": {
          "mode": { "title": "GPS mode", "type": "string", "enum": ["auto", "uart", "i2c", "disabled"] },
          "rx_pin": { "title": "GPS UART RX pin (-1 disables)", "type": "integer", "minimum": -1, "maximum": 39 },
          "tx_pin": { "title": "GPS UART TX pin (-1 receive only)", "type": "integer", "minimum": -1, "maximum": 39 },
          "baud": { "title": "GPS baud (0 scans common bauds)", "type": "integer" },
          "publish_navigation_position": { "title": "Publish local GPS to navigation.*", "type": "boolean" },
          "min_satellites": { "title": "Minimum satellites", "type": "integer", "minimum": 1, "maximum": 16 },
          "max_hdop": { "title": "Maximum HDOP", "type": "number", "minimum": 0.5, "maximum": 10.0 },
          "max_fix_age_ms": { "title": "Maximum fix age, ms", "type": "integer", "minimum": 1000, "maximum": 60000 },
          "stable_samples": { "title": "Stable samples before arming", "type": "integer", "minimum": 1, "maximum": 20 }
        }
      },
      "anchor_watch": {
        "title": "Anchor watch",
        "type": "object",
        "properties": {
          "enabled": { "title": "Enable anchor watch", "type": "boolean" },
          "auto_arm": { "title": "Auto-arm after deployment", "type": "boolean" },
          "deploy_threshold_m": { "title": "Deploy threshold, m", "type": "number" },
          "onboard_threshold_m": { "title": "On-board threshold, m", "type": "number" },
          "manual_radius_m": { "title": "Manual radius, m", "type": "number" },
          "automatic_radius": { "title": "Automatic radius", "type": "boolean" },
          "scope_multiplier": { "title": "Scope multiplier", "type": "number" },
          "boat_length_m": { "title": "Boat length, m", "type": "number" },
          "bow_offset_m": { "title": "Bow GPS offset, m", "type": "number" },
          "gps_error_margin_m": { "title": "GPS error margin, m", "type": "number" },
          "min_radius_m": { "title": "Minimum automatic radius, m", "type": "number" },
          "arming_delay_ms": { "title": "Arming delay, ms", "type": "integer" },
          "alarm_delay_ms": { "title": "Alarm delay, ms", "type": "integer" },
          "clear_delay_ms": { "title": "Clear delay, ms", "type": "integer" },
          "hysteresis_m": { "title": "Clear hysteresis, m", "type": "number" },
          "waypoint_enabled": { "title": "Save waypoint locally", "type": "boolean" },
          "waypoint_delete_on_disarm": { "title": "Delete waypoint on disarm", "type": "boolean" },
          "n2k_publish_gnss": { "title": "Publish GPS PGNs", "type": "boolean" },
          "n2k_anchor_watch_as_active_waypoint": { "title": "Publish anchor as active N2K waypoint", "type": "boolean" },
          "anchor_position_strategy": { "title": "Anchor position strategy", "type": "string" },
          "waypoint_id": { "title": "Waypoint ID", "type": "string" }
        }
      }
    }
  })###";
}

}  // namespace sensesp
