#pragma once

#include <cstdint>

#include "imu_types.h"
#include "pose_calibration.h"
#include "rgb_types.h"

// Left-hand blaster: raising the arm charges the repulsor — a smooth 0..1
// charge level that drives the jewel LED animation and the charge-up sound.
// Lowering the arm ramps the charge (and everything it drives) back down.
// Pure logic, host-testable.

namespace blaster_charge {

// Arm-raised detection: the angle between current gravity and the
// boot-calibrated "down" reference (see pose_calibration), so it works for
// any sensor mounting. Raised past ~120 deg, lowered back under ~90 deg —
// wide hysteresis plus debounce so wobble at a threshold can't flicker.
// With an invalid calibration the dot reads +1 and the blaster stays safe.
inline constexpr float kRaiseDot = -0.5f;  // cos(120 deg)
inline constexpr float kLowerDot = 0.0f;   // cos(90 deg)
inline constexpr uint32_t kPoseDebounceMs = 150;

// Charge ramp: ~1.2 s to full power, ~0.3 s to wind down.
inline constexpr float kChargeUpS = 1.2f;
inline constexpr float kChargeDownS = 0.3f;

// LED jewel layout
inline constexpr uint8_t kLedCount = 7;
inline constexpr uint8_t kCenterLed = 0;  // pixels 1..6 form the ring

struct State {
  bool raised = false;
  bool candidate = false;
  uint32_t candidate_since_ms = 0;
  float charge = 0.0f;  // 0..1
};

// Feeds one left-arm sample; advances pose detection and the charge ramp.
// Returns the new charge level. Invalid samples or an invalid calibration
// reference read as "arm lowered".
float update(State& state, const pose_calibration::Reference& down_ref,
             const ImuSample& left_arm, uint32_t now_ms, float dt_s);

// Renders the jewel for a charge level: ring fills clockwise as charge
// builds (deep blue warming to cyan-white), center pulses white when primed.
void render_leds(float charge, uint32_t now_ms, RgbColor out[kLedCount]);

}  // namespace blaster_charge
