#pragma once

#include <cstdint>

#include "imu_types.h"

// Maps limb IMU motion to a smoothed 0..1 intensity that drives the sound
// synth. Pure logic, host-testable: time arrives as dt, samples as data.

namespace motion_intensity {

// Rotation rate treated as "full effort" — anything faster saturates at 1.
inline constexpr float kSaturationDps = 200.0f;
// Fast attack so sound starts with the movement; slower release so it
// trails off like spinning-down machinery.
inline constexpr float kAttackTauS = 0.05f;
inline constexpr float kReleaseTauS = 0.30f;

struct State {
  float level = 0.0f;  // smoothed intensity, 0..1
};

// Gyro magnitude in deg/s; 0 for invalid samples.
float sample_magnitude_dps(const ImuSample& sample);

// Raw (unsmoothed) intensity for a limb pair: the busier limb wins.
float limb_pair_intensity(const ImuSample& a, const ImuSample& b);

// Advances the smoothed level toward `target` over dt seconds and returns it.
float update(State& state, float target, float dt_s);

}  // namespace motion_intensity
