#pragma once

#include <cstdint>

#include "../config.h"

// 4x hobby servo on LEDC PWM (50 Hz). Angles map linearly to the
// 500-2500 us pulse range; out-of-range angles are clamped.

class Servos {
 public:
  // Configures one LEDC channel per servo. False if any channel fails.
  bool begin();

  // Commands one servo to an angle in degrees (clamped to 0..180).
  void set_angle(uint8_t index, float degrees);

 private:
  bool ready_ = false;
};
