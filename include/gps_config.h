#pragma once

#include <Arduino.h>

struct GpsConfig {
  String mode = "auto";
  int rx_pin = -1;
  int tx_pin = -1;
  uint32_t baud = 9600;
  bool publish_navigation_position = false;
  uint8_t min_satellites = 5;
  float max_hdop = 2.5f;
  uint32_t max_fix_age_ms = 10000;
  uint8_t stable_samples = 5;
};
