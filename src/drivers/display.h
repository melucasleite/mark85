#pragma once

#include <Adafruit_ST7789.h>

#include <cstdint>

#include "../config.h"

// Waveshare 1.69" ST7789V2 (240x280) over SPI.
// Exposes a fixed-grid text API: 20 columns x 17 rows at text size 2.
// Lines are padded to full width on draw, so redrawing a row cleanly
// overwrites its previous content (no flicker, no explicit clearing).

class Display {
 public:
  Display();

  // Initializes SPI and the panel, turns on the backlight, clears to black.
  // SPI is write-only so panel presence can't be verified; returns false
  // only if init obviously failed.
  bool begin();

  // Draws `text` at the given row (0..16), padded/truncated to 20 chars.
  void draw_line(uint8_t row, const char* text);

 private:
  static constexpr uint8_t kTextSize = 2;
  static constexpr uint8_t kRowHeightPx = 16;
  static constexpr uint8_t kMaxRows = config::kLcdHeight / kRowHeightPx;

  Adafruit_ST7789 tft_;
};
