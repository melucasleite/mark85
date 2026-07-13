#include <unity.h>

#include <cmath>

#include "domains/pose_calibration.h"

using pose_calibration::Calibrator;
using pose_calibration::down_dot;
using pose_calibration::feed;
using pose_calibration::finalize;
using pose_calibration::Reference;

namespace {

ImuSample accel(int16_t x, int16_t y, int16_t z) {
  ImuSample s;
  s.accel_raw[0] = x;
  s.accel_raw[1] = y;
  s.accel_raw[2] = z;
  s.valid = true;
  return s;
}

// 1 g split evenly across all three axes: 16384/sqrt(3)
constexpr int16_t kDiag = 9459;

}  // namespace

void setUp() {}
void tearDown() {}

static void test_still_diagonal_mount_calibrates() {
  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    feed(cal, accel(kDiag, kDiag, kDiag));
  }
  const Reference ref = finalize(cal);
  TEST_ASSERT_TRUE(ref.valid);
  const float expect = 1.0f / std::sqrt(3.0f);
  TEST_ASSERT_TRUE(std::fabs(ref.unit[0] - expect) < 0.01f);
  TEST_ASSERT_TRUE(std::fabs(ref.unit[1] - expect) < 0.01f);
  TEST_ASSERT_TRUE(std::fabs(ref.unit[2] - expect) < 0.01f);
}

static void test_too_few_samples_is_invalid() {
  Calibrator cal;
  for (int i = 0; i < 10; ++i) {
    feed(cal, accel(16384, 0, 0));
  }
  TEST_ASSERT_FALSE(finalize(cal).valid);
}

static void test_movement_during_window_is_invalid() {
  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    // Magnitude swinging between ~0.6 g and ~1.5 g: the arm was moving.
    feed(cal, accel((i % 2) ? 9800 : 24500, 0, 0));
  }
  TEST_ASSERT_FALSE(finalize(cal).valid);
}

static void test_weak_magnitude_is_invalid() {
  // Still and directional, but only ~0.5 g: the accel is on the wrong full
  // scale or half-dead. Spread is zero, so this isolates the magnitude gate.
  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    feed(cal, accel(8192, 0, 0));  // 0.5 g, below kMinMeanG
  }
  TEST_ASSERT_FALSE(finalize(cal).valid);
}

static void test_excessive_magnitude_is_invalid() {
  // Still and directional at ~1.5 g: above kMaxMeanG. Again spread is zero,
  // so only the magnitude ceiling can reject it.
  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    feed(cal, accel(24576, 0, 0));  // 1.5 g, above kMaxMeanG
  }
  TEST_ASSERT_FALSE(finalize(cal).valid);
}

static void test_down_dot_near_freefall_reads_as_lowered() {
  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    feed(cal, accel(16384, 0, 0));
  }
  const Reference ref = finalize(cal);
  TEST_ASSERT_TRUE(ref.valid);
  // ~0.06 g: below kMinUsableG, direction is meaningless -> safe "lowered".
  TEST_ASSERT_TRUE(down_dot(ref, accel(-1000, 0, 0)) == 1.0f);
}

static void test_wandering_direction_is_invalid() {
  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    // Constant 1 g magnitude but the direction flips: averages to nothing.
    feed(cal, accel((i % 2) ? 16384 : -16384, 0, 0));
  }
  TEST_ASSERT_FALSE(finalize(cal).valid);
}

static void test_invalid_samples_are_skipped() {
  Calibrator cal;
  ImuSample dead;  // valid = false
  for (int i = 0; i < 200; ++i) {
    feed(cal, dead);
  }
  TEST_ASSERT_FALSE(finalize(cal).valid);
}

static void test_down_dot_tracks_arm_angle() {
  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    feed(cal, accel(16384, 0, 0));
  }
  const Reference ref = finalize(cal);
  TEST_ASSERT_TRUE(ref.valid);
  // Neutral pose: +1. Inverted (arm overhead): -1. Horizontal: ~0.
  TEST_ASSERT_TRUE(down_dot(ref, accel(16384, 0, 0)) > 0.99f);
  TEST_ASSERT_TRUE(down_dot(ref, accel(-16384, 0, 0)) < -0.99f);
  TEST_ASSERT_TRUE(std::fabs(down_dot(ref, accel(0, 16384, 0))) < 0.01f);
}

static void test_invalid_inputs_read_as_lowered() {
  const Reference no_cal;  // invalid
  TEST_ASSERT_TRUE(down_dot(no_cal, accel(-16384, 0, 0)) == 1.0f);

  Calibrator cal;
  for (int i = 0; i < 60; ++i) {
    feed(cal, accel(16384, 0, 0));
  }
  const Reference ref = finalize(cal);
  ImuSample dead;  // invalid sample
  TEST_ASSERT_TRUE(down_dot(ref, dead) == 1.0f);
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_still_diagonal_mount_calibrates);
  RUN_TEST(test_too_few_samples_is_invalid);
  RUN_TEST(test_weak_magnitude_is_invalid);
  RUN_TEST(test_excessive_magnitude_is_invalid);
  RUN_TEST(test_down_dot_near_freefall_reads_as_lowered);
  RUN_TEST(test_movement_during_window_is_invalid);
  RUN_TEST(test_wandering_direction_is_invalid);
  RUN_TEST(test_invalid_samples_are_skipped);
  RUN_TEST(test_down_dot_tracks_arm_angle);
  RUN_TEST(test_invalid_inputs_read_as_lowered);
  return UNITY_END();
}
