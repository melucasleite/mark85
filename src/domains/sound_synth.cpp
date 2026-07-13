#include "sound_synth.h"

#include <cmath>

namespace sound_synth {

namespace {

constexpr float kTwoPi = 6.28318530718f;

// xorshift32: cheap deterministic noise, mapped to [-1, 1]
float next_noise(uint32_t& rng) {
  rng ^= rng << 13;
  rng ^= rng >> 17;
  rng ^= rng << 5;
  return static_cast<float>(static_cast<int32_t>(rng)) / 2147483648.0f;
}

}  // namespace

void fill(State& state, float intensity, uint32_t sample_rate_hz,
          int16_t* out, size_t count) {
  if (intensity < 0.0f) intensity = 0.0f;
  if (intensity > 1.0f) intensity = 1.0f;

  const float dt = 1.0f / static_cast<float>(sample_rate_hz);
  const float attack = 1.0f - std::exp(-dt / kAttackTauS);
  const float release = 1.0f - std::exp(-dt / kReleaseTauS);

  for (size_t i = 0; i < count; ++i) {
    const float coeff = intensity > state.env ? attack : release;
    state.env += (intensity - state.env) * coeff;

    if (state.env < kSilenceEnv && intensity <= 0.0f) {
      state.env = 0.0f;
      out[i] = 0;
      continue;
    }

    const float freq = kBaseHz + kSpanHz * state.env;
    state.phase1 += kTwoPi * freq * dt;
    state.phase2 += kTwoPi * freq * kPartialRatio * dt;
    if (state.phase1 > kTwoPi) state.phase1 -= kTwoPi;
    if (state.phase2 > kTwoPi) state.phase2 -= kTwoPi;

    const float voice = kToneMix * std::sin(state.phase1) +
                        kPartialMix * std::sin(state.phase2) +
                        kNoiseMix * next_noise(state.rng) * state.env;
    float s = state.env * kMasterGain * voice * 32767.0f;
    if (s > 32767.0f) s = 32767.0f;
    if (s < -32767.0f) s = -32767.0f;
    out[i] = static_cast<int16_t>(s);
  }
}

void add_charge(ChargeVoice& voice, float charge, uint32_t sample_rate_hz,
                int16_t* buf, size_t count) {
  if (charge < 0.0f) charge = 0.0f;
  if (charge > 1.0f) charge = 1.0f;
  if (charge <= 0.0f) {
    return;
  }

  const float dt = 1.0f / static_cast<float>(sample_rate_hz);
  // charge^2 makes the sweep accelerate — slow wind-up, fast top end.
  const float base_freq = kChargeBaseHz + kChargeSpanHz * charge * charge;
  const float amp = kChargeGain * charge;

  for (size_t i = 0; i < count; ++i) {
    voice.vib_phase += kTwoPi * kChargeVibratoHz * dt;
    if (voice.vib_phase > kTwoPi) voice.vib_phase -= kTwoPi;
    const float freq =
        base_freq * (1.0f + kChargeVibratoDepth * charge *
                                std::sin(voice.vib_phase));
    voice.phase += kTwoPi * freq * dt;
    if (voice.phase > kTwoPi) voice.phase -= kTwoPi;

    const float s = amp * (0.7f * std::sin(voice.phase) +
                           0.3f * std::sin(2.0f * voice.phase));
    float mixed = static_cast<float>(buf[i]) + s * 32767.0f;
    if (mixed > 32767.0f) mixed = 32767.0f;
    if (mixed < -32767.0f) mixed = -32767.0f;
    buf[i] = static_cast<int16_t>(mixed);
  }
}

}  // namespace sound_synth
