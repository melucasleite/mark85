#pragma once

#include <Adafruit_NeoPixel.h>

#include <cstddef>
#include <cstdint>

#include "../config.h"
#include "../domains/rgb_types.h"

// Two independent HW-159 jewels (7x WS2812 each): left on D6, right on D12.
// RMT-backed writes; a 7-pixel frame takes ~250 us.

class RgbLeds {
 public:
  RgbLeds();

  // Initializes both strips dark. WS2812 is write-only, so presence can't
  // be verified; returns false only on driver init failure.
  bool begin();

  void show_left(const RgbColor* pixels, size_t count);
  void show_right(const RgbColor* pixels, size_t count);

 private:
  static void show(Adafruit_NeoPixel& strip, const RgbColor* pixels,
                   size_t count);

  Adafruit_NeoPixel left_;
  Adafruit_NeoPixel right_;
};
