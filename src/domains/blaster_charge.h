#pragma once

#include <cstdint>

#include "imu_types.h"
#include "rgb_types.h"

// Left-hand blaster: raising the arm charges the repulsor — a smooth 0..1
// charge level that drives the jewel LED animation and the charge-up sound.
// Lowering the arm ramps the charge (and everything it drives) back down.
// Pure logic, host-testable.

namespace blaster_charge {

// Arm-raised detection from the left-arm accelerometer: gravity along the
// forearm axis. Axis/sign depend on how the IMU is mounted — tune on the
// suit. Hysteresis plus debounce so wobble at the threshold can't flicker.
inline constexpr uint8_t kUpAxis = 0;  // accel X along the forearm
inline constexpr float kUpSign = 1.0f;
inline constexpr float kRaiseThresholdG = 0.6f;
inline constexpr float kLowerThresholdG = 0.4f;
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
// Returns the new charge level. Invalid samples read as "arm lowered".
float update(State& state, const ImuSample& left_arm, uint32_t now_ms,
             float dt_s);

// Renders the jewel for a charge level: ring fills clockwise as charge
// builds (deep blue warming to cyan-white), center pulses white when primed.
void render_leds(float charge, uint32_t now_ms, RgbColor out[kLedCount]);

}  // namespace blaster_charge
