#include "gps_manager.h"

#include <cmath>

namespace {
constexpr uint32_t kScanBauds[] = {9600, 38400, 57600, 115200};
constexpr size_t kScanBaudCount = sizeof(kScanBauds) / sizeof(kScanBauds[0]);

bool isGnssSentence(const String& sentence) {
  return sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC") ||
         sentence.startsWith("$GPGGA") || sentence.startsWith("$GNGGA") ||
         sentence.startsWith("$GPGSA") || sentence.startsWith("$GNGSA");
}
}  // namespace

GpsManager::GpsManager(GpsConfig* config) : config_(config) {}

void GpsManager::begin() {
  if (!config_ || config_->mode == "disabled" || config_->rx_pin < 0) {
    fix_.interface = "disabled";
    return;
  }
  serial_ = &Serial2;
  beginUart(config_->baud);
}

void GpsManager::beginUart(uint32_t baud) {
  current_baud_ = baud;
  serial_->end();
  if (config_->tx_pin >= 0) {
    serial_->begin(baud, SERIAL_8N1, config_->rx_pin, config_->tx_pin);
  } else {
    serial_->begin(baud, SERIAL_8N1, config_->rx_pin, -1);
  }
  fix_.interface = "uart";
  last_scan_ms_ = millis();
}

void GpsManager::update() {
  if (!serial_ || config_->mode == "disabled" || config_->rx_pin < 0) return;

  while (serial_->available() > 0) {
    processByte(static_cast<char>(serial_->read()));
  }

  const uint32_t now = millis();
  if ((config_->mode == "auto" || config_->baud == 0) && !fix_.present &&
      now - last_scan_ms_ > 2500) {
    baud_index_ = (baud_index_ + 1) % kScanBaudCount;
    beginUart(kScanBauds[baud_index_]);
  }

  if (fix_.last_fix_ms != 0 && now - fix_.last_fix_ms > config_->max_fix_age_ms) {
    fix_.valid = false;
    fix_.fix_type = "stale";
    fix_.stable_count = 0;
  }
}

bool GpsManager::hasUsableFix() const {
  if (!config_) return false;
  return fix_.present && fix_.valid && fix_.stable_count >= config_->stable_samples &&
         fix_.satellites >= config_->min_satellites &&
         fix_.hdop <= config_->max_hdop &&
         millis() - fix_.last_fix_ms <= config_->max_fix_age_ms;
}

void GpsManager::processByte(char c) {
  if (c == '$') sentence_ = "$";
  else if (c == '\n' || c == '\r') {
    if (sentence_.length() > 6) processSentence(sentence_);
    sentence_ = "";
  } else if (sentence_.length() > 0 && sentence_.length() < 120) {
    sentence_ += c;
  }
}

void GpsManager::processSentence(const String& sentence) {
  if (!isGnssSentence(sentence) || !sentenceChecksumOk(sentence)) return;
  fix_.present = true;
  config_->baud = current_baud_;
  if (sentence.startsWith("$GPRMC") || sentence.startsWith("$GNRMC")) {
    processRmc(sentence);
  } else if (sentence.startsWith("$GPGGA") || sentence.startsWith("$GNGGA")) {
    processGga(sentence);
  }
  updateQualityGate();
}

void GpsManager::processRmc(const String& sentence) {
  if (field(sentence, 2) != "A") return;
  const double lat = parseNmeaCoordinate(field(sentence, 3), field(sentence, 4));
  const double lon = parseNmeaCoordinate(field(sentence, 5), field(sentence, 6));
  if (lat == 0.0 && lon == 0.0) return;
  fix_.latitude = lat;
  fix_.longitude = lon;
  fix_.sog_m_s = field(sentence, 7).toDouble() * 0.514444;
  fix_.cog_deg = field(sentence, 8).toDouble();
  fix_.last_fix_ms = millis();
  fix_.last_fix_time = field(sentence, 1);
}

void GpsManager::processGga(const String& sentence) {
  const int quality = field(sentence, 6).toInt();
  const double lat = parseNmeaCoordinate(field(sentence, 2), field(sentence, 3));
  const double lon = parseNmeaCoordinate(field(sentence, 4), field(sentence, 5));
  if (quality <= 0 || (lat == 0.0 && lon == 0.0)) return;
  fix_.latitude = lat;
  fix_.longitude = lon;
  fix_.satellites = static_cast<uint8_t>(field(sentence, 7).toInt());
  fix_.hdop = field(sentence, 8).toFloat();
  fix_.last_fix_ms = millis();
  fix_.last_fix_time = field(sentence, 1);
  fix_.fix_type = quality > 1 ? "dgps" : "gps";
}

String GpsManager::field(const String& sentence, int index) const {
  int start = 0;
  int end = -1;
  for (int i = 0; i <= index; i++) {
    start = end + 1;
    end = sentence.indexOf(',', start);
    const int star = sentence.indexOf('*', start);
    if (end < 0 || (star >= 0 && star < end)) end = star;
    if (end < 0) end = sentence.length();
  }
  return sentence.substring(start, end);
}

double GpsManager::parseNmeaCoordinate(const String& value,
                                       const String& hemi) const {
  if (value.length() < 4) return 0.0;
  const double raw = value.toDouble();
  const int degrees = static_cast<int>(raw / 100.0);
  const double minutes = raw - degrees * 100.0;
  double decimal = degrees + minutes / 60.0;
  if (hemi == "S" || hemi == "W") decimal = -decimal;
  return decimal;
}

bool GpsManager::sentenceChecksumOk(const String& sentence) const {
  const int star = sentence.indexOf('*');
  if (star < 0) return true;
  uint8_t checksum = 0;
  for (int i = 1; i < star; i++) checksum ^= static_cast<uint8_t>(sentence[i]);
  const uint8_t expected =
      static_cast<uint8_t>(strtoul(sentence.substring(star + 1).c_str(), nullptr, 16));
  return checksum == expected;
}

void GpsManager::updateQualityGate() {
  const bool quality_ok =
      fix_.satellites >= config_->min_satellites && fix_.hdop <= config_->max_hdop;
  if (fix_.last_fix_ms != 0 && quality_ok) {
    fix_.valid = true;
    fix_.stable_count++;
  } else {
    fix_.valid = false;
    fix_.stable_count = 0;
    if (fix_.fix_type == "none") fix_.fix_type = "poor";
  }
}
