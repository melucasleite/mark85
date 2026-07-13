#include <unity.h>

#include <cmath>

#include "domains/motion_intensity.h"

using motion_intensity::kSaturationDps;
using motion_intensity::limb_pair_intensity;
using motion_intensity::sample_magnitude_dps;
using motion_intensity::State;
using motion_intensity::update;

void setUp() {}
void tearDown() {}

static ImuSample gyro_sample(int16_t gx, int16_t gy, int16_t gz) {
  ImuSample s;
  s.gyro_raw[0] = gx;
  s.gyro_raw[1] = gy;
  s.gyro_raw[2] = gz;
  s.valid = true;
  return s;
}

static void test_invalid_sample_has_zero_magnitude() {
  ImuSample s;  // valid = false
  s.gyro_raw[0] = 32767;
  TEST_ASSERT_TRUE(sample_magnitude_dps(s) == 0.0f);
}

static void test_magnitude_converts_lsb_to_dps() {
  // 131 LSB = 1 dps; single-axis magnitude is exact
  const ImuSample s = gyro_sample(1310, 0, 0);
  TEST_ASSERT_TRUE(std::fabs(sample_magnitude_dps(s) - 10.0f) < 0.01f);
}

static void test_magnitude_combines_three_axes() {
  // 60 dps on each axis -> 60*sqrt(3) ~= 103.92 dps. Exercises the full
  // Euclidean sum, not just a single-axis shortcut.
  const ImuSample s = gyro_sample(7860, 7860, 7860);  // 60 dps per axis
  TEST_ASSERT_TRUE(std::fabs(sample_magnitude_dps(s) - 60.0f * std::sqrt(3.0f)) <
                   0.05f);
}

static void test_pair_takes_busier_limb() {
  const ImuSample slow = gyro_sample(1310, 0, 0);    // 10 dps
  const ImuSample fast = gyro_sample(13100, 0, 0);   // 100 dps
  const float intensity = limb_pair_intensity(slow, fast);
  TEST_ASSERT_TRUE(std::fabs(intensity - 100.0f / kSaturationDps) < 0.01f);
}

static void test_intensity_saturates_at_one() {
  const ImuSample wild = gyro_sample(32767, 32767, 32767);
  ImuSample idle;
  TEST_ASSERT_TRUE(limb_pair_intensity(wild, idle) == 1.0f);
}

static void test_attack_faster_than_release() {
  State rising;
  State falling;
  falling.level = 1.0f;
  const float dt = 0.02f;
  const float rise = update(rising, 1.0f, dt);         // 0 -> 1
  const float fall = 1.0f - update(falling, 0.0f, dt); // 1 -> 0
  TEST_ASSERT_TRUE(rise > fall);  // attack covers more ground per tick
}

static void test_level_converges_to_target() {
  State s;
  for (int i = 0; i < 500; ++i) {
    update(s, 0.8f, 0.02f);
  }
  TEST_ASSERT_TRUE(std::fabs(s.level - 0.8f) < 0.01f);
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_invalid_sample_has_zero_magnitude);
  RUN_TEST(test_magnitude_converts_lsb_to_dps);
  RUN_TEST(test_magnitude_combines_three_axes);
  RUN_TEST(test_pair_takes_busier_limb);
  RUN_TEST(test_intensity_saturates_at_one);
  RUN_TEST(test_attack_faster_than_release);
  RUN_TEST(test_level_converges_to_target);
  return UNITY_END();
}
