#include "boot_sound.h"

#include <cmath>

namespace boot_sound {

namespace {

constexpr float kTwoPi = 6.28318530718f;

struct Note {
  float start_s;
  float dur_s;
  float freq_hz;
};

// Rising arpeggio: C5 E5 G5 C6 — the classic "console waking up".
constexpr Note kNotes[] = {
    {0.10f, 0.12f, 523.25f},
    {0.26f, 0.12f, 659.25f},
    {0.42f, 0.12f, 783.99f},
    {0.58f, 0.14f, 1046.50f},
};

// Servo wobble: a slide from 220 to 880 Hz with heavy vibrato — the suit
// clearing its throat.
constexpr float kSlideStartS = 0.82f;
constexpr float kSlideDurS = 0.45f;
constexpr float kSlideF0 = 220.0f;
constexpr float kSlideF1 = 880.0f;

// Final ding, ringing out to the end.
constexpr float kDingStartS = 1.38f;
constexpr float kDingFreq = 1318.51f;  // E6

// Chiptune-ish voice: fundamental plus a bit of 3rd harmonic.
float chirp(float phase) {
  return 0.8f * std::sin(phase) + 0.2f * std::sin(3.0f * phase);
}

float note_sample(const Note& n, float t) {
  const float local = t - n.start_s;
  if (local < 0.0f || local > n.dur_s) {
    return 0.0f;
  }
  const float attack = local / 0.005f;
  const float env = (attack < 1.0f ? attack : 1.0f) * (1.0f - local / n.dur_s);
  return env * chirp(kTwoPi * n.freq_hz * local);
}

float slide_sample(float t) {
  const float local = t - kSlideStartS;
  if (local < 0.0f || local > kSlideDurS) {
    return 0.0f;
  }
  // Phase of a linear sweep in closed form (integral of f(t)), so the
  // output is a pure function of t with no cross-sample state.
  const float rate = (kSlideF1 - kSlideF0) / kSlideDurS;
  const float phase =
      kTwoPi * (kSlideF0 * local + 0.5f * rate * local * local);
  const float wobble = std::sin(kTwoPi * 16.0f * local) * 0.5f + 0.5f;
  const float env = std::sin(3.14159265f * local / kSlideDurS);  // swell
  return 0.8f * env * (0.6f + 0.4f * wobble) * std::sin(phase);
}

float ding_sample(float t) {
  const float local = t - kDingStartS;
  if (local < 0.0f) {
    return 0.0f;
  }
  const float env = std::exp(-6.0f * local);
  return env * (0.7f * std::sin(kTwoPi * kDingFreq * local) +
                0.3f * std::sin(kTwoPi * 2.0f * kDingFreq * local));
}

}  // namespace

void fill(State& state, uint32_t sample_rate_hz, int16_t* out, size_t count) {
  const uint32_t end_pos =
      kDurationMs * sample_rate_hz / 1000u;
  for (size_t i = 0; i < count; ++i) {
    if (state.sample_pos >= end_pos) {
      out[i] = 0;
      continue;
    }
    const float t =
        static_cast<float>(state.sample_pos) / sample_rate_hz;
    float s = 0.0f;
    for (const Note& n : kNotes) {
      s += note_sample(n, t);
    }
    s += slide_sample(t);
    s += ding_sample(t);
    s *= kMasterGain * 32767.0f;
    if (s > 32767.0f) s = 32767.0f;
    if (s < -32767.0f) s = -32767.0f;
    out[i] = static_cast<int16_t>(s);
    ++state.sample_pos;
  }
}

bool finished(const State& state, uint32_t sample_rate_hz) {
  return state.sample_pos >= kDurationMs * sample_rate_hz / 1000u;
}

}  // namespace boot_sound
