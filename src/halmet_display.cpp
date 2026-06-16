
#include "halmet_display.h"

#include <WiFi.h>

namespace halmet {

// OLED display width and height, in pixels
const int kScreenWidth = 128;
const int kScreenHeight = 64;
const int kTextRows = kScreenHeight / 8;

String displayed_rows[kTextRows];

void PrintRowIfChanged(Adafruit_SSD1306* display, int row, const String& text) {
  if (row < 0 || row >= kTextRows || displayed_rows[row] == text) return;

  ClearRow(display, row);
  display->setCursor(0, 8 * row);
  display->print(text);
  display->display();
  displayed_rows[row] = text;
}

bool InitializeSSD1306(const std::shared_ptr<sensesp::SensESPBaseApp> sensesp_app,
                       Adafruit_SSD1306** display, TwoWire* i2c) {
  *display = new Adafruit_SSD1306(kScreenWidth, kScreenHeight, i2c, -1);
  bool init_successful = (*display)->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!init_successful) {
    debugD("SSD1306 allocation failed");
    return false;
  }
  delay(100);
  (*display)->setRotation(2);
  (*display)->clearDisplay();
  (*display)->setTextSize(1);
  (*display)->setTextColor(SSD1306_WHITE);
  (*display)->setCursor(0, 0);
  displayed_rows[0] = "Host: " + sensesp_app->get_hostname();
  (*display)->print(displayed_rows[0]);
  (*display)->display();

  return true;
}

/// Clear a text row on an Adafruit graphics display
void ClearRow(Adafruit_SSD1306* display, int row) {
  display->fillRect(0, 8 * row, kScreenWidth, 8, 0);
}

void PrintValue(Adafruit_SSD1306* display, int row, String title, float value) {
  char value_text[16];
  snprintf(value_text, sizeof(value_text), "%.1f", value);
  PrintRowIfChanged(display, row, title + " " + String(value_text));
}

void PrintValue(Adafruit_SSD1306* display, int row, String title,
                String value) {
  PrintRowIfChanged(display, row, title + " " + value);
}

}  // namespace halmet
