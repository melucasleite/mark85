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

void Display::clear() {
  tft_.fillScreen(ST77XX_BLACK);
  tft_.setTextSize(kTextSize);
  tft_.setTextColor(ST77XX_GREEN, ST77XX_BLACK);
}

void Display::draw_splash() {
  // RGB565 palette for the suit
  constexpr uint16_t kSuitRed = 0xC800;
  constexpr uint16_t kSuitRedDark = 0x7000;
  constexpr uint16_t kSuitGold = 0xFD40;
  constexpr uint16_t kEyeGlow = 0xFFFF;
  constexpr uint16_t kAccent = 0x07FF;

  tft_.fillScreen(ST77XX_BLACK);

  // "JARVIS" — size 4 = 24 px/char, 6 chars = 144 px wide, centered
  tft_.setTextColor(kAccent, ST77XX_BLACK);
  tft_.setTextSize(4);
  tft_.setCursor((config::kLcdWidth - 6 * 24) / 2, 14);
  tft_.print("JARVIS");

  // Helmet: red shell with gold faceplate, centered at x=120
  tft_.fillRoundRect(68, 62, 104, 126, 34, kSuitRed);
  tft_.fillRoundRect(84, 94, 72, 82, 14, kSuitGold);
  // Brow strip across the faceplate
  tft_.fillRect(84, 110, 72, 4, kSuitRed);
  // Eyes: white slits with a glow outline
  tft_.fillRoundRect(93, 122, 22, 8, 2, kEyeGlow);
  tft_.fillRoundRect(125, 122, 22, 8, 2, kEyeGlow);
  tft_.drawRoundRect(92, 121, 24, 10, 3, kAccent);
  tft_.drawRoundRect(124, 121, 24, 10, 3, kAccent);
  // Mouth slit and chin line
  tft_.fillRect(108, 154, 24, 4, kSuitRedDark);
  tft_.drawFastVLine(120, 162, 12, kSuitRedDark);

  // Arc reactor below the helmet
  tft_.fillCircle(120, 216, 12, kAccent);
  tft_.fillCircle(120, 216, 6, kEyeGlow);
  tft_.drawCircle(120, 216, 16, kSuitRedDark);

  // "MARK 85" — size 3 = 18 px/char, 7 chars = 126 px wide, centered
  tft_.setTextColor(kSuitGold, ST77XX_BLACK);
  tft_.setTextSize(3);
  tft_.setCursor((config::kLcdWidth - 7 * 18) / 2, 244);
  tft_.print("MARK 85");
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
