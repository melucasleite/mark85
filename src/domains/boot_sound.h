#pragma once

#include <cstddef>
#include <cstdint>

// Boot jingle: a short retro power-up — rising chiptune arpeggio, a servo
// wobble slide, and a final ding — played on both speakers during the
// splash. Pure and deterministic: output is a function of sample position.

namespace boot_sound {

inline constexpr uint32_t kDurationMs = 1900;
inline constexpr float kMasterGain = 0.55f;

struct State {
  uint32_t sample_pos = 0;
};

// Renders the next `count` samples of the jingle; silence once finished.
void fill(State& state, uint32_t sample_rate_hz, int16_t* out, size_t count);

bool finished(const State& state, uint32_t sample_rate_hz);

}  // namespace boot_sound
