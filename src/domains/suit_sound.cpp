#include "suit_sound.h"

namespace suit_sound {

void update_motion(State& state, const ImuSample samples[4], uint32_t now_ms,
                   float dt_s) {
  motion_intensity::update(
      state.left.intensity,
      motion_intensity::limb_pair_intensity(samples[kLeftArm],
                                            samples[kLeftLeg]),
      dt_s);
  motion_intensity::update(
      state.right.intensity,
      motion_intensity::limb_pair_intensity(samples[kRightArm],
                                            samples[kRightLeg]),
      dt_s);
  blaster_charge::update(state.left.blaster, state.left.arm_down,
                         samples[kLeftArm], now_ms, dt_s);
  blaster_charge::update(state.right.blaster, state.right.arm_down,
                         samples[kRightArm], now_ms, dt_s);
}

void render_left(State& state, uint32_t sample_rate_hz, int16_t* out,
                 size_t count) {
  sound_synth::fill(state.left.synth, state.left.intensity.level,
                    sample_rate_hz, out, count);
  sound_synth::add_charge(state.left.charge_voice, state.left.blaster.charge,
                          sample_rate_hz, out, count);
}

void render_right(State& state, uint32_t sample_rate_hz, int16_t* out,
                  size_t count) {
  sound_synth::fill(state.right.synth, state.right.intensity.level,
                    sample_rate_hz, out, count);
  sound_synth::add_charge(state.right.charge_voice,
                          state.right.blaster.charge, sample_rate_hz, out,
                          count);
}

}  // namespace suit_sound
