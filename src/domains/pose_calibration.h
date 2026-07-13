#pragma once

#include <cstdint>

#include "imu_types.h"

// Boot-time pose calibration: while the splash screen shows, the wearer
// stands in the neutral pose (arms at rest) and each IMU averages its
// gravity direction. That captured "down" reference makes pose detection
// independent of how the sensor is strapped on. Pure logic, host-testable.

namespace pose_calibration {

// Acceptance criteria for a calibration window
inline constexpr uint16_t kMinSamples = 50;   // >= 1 s at 50 Hz
inline constexpr float kMinMeanG = 0.8f;      // gravity magnitude sanity
inline constexpr float kMaxMeanG = 1.2f;
inline constexpr float kMaxSpreadG = 0.35f;   // stillness: |a| spread cap
inline constexpr float kMinDirectionality = 0.6f;  // mean vs mean-magnitude

struct Reference {
  float unit[3] = {0.0f, 0.0f, 0.0f};  // calibrated "down" direction
  bool valid = false;
};

struct Calibrator {
  float sum[3] = {0.0f, 0.0f, 0.0f};
  float sum_mag = 0.0f;
  float min_mag = 1e9f;
  float max_mag = 0.0f;
  uint16_t count = 0;
};

// Accumulates one sample; invalid samples are skipped.
void feed(Calibrator& cal, const ImuSample& sample);

// Produces the down reference. Invalid when there were too few samples,
// the magnitude wasn't gravity-like, or the arm was moving.
Reference finalize(const Calibrator& cal);

// Cosine between the current gravity direction and the calibrated down:
// +1 = neutral pose, -1 = fully inverted (arm raised overhead), 0 = 90 deg.
// Returns +1 (reads as "lowered") for an invalid reference, an invalid
// sample, or near-freefall accel where direction is meaningless.
float down_dot(const Reference& ref, const ImuSample& sample);

}  // namespace pose_calibration
