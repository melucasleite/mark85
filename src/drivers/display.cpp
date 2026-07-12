#include "display.h"

#include <Arduino.h>
#include <SPI.h>

#include <cstdio>

Display::Display()
    : tft_(config::kLcdCsPin, config::kLcdDcPin, config::kLcdRstPin) {}

bool Display::begin() {
  pinMode(config::kLcdBacklightPin, OUTPUT);
  digitalWrite(config::kLcdBacklightPin, HIGH);

  SPI.begin(config::kLcdSckPin, -1 /* no MISO */, config::kLcdMosiPin, -1);
  tft_.init(config::kLcdWidth, config::kLcdHeight);
  tft_.setSPISpeed(config::kLcdSpiHz);
  tft_.setRotation(0);
  tft_.fillScreen(ST77XX_BLACK);
  tft_.setTextSize(kTextSize);
  tft_.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
  tft_.setTextWrap(false);
  return true;
}

void Display::draw_line(uint8_t row, const char* text) {
  if (row >= kMaxRows) {
    return;
  }
  // Pad to the full 20-column width so the previous row content is
  // overwritten without an explicit (flickery) clear.
  char padded[21];
  std::snprintf(padded, sizeof(padded), "%-20s", text);
  tft_.setCursor(0, row * kRowHeightPx);
  tft_.print(padded);
}
