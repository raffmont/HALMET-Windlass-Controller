#include "anchor_watch.h"

#include <cmath>

#include "geo_utils.h"

namespace {
constexpr const char* kRadiusExceededPath =
    "notifications.anchoring.anchorWatch.radiusExceeded";
constexpr const char* kGpsLostPath =
    "notifications.anchoring.anchorWatch.gpsLost";
constexpr const char* kAutoArmedPath =
    "notifications.anchoring.anchorWatch.autoArmed";
constexpr const char* kAutoDisarmedPath =
    "notifications.anchoring.anchorWatch.autoDisarmed";
}  // namespace

AnchorWatch::AnchorWatch(AnchorWatchConfig* config, GpsManager* gps,
                         Preferences* prefs)
    : config_(config), gps_(gps), prefs_(prefs) {}

void AnchorWatch::begin() { loadState(); }

void AnchorWatch::update(const ChainCounterState& windlass) {
  if (!config_ || !config_->enabled) {
    transitionTo(AnchorWatchState::Disabled);
    return;
  }

  updateAnchorCandidate(windlass);

  if (shouldAutoDisarm(windlass)) {
    disarm();
    return;
  }

  if (!gps_->hasUsableFix()) {
    if (isArmed()) {
      notify(kGpsLostPath, "warn", "Anchor watch GPS fix is unavailable or poor.");
    } else {
      transitionTo(AnchorWatchState::WaitingForGps);
    }
    return;
  }

  if (snapshot_.state == AnchorWatchState::WaitingForGps ||
      snapshot_.state == AnchorWatchState::Disabled) {
    transitionTo(AnchorWatchState::Ready);
  }

  if (shouldAutoArm(windlass)) {
    if (arm_condition_started_ms_ == 0) arm_condition_started_ms_ = millis();
    if (millis() - arm_condition_started_ms_ >= config_->arming_delay_ms) {
      arm(windlass);
    }
  } else {
    arm_condition_started_ms_ = 0;
  }

  if (isArmed()) updateAlarm();
}

const char* AnchorWatch::stateString() const {
  switch (snapshot_.state) {
    case AnchorWatchState::WaitingForGps:
      return "waitingForGps";
    case AnchorWatchState::Ready:
      return "ready";
    case AnchorWatchState::Armed:
      return "armed";
    case AnchorWatchState::Alarm:
      return "alarm";
    case AnchorWatchState::Suspended:
      return "suspended";
    case AnchorWatchState::Fault:
      return "fault";
    default:
      return "disabled";
  }
}

bool AnchorWatch::isArmed() const {
  return snapshot_.state == AnchorWatchState::Armed ||
         snapshot_.state == AnchorWatchState::Alarm;
}

void AnchorWatch::transitionTo(AnchorWatchState state) {
  if (snapshot_.state == state) return;
  snapshot_.state = state;
  persistState();
}

void AnchorWatch::arm(const ChainCounterState& windlass) {
  const GpsFix& fix =
      candidate_count_ > 0 && candidate_fix_.valid ? candidate_fix_ : gps_->fix();
  snapshot_.anchor_latitude = fix.latitude;
  snapshot_.anchor_longitude = fix.longitude;
  snapshot_.rode_length_at_arm_m = windlass.rode_length_m;
  snapshot_.radius_m = config_->automatic_radius
                           ? automaticRadius(windlass.rode_length_m)
                           : config_->manual_radius_m;
  snapshot_.strategy =
      candidate_count_ > 0 ? config_->anchor_position_strategy : "current_fix";
  snapshot_.armed_at = uptimeStamp();
  snapshot_.waypoint_saved_local = config_->waypoint_enabled;
  transitionTo(AnchorWatchState::Armed);
  notify(kAutoArmedPath, "normal", "Anchor watch armed.");
  persistState();
}

void AnchorWatch::disarm() {
  if (!isArmed() && snapshot_.state != AnchorWatchState::Ready) return;
  snapshot_.disarmed_at = uptimeStamp();
  snapshot_.distance_m = 0.0;
  snapshot_.margin_m = 0.0;
  alarm_condition_started_ms_ = 0;
  clear_condition_started_ms_ = 0;
  transitionTo(gps_->hasUsableFix() ? AnchorWatchState::Ready
                                    : AnchorWatchState::WaitingForGps);
  notify(kRadiusExceededPath, "normal", "Anchor watch back inside radius.");
  notify(kAutoDisarmedPath, "normal", "Anchor watch disarmed.");
  persistState();
}

void AnchorWatch::updateAnchorCandidate(const ChainCounterState& windlass) {
  if (!gps_->hasUsableFix()) return;
  if (windlass.rode_length_m < config_->deploy_threshold_m) return;
  if (windlass.direction != WindlassDirection::Deploying &&
      !windlass.free_fall_latched && !windlass.seafloor_detected) {
    return;
  }
  candidate_fix_ = gps_->fix();
  candidate_count_++;
}

void AnchorWatch::updateAlarm() {
  const GpsFix& fix = gps_->fix();
  snapshot_.distance_m = haversineDistanceMeters(
      snapshot_.anchor_latitude, snapshot_.anchor_longitude, fix.latitude,
      fix.longitude);
  snapshot_.margin_m = snapshot_.distance_m - snapshot_.radius_m;

  const uint32_t now = millis();
  if (snapshot_.distance_m > snapshot_.radius_m) {
    if (alarm_condition_started_ms_ == 0) alarm_condition_started_ms_ = now;
    clear_condition_started_ms_ = 0;
    if (now - alarm_condition_started_ms_ >= config_->alarm_delay_ms) {
      transitionTo(AnchorWatchState::Alarm);
      notify(kRadiusExceededPath, "alarm",
             "Anchor watch radius exceeded: distance " +
                 String(snapshot_.distance_m, 1) + " m, radius " +
                 String(snapshot_.radius_m, 1) + " m.");
    }
  } else if (snapshot_.distance_m <=
             snapshot_.radius_m - config_->hysteresis_m) {
    if (clear_condition_started_ms_ == 0) clear_condition_started_ms_ = now;
    alarm_condition_started_ms_ = 0;
    if (snapshot_.state == AnchorWatchState::Alarm &&
        now - clear_condition_started_ms_ >= config_->clear_delay_ms) {
      transitionTo(AnchorWatchState::Armed);
      notify(kRadiusExceededPath, "normal",
             "Anchor watch back inside radius.");
    }
  }
}

double AnchorWatch::automaticRadius(double rode_length_m) const {
  const double radius = rode_length_m * config_->scope_multiplier +
                        config_->boat_length_m + config_->gps_error_margin_m +
                        config_->bow_offset_m;
  return fmax(config_->min_radius_m, radius);
}

bool AnchorWatch::shouldAutoArm(const ChainCounterState& windlass) const {
  return config_->auto_arm && !isArmed() &&
         windlass.rode_length_m >= config_->deploy_threshold_m &&
         (windlass.direction == WindlassDirection::Deploying ||
          windlass.free_fall_latched || windlass.seafloor_detected ||
          windlass.anchor_detected);
}

bool AnchorWatch::shouldAutoDisarm(const ChainCounterState& windlass) const {
  return isArmed() && windlass.rode_length_m <= config_->onboard_threshold_m &&
         (windlass.direction == WindlassDirection::Retrieving ||
          windlass.active_command == WindlassCommand::Stop);
}

void AnchorWatch::persistState() {
  if (!prefs_) return;
  prefs_->putUChar("aw_state", static_cast<uint8_t>(snapshot_.state));
  prefs_->putDouble("aw_lat", snapshot_.anchor_latitude);
  prefs_->putDouble("aw_lon", snapshot_.anchor_longitude);
  prefs_->putDouble("aw_radius", snapshot_.radius_m);
  prefs_->putDouble("aw_rode", snapshot_.rode_length_at_arm_m);
}

void AnchorWatch::loadState() {
  if (!prefs_) return;
  snapshot_.anchor_latitude = prefs_->getDouble("aw_lat", 0.0);
  snapshot_.anchor_longitude = prefs_->getDouble("aw_lon", 0.0);
  snapshot_.radius_m = prefs_->getDouble("aw_radius", 0.0);
  snapshot_.rode_length_at_arm_m = prefs_->getDouble("aw_rode", 0.0);
  const uint8_t saved = prefs_->getUChar("aw_state", 0);
  if (saved == static_cast<uint8_t>(AnchorWatchState::Armed) ||
      saved == static_cast<uint8_t>(AnchorWatchState::Alarm)) {
    snapshot_.state = AnchorWatchState::Suspended;
  }
}

String AnchorWatch::uptimeStamp() const {
  return "uptime_ms:" + String(millis());
}

void AnchorWatch::notify(const String& path, const String& state,
                         const String& message) {
  snapshot_.notification_path = path;
  snapshot_.notification_state = state;
  snapshot_.notification_message = message;
}
