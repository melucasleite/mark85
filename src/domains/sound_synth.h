#pragma once

#include <cstddef>
#include <cstdint>

// Electro-mechanical suit sound synthesizer: a servo-whir voice whose pitch
// and loudness follow a 0..1 intensity. Pure and deterministic — no Arduino,
// no I/O, no global state; everything lives in the caller-owned State.

namespace sound_synth {

// Whir pitch: base when barely moving, rising with intensity.
inline constexpr float kBaseHz = 70.0f;
inline constexpr float kSpanHz = 260.0f;
// Voice mix: fundamental + inharmonic partial (gear mesh) + noise (hiss).
inline constexpr float kToneMix = 0.55f;
inline constexpr float kPartialMix = 0.20f;
inline constexpr float kNoiseMix = 0.25f;
inline constexpr float kPartialRatio = 2.71f;  // inharmonic on purpose
inline constexpr float kMasterGain = 0.75f;
// Envelope: fast attack, slower release, and a floor below which output
// snaps to true silence.
inline constexpr float kAttackTauS = 0.03f;
inline constexpr float kReleaseTauS = 0.15f;
inline constexpr float kSilenceEnv = 1e-3f;

struct State {
  float phase1 = 0.0f;
  float phase2 = 0.0f;
  float env = 0.0f;
  uint32_t rng = 0x12345678u;
};

// Renders `count` mono int16 samples for the given intensity (clamped 0..1).
void fill(State& state, float intensity, uint32_t sample_rate_hz,
          int16_t* out, size_t count);

}  // namespace sound_synth
