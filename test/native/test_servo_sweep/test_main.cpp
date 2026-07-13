#include <unity.h>

#include <cmath>

#include "domains/servo_sweep.h"

using servo_sweep::angle_at;
using servo_sweep::kSweepPeriodMs;

void setUp() {}
void tearDown() {}

static void test_starts_at_zero() {
  TEST_ASSERT_TRUE(angle_at(0) == 0.0f);
}

static void test_reaches_180_at_half_period() {
  TEST_ASSERT_TRUE(std::fabs(angle_at(kSweepPeriodMs / 2) - 180.0f) < 0.1f);
}

static void test_returns_to_zero_at_full_period() {
  TEST_ASSERT_TRUE(angle_at(kSweepPeriodMs) == 0.0f);
}

static void test_quarter_points_hit_90() {
  TEST_ASSERT_TRUE(std::fabs(angle_at(kSweepPeriodMs / 4) - 90.0f) < 0.1f);
  TEST_ASSERT_TRUE(std::fabs(angle_at(3 * kSweepPeriodMs / 4) - 90.0f) < 0.1f);
}

static void test_rises_then_falls() {
  const float a1 = angle_at(1000);
  const float a2 = angle_at(2000);
  TEST_ASSERT_TRUE(a2 > a1);  // rising leg
  const float a3 = angle_at(kSweepPeriodMs / 2 + 1000);
  const float a4 = angle_at(kSweepPeriodMs / 2 + 2000);
  TEST_ASSERT_TRUE(a4 < a3);  // falling leg
}

static void test_bounded_and_periodic() {
  for (uint32_t t = 0; t < 3 * kSweepPeriodMs; t += 97) {
    const float a = angle_at(t);
    TEST_ASSERT_TRUE(a >= 0.0f && a <= 180.0f);
    TEST_ASSERT_TRUE(std::fabs(a - angle_at(t + kSweepPeriodMs)) < 0.01f);
  }
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_starts_at_zero);
  RUN_TEST(test_reaches_180_at_half_period);
  RUN_TEST(test_returns_to_zero_at_full_period);
  RUN_TEST(test_quarter_points_hit_90);
  RUN_TEST(test_rises_then_falls);
  RUN_TEST(test_bounded_and_periodic);
  return UNITY_END();
}
