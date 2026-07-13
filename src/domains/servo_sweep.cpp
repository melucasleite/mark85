#include "servo_sweep.h"

namespace servo_sweep {

float angle_at(uint32_t now_ms) {
  const uint32_t t = now_ms % kSweepPeriodMs;
  const float phase = static_cast<float>(t) / kSweepPeriodMs;  // 0..1
  const float tri = phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;
  return kMaxAngleDeg * tri;
}

}  // namespace servo_sweep
