#pragma once

#include <cstddef>
#include <cstdint>

#include "imu_types.h"
#include "motion_intensity.h"
#include "sound_synth.h"

// Ties limb motion to the two speakers: left limbs (L-ARM, L-LEG) drive the
// left channel, right limbs the right channel. Pure logic, host-testable.

namespace suit_sound {

// Limb indices, matching the mux channel order.
inline constexpr uint8_t kLeftArm = 0;
inline constexpr uint8_t kRightArm = 1;
inline constexpr uint8_t kLeftLeg = 2;
inline constexpr uint8_t kRightLeg = 3;

struct Side {
  motion_intensity::State intensity;
  sound_synth::State synth;
};

struct State {
  Side left;
  Side right;
};

// Feeds the latest limb samples into both sides' intensity smoothing.
void update_motion(State& state, const ImuSample samples[4], float dt_s);

// Render mono int16 audio for one channel at the current intensity.
void render_left(State& state, uint32_t sample_rate_hz, int16_t* out,
                 size_t count);
void render_right(State& state, uint32_t sample_rate_hz, int16_t* out,
                  size_t count);

}  // namespace suit_sound
