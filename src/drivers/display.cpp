#include "display.h"

#include <Arduino.h>
#include <SPI.h>

#include <cmath>
#include <cstdio>

namespace {

// Vaporwave RGB565 palette, shared by the splash and telemetry screens
constexpr uint16_t kMagenta = 0xF81F;
constexpr uint16_t kMagentaDim = 0x780F;
constexpr uint16_t kCyan = 0x07FF;
constexpr uint16_t kWhite = 0xFFFF;
constexpr uint16_t kShellFill = 0x1867;  // dark blue-purple
constexpr uint16_t kFaceFill = 0x0863;   // near-black indigo

constexpr uint16_t kCyanDim = 0x0339;

// Telemetry screen layout (y coordinates)
constexpr int16_t kLimbTop = 34;      // 2x2 limb grid, 20 px row pitch
constexpr int16_t kLimbPitch = 20;
constexpr int16_t kSep1Y = 76;
constexpr int16_t kMeterTop = 82;     // 4 bar meters, 22 px pitch
constexpr int16_t kMeterPitch = 22;
constexpr int16_t kMeterBarX = 70;
constexpr int16_t kMeterBarW = 164;
constexpr int16_t kMeterBarH = 12;
constexpr int16_t kSep2Y = 168;
constexpr int16_t kCalY = 174;
constexpr int16_t kFlapY = 194;
constexpr int16_t kSysY = 214;
constexpr int16_t kUptimeY = 234;
constexpr int16_t kFooterY = 254;     // mini grid-floor deco

// Left limbs/meters are cyan; right are magenta — mirroring the speakers.
uint16_t side_accent(bool left) { return left ? kCyan : kMagenta; }
uint16_t side_accent_dim(bool left) { return left ? kCyanDim : kMagentaDim; }

}  // namespace

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

  // Calibration status: the wearer must hold the neutral pose while the
  // splash is up (see pose_calibration). Size 1 = 6 px/char, 19 chars.
  const char* status = "CALIBRATING SENSORS";
  const int16_t status_x = (config::kLcdWidth - 19 * 6) / 2;
  tft_.fillRect(0, 270, config::kLcdWidth, 10, ST77XX_BLACK);
  tft_.setTextSize(1);
  tft_.setTextColor(kCyan);
  tft_.setCursor(status_x, 271);
  tft_.print(status);
}

void Display::begin_monitor_screen() {
  tft_.fillScreen(ST77XX_BLACK);

  // Glitch title: magenta ghost under cyan text, centered (16 chars, size 2)
  const char* title = "MARK85 TELEMETRY";
  const int16_t title_x = (config::kLcdWidth - 16 * 12) / 2;
  tft_.setTextSize(2);
  tft_.setTextColor(kMagenta);
  tft_.setCursor(title_x + 1, 7);
  tft_.print(title);
  tft_.setTextColor(kCyan);
  tft_.setCursor(title_x, 6);
  tft_.print(title);
  tft_.drawFastHLine(0, 26, config::kLcdWidth, kMagenta);

  tft_.setTextSize(2);
  // Limb names, 2x2 grid (dots drawn dynamically next to them)
  const char* limbs[4] = {"L-ARM", "R-ARM", "L-LEG", "R-LEG"};
  for (uint8_t i = 0; i < 4; ++i) {
    const bool left = (i % 2) == 0;
    tft_.setTextColor(side_accent(left), ST77XX_BLACK);
    tft_.setCursor(left ? 6 : 126, kLimbTop + (i / 2) * kLimbPitch);
    tft_.print(limbs[i]);
  }

  tft_.drawFastHLine(0, kSep1Y, config::kLcdWidth, kMagentaDim);

  // Meter labels + outlines
  const char* meters[4] = {"MOV L", "MOV R", "CHG L", "CHG R"};
  for (uint8_t s = 0; s < 4; ++s) {
    const bool left = (s % 2) == 0;
    const int16_t y = kMeterTop + s * kMeterPitch;
    tft_.setTextColor(side_accent(left), ST77XX_BLACK);
    tft_.setCursor(6, y);
    tft_.print(meters[s]);
    tft_.drawRect(kMeterBarX - 1, y, kMeterBarW + 2, kMeterBarH + 2,
                  side_accent_dim(left));
  }

  tft_.drawFastHLine(0, kSep2Y, config::kLcdWidth, kMagentaDim);

  // Calibration labels (dots drawn dynamically)
  tft_.setTextColor(kWhite, ST77XX_BLACK);
  tft_.setCursor(6, kCalY);
  tft_.print("CAL");
  tft_.setTextColor(kCyan, ST77XX_BLACK);
  tft_.setCursor(66, kCalY);
  tft_.print("L");
  tft_.setTextColor(kMagenta, ST77XX_BLACK);
  tft_.setCursor(126, kCalY);
  tft_.print("R");

  // System labels (dots drawn dynamically)
  const char* systems[4] = {"MUX", "AUD", "RGB", "SRV"};
  tft_.setTextColor(kWhite, ST77XX_BLACK);
  for (uint8_t s = 0; s < 4; ++s) {
    tft_.setCursor(6 + s * 60, kSysY);
    tft_.print(systems[s]);
  }

  // Grid-floor footer: horizon glow + receding magenta lines
  tft_.drawFastHLine(0, kFooterY - 1, config::kLcdWidth, kWhite);
  tft_.drawFastHLine(0, kFooterY, config::kLcdWidth, kCyan);
  for (int16_t k = -5; k <= 5; ++k) {
    tft_.drawLine(120, kFooterY, 120 + k * 52, config::kLcdHeight - 1,
                  kMagenta);
  }
  tft_.drawFastHLine(0, kFooterY + 6, config::kLcdWidth, kMagenta);
  tft_.drawFastHLine(0, kFooterY + 14, config::kLcdWidth, kMagenta);
  tft_.drawFastHLine(0, kFooterY + 24, config::kLcdWidth, kMagenta);
}

void Display::draw_limb_status(uint8_t limb, bool online) {
  if (limb >= 4) {
    return;
  }
  const bool left = (limb % 2) == 0;
  const int16_t cx = (left ? 6 : 126) + 5 * 12 + 12;  // after the name
  const int16_t cy = kLimbTop + (limb / 2) * kLimbPitch + 6;
  tft_.fillCircle(cx, cy, 5,
                  online ? side_accent(left) : side_accent_dim(left));
}

void Display::draw_meter(uint8_t slot, float fraction) {
  if (slot >= 4) {
    return;
  }
  if (fraction < 0.0f) fraction = 0.0f;
  if (fraction > 1.0f) fraction = 1.0f;
  const bool left = (slot % 2) == 0;
  const int16_t y = kMeterTop + slot * kMeterPitch + 1;
  const int16_t fill_w = static_cast<int16_t>(fraction * kMeterBarW);
  tft_.fillRect(kMeterBarX, y, fill_w, kMeterBarH, side_accent(left));
  tft_.fillRect(kMeterBarX + fill_w, y, kMeterBarW - fill_w, kMeterBarH,
                ST77XX_BLACK);
}

void Display::draw_cal_status(bool left_ok, bool right_ok) {
  tft_.fillCircle(66 + 12 + 12, kCalY + 6, 5, left_ok ? kCyan : kCyanDim);
  tft_.fillCircle(126 + 12 + 12, kCalY + 6, 5,
                  right_ok ? kMagenta : kMagentaDim);
}

void Display::draw_system_status(bool mux, bool audio, bool rgb,
                                 bool servos) {
  const bool ok[4] = {mux, audio, rgb, servos};
  for (uint8_t s = 0; s < 4; ++s) {
    tft_.fillCircle(6 + s * 60 + 3 * 12 + 8, kSysY + 6, 4,
                    ok[s] ? kWhite : kMagentaDim);
  }
}

void Display::draw_flap_line(const char* text) {
  // 19 columns: the row starts at x=6, so 20 would overflow the panel.
  char padded[20];
  std::snprintf(padded, sizeof(padded), "%-19s", text);
  tft_.setTextSize(2);
  tft_.setTextColor(kWhite, ST77XX_BLACK);
  tft_.setCursor(6, kFlapY);
  tft_.print(padded);
}

void Display::draw_uptime_line(const char* text) {
  char padded[20];
  std::snprintf(padded, sizeof(padded), "%-19s", text);
  tft_.setTextSize(2);
  tft_.setTextColor(kWhite, ST77XX_BLACK);
  tft_.setCursor(6, kUptimeY);
  tft_.print(padded);
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
