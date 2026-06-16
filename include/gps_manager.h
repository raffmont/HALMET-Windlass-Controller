#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

#include "gps_config.h"

struct GpsFix {
  bool present = false;
  bool valid = false;
  double latitude = 0.0;
  double longitude = 0.0;
  double sog_m_s = 0.0;
  double cog_deg = 0.0;
  float hdop = 99.9f;
  uint8_t satellites = 0;
  uint32_t last_fix_ms = 0;
  uint32_t stable_count = 0;
  String interface = "none";
  String fix_type = "none";
  String last_fix_time = "";
};

class GpsManager {
 public:
  explicit GpsManager(GpsConfig* config);
  void begin();
  void update();
  const GpsFix& fix() const { return fix_; }
  bool hasUsableFix() const;

 private:
  void beginUart(uint32_t baud);
  void processByte(char c);
  void processSentence(const String& sentence);
  void processRmc(const String& sentence);
  void processGga(const String& sentence);
  String field(const String& sentence, int index) const;
  double parseNmeaCoordinate(const String& value, const String& hemi) const;
  bool sentenceChecksumOk(const String& sentence) const;
  void updateQualityGate();

  GpsConfig* config_;
  HardwareSerial* serial_ = nullptr;
  GpsFix fix_;
  String sentence_;
  uint8_t baud_index_ = 0;
  uint32_t last_scan_ms_ = 0;
  uint32_t current_baud_ = 0;
};
