#include <unity.h>

#include <cstdint>

#include "domains/boot_sequence.h"

using boot_sequence::kSplashDurationMs;
using boot_sequence::Phase;
using boot_sequence::phase_at;

void setUp() {}
void tearDown() {}

static void test_splash_at_boot() {
  TEST_ASSERT_TRUE(phase_at(0, 0) == Phase::kSplash);
}

static void test_splash_until_duration_elapses() {
  TEST_ASSERT_TRUE(phase_at(0, kSplashDurationMs - 1) == Phase::kSplash);
}

static void test_monitor_at_exact_duration() {
  TEST_ASSERT_TRUE(phase_at(0, kSplashDurationMs) == Phase::kMonitor);
}

static void test_monitor_long_after_boot() {
  TEST_ASSERT_TRUE(phase_at(1000, 1000000) == Phase::kMonitor);
}

static void test_nonzero_boot_time() {
  const uint32_t boot = 123456;
  TEST_ASSERT_TRUE(phase_at(boot, boot + kSplashDurationMs - 1) ==
                   Phase::kSplash);
  TEST_ASSERT_TRUE(phase_at(boot, boot + kSplashDurationMs) ==
                   Phase::kMonitor);
}

static void test_millis_wraparound() {
  // Boot 1 ms before the uint32 wrap: 4 s after boot the counter has wrapped
  // to ~4e9 less than boot_ms, but unsigned subtraction keeps the math right.
  const uint32_t boot = 0xFFFFFFFF;
  const uint32_t now_during = boot + 4000;  // wrapped
  const uint32_t now_after = boot + kSplashDurationMs;  // wrapped
  TEST_ASSERT_TRUE(phase_at(boot, now_during) == Phase::kSplash);
  TEST_ASSERT_TRUE(phase_at(boot, now_after) == Phase::kMonitor);
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_splash_at_boot);
  RUN_TEST(test_splash_until_duration_elapses);
  RUN_TEST(test_monitor_at_exact_duration);
  RUN_TEST(test_monitor_long_after_boot);
  RUN_TEST(test_nonzero_boot_time);
  RUN_TEST(test_millis_wraparound);
  return UNITY_END();
}
