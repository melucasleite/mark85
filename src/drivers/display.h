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

  // Full-screen boot splash: "JARVIS" / Iron Man helmet / "MARK 85",
  // with the firmware version in the top-left corner.
  void draw_splash(const char* version);

  // Animated splash floor frame: the perspective grid scrolls toward the
  // viewer. Call at ~30 Hz while the splash is up; redraws only the region
  // below the horizon (including the MARK 85 and CALIBRATING overlays).
  void draw_splash_floor(uint32_t now_ms);

  // Clears to black and restores the monitor text style.
  void clear();

  // Telemetry screen chrome, drawn once: title, section separators, static
  // labels for limbs/meters/cal/systems, and the grid-floor footer.
  void begin_monitor_screen();

  // Limb online dot in a 2x2 grid (0 L-ARM, 1 R-ARM, 2 L-LEG, 3 R-LEG).
  // Bright accent = online, dim = offline; cyan left, magenta right.
  void draw_limb_status(uint8_t limb, bool online);

  // Horizontal bar meter, slot 0..3: MOV L, MOV R, CHG L, CHG R.
  void draw_meter(uint8_t slot, float fraction);

  // Per-arm calibration dots (bright = calibrated).
  void draw_cal_status(bool left_ok, bool right_ok);

  // Boot-health dots for mux / audio / rgb / servos.
  void draw_system_status(bool mux, bool audio, bool rgb, bool servos);

  // Full-width white dynamic text rows (padded in place, no flicker).
  void draw_flap_line(const char* text);
  void draw_uptime_line(const char* text);

 private:
  static constexpr uint8_t kTextSize = 2;
  static constexpr uint8_t kRowHeightPx = 16;
  static constexpr uint8_t kMaxRows = config::kLcdHeight / kRowHeightPx;

  // Chromatic-aberration title: magenta ghost offset under the main text.
  void glitch_print(const char* s, uint8_t size, int16_t x, int16_t y,
                    uint16_t color);

  Adafruit_ST7789 tft_;
};
