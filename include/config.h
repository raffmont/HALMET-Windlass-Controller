#pragma once

#ifndef HALMET_WINDLASS_VERSION
#define HALMET_WINDLASS_VERSION "0.4.0"
#endif

// Board pins. Override in platformio.ini if your HALMET revision/example exposes
// different names.
#ifndef GP2_SENSOR_PIN
#define GP2_SENSOR_PIN 34
#endif
#ifndef WINDLASS_UP_SENSE_PIN
#define WINDLASS_UP_SENSE_PIN 35
#endif
#ifndef WINDLASS_DOWN_SENSE_PIN
#define WINDLASS_DOWN_SENSE_PIN 32
#endif
#ifndef WINDLASS_UP_RELAY_PIN
#define WINDLASS_UP_RELAY_PIN 17
#endif
#ifndef WINDLASS_DOWN_RELAY_PIN
#define WINDLASS_DOWN_RELAY_PIN 16
#endif
#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN 2
#endif

// ESP32 TWAI/CAN pins routed to the NMEA 2000 transceiver.
#ifndef ESP32_CAN_TX_PIN
#define ESP32_CAN_TX_PIN GPIO_NUM_5
#endif
#ifndef ESP32_CAN_RX_PIN
#define ESP32_CAN_RX_PIN GPIO_NUM_4
#endif

// External high-level-trigger relay module, driven by non-isolated HALMET
// GPIOs. Runtime-configurable in SensESP setup; default energizes a relay when
// the GPIO is HIGH.
#define DEFAULT_RELAY_ACTIVE_HIGH 1
#define SENSE_ACTIVE_HIGH 1

// GP2 dry contact: normally open, closes once per gypsy turn; 1 turn = 33 cm.
static constexpr double DEFAULT_METERS_PER_PULSE = 0.33;
static constexpr uint32_t DEFAULT_CHAIN_PULSE_DEBOUNCE_MS = 75;
static constexpr uint32_t SPEED_WINDOW_MS = 1000;

// Free-fall and anchor event detection.
static constexpr double DEFAULT_FREEFALL_MIN_SPEED_M_S = 0.20;
static constexpr uint32_t DEFAULT_FREEFALL_MIN_PULSES = 2;
static constexpr uint32_t DEFAULT_SEAFLOOR_NO_PULSE_MS = 1800;
static constexpr double DEFAULT_SEAFLOOR_MIN_LENGTH_M = 1.00;
static constexpr double DEFAULT_ANCHOR_DETECTED_LENGTH_M = 0.33;

// Safety timing.
static constexpr uint32_t DEFAULT_COMMAND_DEADMAN_MS = 1200;
static constexpr uint32_t MAX_RELAY_ON_MS = 15000;
static constexpr uint32_t DEFAULT_STALL_DETECT_MS = 3500;
static constexpr double DEFAULT_MIN_SAFE_LENGTH_M = 0.50;

// Publishing.
static constexpr uint32_t SIGNALK_PUBLISH_PERIOD_MS = 1000;
static constexpr uint32_t N2K_PUBLISH_PERIOD_MS = 1000;

#ifndef DEVICE_INSTANCE
#define DEVICE_INSTANCE 42
#endif
