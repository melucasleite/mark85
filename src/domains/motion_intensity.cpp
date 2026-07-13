#include "motion_intensity.h"

#include <cmath>

namespace motion_intensity {

namespace {
constexpr float kGyroLsbPerDps = 131.0f;  // MPU-6050 at +/-250 dps
}

float sample_magnitude_dps(const ImuSample& sample) {
  if (!sample.valid) {
    return 0.0f;
  }
  const float x = sample.gyro_raw[0] / kGyroLsbPerDps;
  const float y = sample.gyro_raw[1] / kGyroLsbPerDps;
  const float z = sample.gyro_raw[2] / kGyroLsbPerDps;
  return std::sqrt(x * x + y * y + z * z);
}

float limb_pair_intensity(const ImuSample& a, const ImuSample& b) {
  const float mag_a = sample_magnitude_dps(a);
  const float mag_b = sample_magnitude_dps(b);
  const float mag = mag_a > mag_b ? mag_a : mag_b;
  const float intensity = mag / kSaturationDps;
  return intensity > 1.0f ? 1.0f : intensity;
}

float update(State& state, float target, float dt_s) {
  const float tau = target > state.level ? kAttackTauS : kReleaseTauS;
  const float coeff = 1.0f - std::exp(-dt_s / tau);
  state.level += (target - state.level) * coeff;
  return state.level;
}

}  // namespace motion_intensity
