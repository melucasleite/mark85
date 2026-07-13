#include "blaster_charge.h"

#include <cmath>

namespace blaster_charge {

namespace {

constexpr float kTwoPi = 6.28318530718f;
constexpr float kPrimedFrom = 0.95f;  // center LED ignites near full charge

uint8_t to_u8(float v) {
  if (v <= 0.0f) return 0;
  if (v >= 255.0f) return 255;
  return static_cast<uint8_t>(v);
}

}  // namespace

float update(State& state, const pose_calibration::Reference& down_ref,
             const ImuSample& left_arm, uint32_t now_ms, float dt_s) {
  // +1 = neutral pose, -1 = inverted; invalid inputs read as +1 (lowered).
  const float dot = pose_calibration::down_dot(down_ref, left_arm);

  // Hysteresis: a raised arm must swing back under ~90 deg to count as
  // lowered, and a lowered arm must pass ~120 deg to count as raised.
  const bool pose_now =
      state.raised ? (dot < kLowerDot) : (dot < kRaiseDot);

  // Debounce: the new pose must hold for kPoseDebounceMs before it latches.
  if (pose_now == state.raised) {
    state.candidate = state.raised;
  } else if (pose_now != state.candidate) {
    state.candidate = pose_now;
    state.candidate_since_ms = now_ms;
  } else if (now_ms - state.candidate_since_ms >= kPoseDebounceMs) {
    state.raised = pose_now;
  }

  const float rate = state.raised ? dt_s / kChargeUpS : -dt_s / kChargeDownS;
  state.charge += rate;
  if (state.charge < 0.0f) state.charge = 0.0f;
  if (state.charge > 1.0f) state.charge = 1.0f;
  return state.charge;
}

void render_leds(float charge, uint32_t now_ms, RgbColor out[kLedCount]) {
  if (charge < 0.0f) charge = 0.0f;
  if (charge > 1.0f) charge = 1.0f;

  // Ring pixels 1..6 fill sequentially; each fades in over its slice.
  const float progress = charge * 6.0f;
  for (uint8_t i = 0; i < 6; ++i) {
    float lit = progress - static_cast<float>(i);
    if (lit < 0.0f) lit = 0.0f;
    if (lit > 1.0f) lit = 1.0f;
    out[1 + i].r = to_u8(200.0f * lit * charge * charge);
    out[1 + i].g = to_u8(230.0f * lit * charge);
    out[1 + i].b = to_u8(255.0f * lit * (0.35f + 0.65f * charge));
  }

  // Center: white-hot pulse once primed, dark otherwise.
  float primed = (charge - kPrimedFrom) / (1.0f - kPrimedFrom);
  if (primed < 0.0f) primed = 0.0f;
  const float pulse =
      0.7f + 0.3f * std::sin(kTwoPi * 3.0f * (now_ms % 1000u) / 1000.0f);
  const uint8_t v = to_u8(255.0f * primed * pulse);
  out[kCenterLed].r = v;
  out[kCenterLed].g = v;
  out[kCenterLed].b = v;
}

}  // namespace blaster_charge
