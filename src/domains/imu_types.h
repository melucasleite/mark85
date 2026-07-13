#pragma once

#include <cstdint>

// Plain data types shared between drivers and domains.
// Host-safe: no Arduino or hardware includes.

struct ImuSample {
  int16_t accel_raw[3] = {0, 0, 0};  // x, y, z at +/-2 g full scale
  int16_t gyro_raw[3] = {0, 0, 0};   // x, y, z at +/-250 dps full scale
  bool valid = false;                // false: sensor missing or read failed
};
