#include "pose_calibration.h"

#include <cmath>

namespace pose_calibration {

namespace {
constexpr float kAccelLsbPerG = 16384.0f;
constexpr float kMinUsableG = 0.3f;  // below this, direction is meaningless
}  // namespace

void feed(Calibrator& cal, const ImuSample& sample) {
  if (!sample.valid) {
    return;
  }
  const float x = sample.accel_raw[0] / kAccelLsbPerG;
  const float y = sample.accel_raw[1] / kAccelLsbPerG;
  const float z = sample.accel_raw[2] / kAccelLsbPerG;
  const float mag = std::sqrt(x * x + y * y + z * z);

  cal.sum[0] += x;
  cal.sum[1] += y;
  cal.sum[2] += z;
  cal.sum_mag += mag;
  if (mag < cal.min_mag) cal.min_mag = mag;
  if (mag > cal.max_mag) cal.max_mag = mag;
  ++cal.count;
}

Reference finalize(const Calibrator& cal) {
  Reference ref;
  if (cal.count < kMinSamples) {
    return ref;
  }
  const float mean_mag = cal.sum_mag / cal.count;
  if (mean_mag < kMinMeanG || mean_mag > kMaxMeanG) {
    return ref;  // not gravity-like: wrong range or broken sensor
  }
  if (cal.max_mag - cal.min_mag > kMaxSpreadG) {
    return ref;  // arm was moving during the window
  }

  const float mx = cal.sum[0] / cal.count;
  const float my = cal.sum[1] / cal.count;
  const float mz = cal.sum[2] / cal.count;
  const float mean_len = std::sqrt(mx * mx + my * my + mz * mz);
  // A wandering direction averages toward zero even if magnitudes look calm.
  if (mean_len < kMinDirectionality * mean_mag) {
    return ref;
  }

  ref.unit[0] = mx / mean_len;
  ref.unit[1] = my / mean_len;
  ref.unit[2] = mz / mean_len;
  ref.valid = true;
  return ref;
}

float down_dot(const Reference& ref, const ImuSample& sample) {
  if (!ref.valid || !sample.valid) {
    return 1.0f;
  }
  const float x = sample.accel_raw[0] / kAccelLsbPerG;
  const float y = sample.accel_raw[1] / kAccelLsbPerG;
  const float z = sample.accel_raw[2] / kAccelLsbPerG;
  const float mag = std::sqrt(x * x + y * y + z * z);
  if (mag < kMinUsableG) {
    return 1.0f;
  }
  return (x * ref.unit[0] + y * ref.unit[1] + z * ref.unit[2]) / mag;
}

}  // namespace pose_calibration
