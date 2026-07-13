#include "rgb_leds.h"

RgbLeds::RgbLeds()
    : left_(config::kRgbLedCount, config::kRgbLeftPin, NEO_GRB + NEO_KHZ800),
      right_(config::kRgbLedCount, config::kRgbRightPin,
             NEO_GRB + NEO_KHZ800) {}

bool RgbLeds::begin() {
  left_.begin();
  right_.begin();
  left_.clear();
  right_.clear();
  left_.show();
  right_.show();
  return true;
}

void RgbLeds::show_left(const RgbColor* pixels, size_t count) {
  show(left_, pixels, count);
}

void RgbLeds::show_right(const RgbColor* pixels, size_t count) {
  show(right_, pixels, count);
}

void RgbLeds::show(Adafruit_NeoPixel& strip, const RgbColor* pixels,
                   size_t count) {
  const size_t n =
      count < config::kRgbLedCount ? count : config::kRgbLedCount;
  for (size_t i = 0; i < n; ++i) {
    strip.setPixelColor(i, pixels[i].r, pixels[i].g, pixels[i].b);
  }
  strip.show();
}
