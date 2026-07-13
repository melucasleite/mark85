#include <unity.h>

#include "domains/blaster_charge.h"

using blaster_charge::kLedCount;
using blaster_charge::render_leds;
using blaster_charge::State;
using blaster_charge::update;

namespace {
constexpr float kDt = 0.02f;  // 50 Hz, matching the IMU tick

ImuSample arm_at(float g_along_forearm) {
  ImuSample s;
  s.accel_raw[blaster_charge::kUpAxis] =
      static_cast<int16_t>(g_along_forearm * 16384.0f);
  s.valid = true;
  return s;
}

// Runs `ticks` updates and returns the final charge.
float run(State& state, const ImuSample& sample, uint32_t& now_ms,
          int ticks) {
  float charge = 0.0f;
  for (int i = 0; i < ticks; ++i) {
    now_ms += 20;
    charge = update(state, sample, now_ms, kDt);
  }
  return charge;
}
}  // namespace

void setUp() {}
void tearDown() {}

static void test_brief_spike_does_not_latch() {
  State s;
  uint32_t now = 0;
  // 100 ms raised (< 150 ms debounce), then back down
  run(s, arm_at(1.0f), now, 5);
  run(s, arm_at(0.0f), now, 20);
  TEST_ASSERT_FALSE(s.raised);
  TEST_ASSERT_TRUE(s.charge == 0.0f);
}

static void test_sustained_raise_latches_and_charges() {
  State s;
  uint32_t now = 0;
  const float charge = run(s, arm_at(1.0f), now, 30);  // 600 ms raised
  TEST_ASSERT_TRUE(s.raised);
  TEST_ASSERT_TRUE(charge > 0.2f);
  TEST_ASSERT_TRUE(charge < 1.0f);  // not yet full at 600 ms
}

static void test_charge_reaches_full() {
  State s;
  uint32_t now = 0;
  const float charge = run(s, arm_at(1.0f), now, 100);  // 2 s raised
  TEST_ASSERT_TRUE(charge == 1.0f);
}

static void test_lowering_winds_charge_down_to_zero() {
  State s;
  uint32_t now = 0;
  run(s, arm_at(1.0f), now, 100);                       // fully charged
  const float charge = run(s, arm_at(0.0f), now, 40);   // 800 ms lowered
  TEST_ASSERT_FALSE(s.raised);
  TEST_ASSERT_TRUE(charge == 0.0f);
}

static void test_hysteresis_holds_pose_between_thresholds() {
  State s;
  uint32_t now = 0;
  run(s, arm_at(1.0f), now, 30);
  TEST_ASSERT_TRUE(s.raised);
  // 0.5 g sits between the 0.4 lower and 0.6 raise thresholds:
  // a raised arm must stay raised there.
  run(s, arm_at(0.5f), now, 30);
  TEST_ASSERT_TRUE(s.raised);
}

static void test_invalid_sample_reads_as_lowered() {
  State s;
  uint32_t now = 0;
  run(s, arm_at(1.0f), now, 100);
  ImuSample dead;  // valid = false
  run(s, dead, now, 40);
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
  // At half charge the first ring pixels glow, the last is still dark,
  // and the center hasn't ignited.
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
  TEST_ASSERT_TRUE(px[blaster_charge::kCenterLed].r > 100);  // primed pulse
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
  RUN_TEST(test_invalid_sample_reads_as_lowered);
  RUN_TEST(test_leds_dark_at_zero_charge);
  RUN_TEST(test_ring_fills_with_charge);
  RUN_TEST(test_full_charge_lights_ring_and_center);
  return UNITY_END();
}
