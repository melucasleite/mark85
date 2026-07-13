#pragma once

#include <cstdint>

// Placeholder servo motion for bench-testing the flap hardware: a slow
// continuous 0 -> 180 -> 0 sweep. Will be replaced by real flap control.
// Pure logic, host-testable: time arrives as a parameter.

namespace servo_sweep {

inline constexpr uint32_t kSweepPeriodMs = 8000;  // full 0->180->0 round trip
inline constexpr float kMaxAngleDeg = 180.0f;

// Triangle wave: 0 deg at t=0, 180 deg at half period, back to 0.
float angle_at(uint32_t now_ms);

}  // namespace servo_sweep
