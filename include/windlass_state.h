#pragma once

#include <Arduino.h>

#include "config.h"

enum class WindlassDirection : uint8_t {
  Stopped = 0,
  Retrieving = 1,
  Deploying = 2,
  Unknown = 3
};

enum class WindlassCommand : uint8_t { Stop = 0, Up = 1, Down = 2 };

enum class WindlassMode : uint8_t {
  Idle = 0,
  Retrieving = 1,
  Deploying = 2,
  FreeFall = 3,
  Seafloor = 4,
  Fault = 5
};

enum class WindlassEvent : uint8_t {
  None = 0,
  AnchorDetected = 1,
  FreeFallDetected = 2,
  SeafloorDetected = 3,
  AnchorAlarmSuggested = 4
};

enum WindlassFault : uint32_t {
  FaultNone = 0,
  FaultCommandTimeout = 1UL << 0,
  FaultMutuallyExclusiveCommand = 1UL << 1,
  FaultNoPulsesWhileRunning = 1UL << 2,
  FaultNearZeroLimit = 1UL << 3,
  FaultNmea2000Error = 1UL << 4,
  FaultSignalKDisconnected = 1UL << 5,
  FaultWifiDisconnected = 1UL << 6,
  FaultInvalidCommand = 1UL << 7,
  FaultFreeFallDetected = 1UL << 8
};

struct ChainCounterState {
  volatile int64_t pulses_total = 0;
  volatile uint32_t last_pulse_ms = 0;
  int64_t last_speed_pulses = 0;
  uint32_t last_speed_ms = 0;
  double meters_per_pulse = DEFAULT_METERS_PER_PULSE;
  double rode_length_m = 0.0;
  double rode_speed_m_s = 0.0;
  WindlassDirection direction = WindlassDirection::Stopped;
  WindlassCommand active_command = WindlassCommand::Stop;
  uint32_t fault_flags = FaultNone;
  uint32_t command_started_ms = 0;
  uint32_t command_refreshed_ms = 0;
  bool manual_up = false;
  bool manual_down = false;
  bool n2k_started = false;

  WindlassMode mode = WindlassMode::Idle;
  WindlassEvent last_event = WindlassEvent::None;
  bool free_fall_active = false;
  bool free_fall_latched = false;
  bool anchor_detected = false;
  bool seafloor_detected = false;
  bool anchor_alarm_suggested = false;
  bool event_dirty = false;
  int64_t last_event_pulses = 0;
  uint32_t last_event_ms = 0;
  uint32_t deployment_started_ms = 0;
  int64_t deployment_started_pulses = 0;
  uint32_t moving_without_command_started_ms = 0;
};

inline const char* direction_to_string(WindlassDirection direction) {
  switch (direction) {
    case WindlassDirection::Retrieving:
      return "retrieving";
    case WindlassDirection::Deploying:
      return "deploying";
    case WindlassDirection::Stopped:
      return "stopped";
    default:
      return "unknown";
  }
}

inline const char* mode_to_string(WindlassMode mode) {
  switch (mode) {
    case WindlassMode::Retrieving:
      return "retrieving";
    case WindlassMode::Deploying:
      return "deploying";
    case WindlassMode::FreeFall:
      return "free_fall";
    case WindlassMode::Seafloor:
      return "seafloor";
    case WindlassMode::Fault:
      return "fault";
    default:
      return "idle";
  }
}

inline const char* event_to_string(WindlassEvent event) {
  switch (event) {
    case WindlassEvent::AnchorDetected:
      return "anchor_detected";
    case WindlassEvent::FreeFallDetected:
      return "free_fall_detected";
    case WindlassEvent::SeafloorDetected:
      return "seafloor_detected";
    case WindlassEvent::AnchorAlarmSuggested:
      return "anchor_alarm_suggested";
    default:
      return "none";
  }
}

inline const char* command_to_string(WindlassCommand command) {
  switch (command) {
    case WindlassCommand::Up:
      return "up";
    case WindlassCommand::Down:
      return "down";
    default:
      return "stop";
  }
}

inline bool parse_command_from_string(const String& value,
                                      WindlassCommand& command) {
  if (value == "up" || value == "retrieve" || value == "retrieving") {
    command = WindlassCommand::Up;
    return true;
  }
  if (value == "down" || value == "deploy" || value == "deploying") {
    command = WindlassCommand::Down;
    return true;
  }
  if (value == "stop") {
    command = WindlassCommand::Stop;
    return true;
  }
  return false;
}

inline WindlassCommand command_from_string(const String& value) {
  WindlassCommand command = WindlassCommand::Stop;
  parse_command_from_string(value, command);
  return command;
}
