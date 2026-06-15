#pragma once

// Default Signal K paths. Runtime values are editable in the SensESP web UI
// under /Windlass/Configuration and are copied into RuntimeConfig at boot.
static constexpr const char* DEFAULT_SK_RODE_LENGTH =
    "anchoring.windlass.rode.length";
static constexpr const char* DEFAULT_SK_RODE_SPEED =
    "anchoring.windlass.rode.speed";
static constexpr const char* DEFAULT_SK_DIRECTION =
    "anchoring.windlass.direction";
static constexpr const char* DEFAULT_SK_STATE = "anchoring.windlass.state";
static constexpr const char* DEFAULT_SK_COMMAND_STATUS =
    "anchoring.windlass.command.status";
static constexpr const char* DEFAULT_SK_COMMAND_REQUEST =
    "anchoring.windlass.command.request";
static constexpr const char* DEFAULT_SK_PULSES =
    "anchoring.windlass.counter.pulses";
static constexpr const char* DEFAULT_SK_METERS_PER_PULSE =
    "anchoring.windlass.counter.metersPerPulse";
static constexpr const char* DEFAULT_SK_FAULTS = "anchoring.windlass.faults";
static constexpr const char* DEFAULT_SK_MODE = "anchoring.windlass.mode";
static constexpr const char* DEFAULT_SK_FREEFALL_DETECTED =
    "anchoring.windlass.freeFall.detected";
static constexpr const char* DEFAULT_SK_ANCHOR_DETECTED =
    "anchoring.windlass.anchor.detected";
static constexpr const char* DEFAULT_SK_SEAFLOOR_DETECTED =
    "anchoring.windlass.anchor.seafloorDetected";
static constexpr const char* DEFAULT_SK_EVENT = "anchoring.windlass.event";
static constexpr const char* DEFAULT_SK_NOTIFICATION =
    "notifications.anchoring.windlass";
