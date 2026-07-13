#include "display.h"

#include <Arduino.h>
#include <SPI.h>

#include <cmath>
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

void Display::clear() {
  tft_.fillScreen(ST77XX_BLACK);
  tft_.setTextSize(kTextSize);
  tft_.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
}

void Display::draw_splash() {
  // Vaporwave RGB565 palette
  constexpr uint16_t kMagenta = 0xF81F;
  constexpr uint16_t kMagentaDim = 0x780F;
  constexpr uint16_t kCyan = 0x07FF;
  constexpr uint16_t kWhite = 0xFFFF;
  constexpr uint16_t kShellFill = 0x1867;   // dark blue-purple
  constexpr uint16_t kFaceFill = 0x0863;    // near-black indigo
  constexpr int16_t kHorizonY = 172;

  tft_.fillScreen(ST77XX_BLACK);

  // Dusk gradient sky: deep purple fading to black toward the horizon
  constexpr uint16_t kSky[] = {0x3009, 0x2807, 0x2006, 0x1804, 0x1003, 0x0802};
  for (int16_t i = 0; i < 6; ++i) {
    tft_.fillRect(0, i * 12, config::kLcdWidth, 12, kSky[i]);
  }

  // Vaporwave sun on the horizon: yellow-to-magenta bands with slit gaps
  constexpr int16_t kSunCx = 120;
  constexpr int16_t kSunCy = 150;
  constexpr int16_t kSunR = 55;
  constexpr uint16_t kSun[] = {0xFFE0, 0xFEC0, 0xFDA0, 0xFC80,
                               0xFB81, 0xFA53, 0xF936, 0xF81F};
  for (int16_t y = kSunCy - kSunR; y < kHorizonY - 1; y += 2) {
    // Growing horizontal slits through the lower half
    if (y >= kSunCy - 6 && ((y / 4) % 2) == 1) {
      continue;
    }
    const int16_t dy = y - kSunCy;
    const int16_t half = static_cast<int16_t>(
        sqrtf(static_cast<float>(kSunR * kSunR - dy * dy)));
    const int16_t idx = (y - (kSunCy - kSunR)) * 8 / (2 * kSunR);
    tft_.fillRect(kSunCx - half, y, 2 * half, 2, kSun[idx < 7 ? idx : 7]);
  }

  // Helmet: dark silhouette with neon outlines, floating over the sun
  tft_.fillRoundRect(78, 56, 84, 96, 26, kShellFill);
  tft_.drawRoundRect(76, 54, 88, 100, 28, kMagentaDim);  // outer glow
  tft_.drawRoundRect(77, 55, 86, 98, 27, kMagenta);
  tft_.fillRoundRect(92, 82, 56, 58, 10, kFaceFill);
  tft_.drawRoundRect(91, 81, 58, 60, 11, kCyan);
  tft_.fillRect(92, 96, 56, 3, kMagenta);  // brow strip
  // Eyes: white slits in cyan glow frames
  tft_.fillRoundRect(98, 104, 18, 6, 2, kWhite);
  tft_.drawRoundRect(97, 103, 20, 8, 3, kCyan);
  tft_.fillRoundRect(124, 104, 18, 6, 2, kWhite);
  tft_.drawRoundRect(123, 103, 20, 8, 3, kCyan);
  tft_.fillRect(112, 128, 16, 3, kCyan);  // mouth slit

  // Arc reactor between chin and horizon
  tft_.drawCircle(120, 162, 9, kMagentaDim);
  tft_.fillCircle(120, 162, 6, kCyan);
  tft_.fillCircle(120, 162, 3, kWhite);

  // Horizon glow line
  tft_.drawFastHLine(0, kHorizonY - 1, config::kLcdWidth, kWhite);
  tft_.drawFastHLine(0, kHorizonY, config::kLcdWidth, kCyan);

  // Perspective grid floor: rays from the vanishing point + horizontals
  for (int16_t k = -5; k <= 5; ++k) {
    tft_.drawLine(120, kHorizonY, 120 + k * 52, config::kLcdHeight - 1,
                  kMagenta);
  }
  constexpr int16_t kGridRows[] = {176, 182, 190, 200, 213, 230, 251, 276};
  for (const int16_t y : kGridRows) {
    tft_.drawFastHLine(0, y, config::kLcdWidth, kMagenta);
  }

  // Chromatic-aberration titles: magenta ghost offset under cyan text
  const auto glitch_text = [this](const char* s, uint8_t size, int16_t x,
                                  int16_t y, uint16_t main_color) {
    tft_.setTextSize(size);
    tft_.setTextColor(kMagenta);
    tft_.setCursor(x + 2, y + 2);
    tft_.print(s);
    tft_.setTextColor(main_color);
    tft_.setCursor(x, y);
    tft_.print(s);
  };
  glitch_text("JARVIS", 4, (config::kLcdWidth - 6 * 24) / 2, 12, kCyan);
  glitch_text("MARK 85", 3, (config::kLcdWidth - 7 * 18) / 2, 244, kWhite);
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
