#include <unity.h>

#include "domains/blaster_charge.h"

using blaster_charge::kLedCount;
using blaster_charge::render_leds;
using blaster_charge::State;
using blaster_charge::update;
using pose_calibration::Reference;

namespace {
constexpr float kDt = 0.02f;  // 50 Hz, matching the IMU tick

// Calibrated "down" along +X, as if gravity sits on the sensor's X axis
// in the neutral pose.
Reference down_x() {
  Reference r;
  r.unit[0] = 1.0f;
  r.valid = true;
  return r;
}

ImuSample accel(int16_t x, int16_t y, int16_t z) {
  ImuSample s;
  s.accel_raw[0] = x;
  s.accel_raw[1] = y;
  s.accel_raw[2] = z;
  s.valid = true;
  return s;
}

ImuSample arm_down() { return accel(16384, 0, 0); }
ImuSample arm_raised() { return accel(-16384, 0, 0); }

// Runs `ticks` updates and returns the final charge.
float run(State& state, const Reference& ref, const ImuSample& sample,
          uint32_t& now_ms, int ticks) {
  float charge = 0.0f;
  for (int i = 0; i < ticks; ++i) {
    now_ms += 20;
    charge = update(state, ref, sample, now_ms, kDt);
  }
  return charge;
}
}  // namespace

void setUp() {}
void tearDown() {}

static void test_brief_spike_does_not_latch() {
  State s;
  uint32_t now = 0;
  const Reference ref = down_x();
  run(s, ref, arm_raised(), now, 5);  // 100 ms < 150 ms debounce
  run(s, ref, arm_down(), now, 20);
  TEST_ASSERT_FALSE(s.raised);
  TEST_ASSERT_TRUE(s.charge == 0.0f);
}

static void test_sustained_raise_latches_and_charges() {
  State s;
  uint32_t now = 0;
  const Reference ref = down_x();
  const float charge = run(s, ref, arm_raised(), now, 30);  // 600 ms
  TEST_ASSERT_TRUE(s.raised);
  TEST_ASSERT_TRUE(charge > 0.2f);
  TEST_ASSERT_TRUE(charge < 1.0f);
}

static void test_charge_reaches_full() {
  State s;
  uint32_t now = 0;
  const Reference ref = down_x();
  TEST_ASSERT_TRUE(run(s, ref, arm_raised(), now, 100) == 1.0f);
}

static void test_lowering_winds_charge_down_to_zero() {
  State s;
  uint32_t now = 0;
  const Reference ref = down_x();
  run(s, ref, arm_raised(), now, 100);
  const float charge = run(s, ref, arm_down(), now, 40);
  TEST_ASSERT_FALSE(s.raised);
  TEST_ASSERT_TRUE(charge == 0.0f);
}

static void test_hysteresis_holds_pose_between_thresholds() {
  State s;
  uint32_t now = 0;
  const Reference ref = down_x();
  run(s, ref, arm_raised(), now, 30);
  TEST_ASSERT_TRUE(s.raised);
  // dot = -0.39: between raise (-0.5) and lower (0.0) — raised must hold.
  const ImuSample between = accel(-4915, 11469, 0);
  run(s, ref, between, now, 30);
  TEST_ASSERT_TRUE(s.raised);
}

static void test_any_mounting_orientation_works() {
  // Calibrated down on a diagonal — a crooked strap — still charges when
  // gravity flips to the opposite diagonal.
  Reference ref;
  const float inv_sqrt3 = 0.57735f;
  ref.unit[0] = inv_sqrt3;
  ref.unit[1] = inv_sqrt3;
  ref.unit[2] = inv_sqrt3;
  ref.valid = true;
  State s;
  uint32_t now = 0;
  const float charge = run(s, ref, accel(-9459, -9459, -9459), now, 30);
  TEST_ASSERT_TRUE(s.raised);
  TEST_ASSERT_TRUE(charge > 0.2f);
}

static void test_invalid_calibration_disables_blaster() {
  State s;
  uint32_t now = 0;
  const Reference no_cal;  // invalid
  const float charge = run(s, no_cal, arm_raised(), now, 100);
  TEST_ASSERT_FALSE(s.raised);
  TEST_ASSERT_TRUE(charge == 0.0f);
}

static void test_invalid_sample_reads_as_lowered() {
  State s;
  uint32_t now = 0;
  const Reference ref = down_x();
  run(s, ref, arm_raised(), now, 100);
  ImuSample dead;  // valid = false
  run(s, ref, dead, now, 40);
  TEST_ASSERT_FALSE(s.raised);
  TEST_ASSERT_TRUE(s.charge == 0.0f);
}

static void test_leds_dark_at_zero_charge() {
  RgbColor px[kLedCount];
  render_leds(0.0f, 12345, px);
  for (uint8_t i = 0; i < kLedCount; ++i) {
    TEST_ASSERT_EQUAL_UINT8(0, px[i].r);
    TEST_ASSERT_EQUAL_UINT8(0, px[i].g);
    TEST_ASSERT_EQUAL_UINT8(0, px[i].b);
  }
}

static void test_ring_fills_with_charge() {
  RgbColor px[kLedCount];
  render_leds(0.5f, 0, px);
  TEST_ASSERT_TRUE(px[1].b > 0);
  TEST_ASSERT_EQUAL_UINT8(0, px[6].b);
  TEST_ASSERT_EQUAL_UINT8(0, px[blaster_charge::kCenterLed].r);
}

static void test_full_charge_lights_ring_and_center() {
  RgbColor px[kLedCount];
  render_leds(1.0f, 250, px);
  for (uint8_t i = 1; i < kLedCount; ++i) {
    TEST_ASSERT_TRUE(px[i].b > 0);
  }
  TEST_ASSERT_TRUE(px[blaster_charge::kCenterLed].r > 100);
}

static void test_render_leds_clamps_out_of_range() {
  RgbColor over[kLedCount];
  RgbColor full[kLedCount];
  render_leds(5.0f, 250, over);   // clamps to 1.0
  render_leds(1.0f, 250, full);
  for (uint8_t i = 0; i < kLedCount; ++i) {
    TEST_ASSERT_EQUAL_UINT8(full[i].r, over[i].r);
    TEST_ASSERT_EQUAL_UINT8(full[i].g, over[i].g);
    TEST_ASSERT_EQUAL_UINT8(full[i].b, over[i].b);
  }

  RgbColor under[kLedCount];
  render_leds(-2.0f, 250, under);  // clamps to 0.0 -> dark
  for (uint8_t i = 0; i < kLedCount; ++i) {
    TEST_ASSERT_EQUAL_UINT8(0, under[i].r);
    TEST_ASSERT_EQUAL_UINT8(0, under[i].g);
    TEST_ASSERT_EQUAL_UINT8(0, under[i].b);
  }
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_brief_spike_does_not_latch);
  RUN_TEST(test_sustained_raise_latches_and_charges);
  RUN_TEST(test_charge_reaches_full);
  RUN_TEST(test_lowering_winds_charge_down_to_zero);
  RUN_TEST(test_hysteresis_holds_pose_between_thresholds);
  RUN_TEST(test_any_mounting_orientation_works);
  RUN_TEST(test_invalid_calibration_disables_blaster);
  RUN_TEST(test_invalid_sample_reads_as_lowered);
  RUN_TEST(test_leds_dark_at_zero_charge);
  RUN_TEST(test_ring_fills_with_charge);
  RUN_TEST(test_full_charge_lights_ring_and_center);
  RUN_TEST(test_render_leds_clamps_out_of_range);
  return UNITY_END();
}
