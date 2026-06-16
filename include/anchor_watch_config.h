#pragma once

#include <Arduino.h>

struct AnchorWatchConfig {
  bool enabled = true;
  bool auto_arm = true;
  float deploy_threshold_m = 5.0f;
  float onboard_threshold_m = 0.5f;
  float manual_radius_m = 35.0f;
  bool automatic_radius = true;
  float scope_multiplier = 1.15f;
  float boat_length_m = 10.0f;
  float bow_offset_m = 0.0f;
  float gps_error_margin_m = 10.0f;
  float min_radius_m = 25.0f;
  uint32_t arming_delay_ms = 10000;
  uint32_t alarm_delay_ms = 15000;
  uint32_t clear_delay_ms = 30000;
  float hysteresis_m = 5.0f;
  bool waypoint_enabled = true;
  bool waypoint_delete_on_disarm = false;
  bool n2k_publish_gnss = false;
  bool n2k_anchor_watch_as_active_waypoint = false;
  String anchor_position_strategy = "weighted_set_fix";
  String waypoint_id = "halmet-anchor-watch-current";
};
