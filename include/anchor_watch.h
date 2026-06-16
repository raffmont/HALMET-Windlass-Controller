#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "anchor_watch_config.h"
#include "gps_manager.h"
#include "windlass_state.h"

enum class AnchorWatchState : uint8_t {
  Disabled,
  WaitingForGps,
  Ready,
  Armed,
  Alarm,
  Suspended,
  Fault
};

struct AnchorWatchSnapshot {
  AnchorWatchState state = AnchorWatchState::Disabled;
  double anchor_latitude = 0.0;
  double anchor_longitude = 0.0;
  double radius_m = 0.0;
  double distance_m = 0.0;
  double margin_m = 0.0;
  double rode_length_at_arm_m = 0.0;
  String strategy = "none";
  String armed_at = "";
  String disarmed_at = "";
  String notification_path = "";
  String notification_state = "normal";
  String notification_message = "";
  bool waypoint_saved_local = false;
};

class AnchorWatch {
 public:
  AnchorWatch(AnchorWatchConfig* config, GpsManager* gps, Preferences* prefs);
  void begin();
  void update(const ChainCounterState& windlass);
  const AnchorWatchSnapshot& snapshot() const { return snapshot_; }
  const char* stateString() const;
  bool isArmed() const;

 private:
  void transitionTo(AnchorWatchState state);
  void arm(const ChainCounterState& windlass);
  void disarm();
  void updateAnchorCandidate(const ChainCounterState& windlass);
  void updateAlarm();
  double automaticRadius(double rode_length_m) const;
  bool shouldAutoArm(const ChainCounterState& windlass) const;
  bool shouldAutoDisarm(const ChainCounterState& windlass) const;
  void persistState();
  void loadState();
  String uptimeStamp() const;
  void notify(const String& path, const String& state, const String& message);

  AnchorWatchConfig* config_;
  GpsManager* gps_;
  Preferences* prefs_;
  AnchorWatchSnapshot snapshot_;
  GpsFix candidate_fix_;
  uint32_t candidate_count_ = 0;
  uint32_t arm_condition_started_ms_ = 0;
  uint32_t alarm_condition_started_ms_ = 0;
  uint32_t clear_condition_started_ms_ = 0;
};
