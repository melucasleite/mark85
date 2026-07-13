#pragma once

#include <cstdint>

// Plain color type shared between domains and the RGB driver.
// Host-safe: no Arduino or hardware includes.

struct RgbColor {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
};
