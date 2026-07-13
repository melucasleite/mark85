#include <unity.h>

#include <cstdlib>

#include "domains/suit_sound.h"

namespace {
constexpr uint32_t kRate = 22050;
constexpr size_t kChunk = 256;
constexpr float kDt = 0.02f;

int16_t peak(const int16_t* buf, size_t n) {
  int16_t p = 0;
  for (size_t i = 0; i < n; ++i) {
    const int16_t a = static_cast<int16_t>(std::abs(buf[i]));
    if (a > p) p = a;
  }
  return p;
}

ImuSample moving_sample() {
  ImuSample s;
  s.gyro_raw[0] = 26200;  // ~200 dps -> full intensity
  s.valid = true;
  return s;
}
}  // namespace

void setUp() {}
void tearDown() {}

static void test_left_limbs_drive_left_channel_only() {
  suit_sound::State state;
  ImuSample samples[4];                          // all idle/invalid
  samples[suit_sound::kLeftArm] = moving_sample();  // only L-ARM moves

  int16_t left[kChunk];
  int16_t right[kChunk];
  uint32_t now = 0;
  for (int i = 0; i < 25; ++i) {  // half a second of motion
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
    suit_sound::render_right(state, kRate, right, kChunk);
  }

  TEST_ASSERT_TRUE_MESSAGE(peak(left, kChunk) > 5000,
                           "left channel should be loud");
  TEST_ASSERT_EQUAL_INT16_MESSAGE(0, peak(right, kChunk),
                                  "right channel should stay silent");
}

static void test_right_leg_drives_right_channel() {
  suit_sound::State state;
  ImuSample samples[4];
  samples[suit_sound::kRightLeg] = moving_sample();

  int16_t left[kChunk];
  int16_t right[kChunk];
  uint32_t now = 0;
  for (int i = 0; i < 25; ++i) {
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
    suit_sound::render_right(state, kRate, right, kChunk);
  }

  TEST_ASSERT_TRUE(peak(right, kChunk) > 5000);
  TEST_ASSERT_EQUAL_INT16(0, peak(left, kChunk));
}

static pose_calibration::Reference down_x() {
  pose_calibration::Reference r;
  r.unit[0] = 1.0f;
  r.valid = true;
  return r;
}

static void test_raised_left_arm_charges_left_channel() {
  suit_sound::State state;
  state.left.arm_down = down_x();
  ImuSample samples[4];
  // Arm held overhead and still: gravity inverted vs the calibrated down.
  samples[suit_sound::kLeftArm].accel_raw[0] = -16384;
  samples[suit_sound::kLeftArm].valid = true;

  int16_t left[kChunk];
  int16_t right[kChunk];
  uint32_t now = 0;
  for (int i = 0; i < 50; ++i) {  // 1 s raised: latched + charging
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
    suit_sound::render_right(state, kRate, right, kChunk);
  }
  TEST_ASSERT_TRUE_MESSAGE(peak(left, kChunk) > 2000,
                           "charge voice should sound on the left");
  TEST_ASSERT_EQUAL_INT16(0, peak(right, kChunk));

  // Lower the arm: charge winds down and the sound fully stops.
  samples[suit_sound::kLeftArm].accel_raw[0] = 16384;
  for (int i = 0; i < 40; ++i) {  // 800 ms lowered
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
  }
  TEST_ASSERT_EQUAL_INT16_MESSAGE(0, peak(left, kChunk),
                                  "charge sound must stop when lowered");
}

static void test_raised_right_arm_charges_right_channel() {
  suit_sound::State state;
  state.right.arm_down = down_x();
  ImuSample samples[4];
  samples[suit_sound::kRightArm].accel_raw[0] = -16384;
  samples[suit_sound::kRightArm].valid = true;

  int16_t left[kChunk];
  int16_t right[kChunk];
  uint32_t now = 0;
  for (int i = 0; i < 50; ++i) {
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
    suit_sound::render_right(state, kRate, right, kChunk);
  }
  TEST_ASSERT_TRUE_MESSAGE(peak(right, kChunk) > 2000,
                           "charge voice should sound on the right");
  TEST_ASSERT_EQUAL_INT16(0, peak(left, kChunk));
}

static void test_uncalibrated_arm_never_charges() {
  suit_sound::State state;  // both references invalid
  ImuSample samples[4];
  samples[suit_sound::kLeftArm].accel_raw[0] = -16384;
  samples[suit_sound::kLeftArm].valid = true;

  int16_t left[kChunk];
  uint32_t now = 0;
  for (int i = 0; i < 100; ++i) {
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
  }
  TEST_ASSERT_EQUAL_INT16_MESSAGE(0, peak(left, kChunk),
                                  "no calibration -> blaster stays off");
}

static void test_simultaneous_limbs_sound_both_channels() {
  // A left limb and a right limb moving at once: both channels must sound,
  // independently. Single-limb tests never exercise this.
  suit_sound::State state;
  ImuSample samples[4];
  samples[suit_sound::kLeftArm] = moving_sample();
  samples[suit_sound::kRightLeg] = moving_sample();

  int16_t left[kChunk];
  int16_t right[kChunk];
  uint32_t now = 0;
  for (int i = 0; i < 25; ++i) {
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
    suit_sound::render_right(state, kRate, right, kChunk);
  }
  TEST_ASSERT_TRUE(peak(left, kChunk) > 5000);
  TEST_ASSERT_TRUE(peak(right, kChunk) > 5000);
}

static void test_charge_over_whir_mix_stays_bounded() {
  // Left arm raised AND rotating fast: the charge voice layers onto the whir
  // on the same channel. This is the only path where add_charge mixes into a
  // buffer fill() already filled at real levels -- assert it saturates
  // cleanly rather than wrapping.
  suit_sound::State state;
  state.left.arm_down = down_x();
  ImuSample samples[4];
  samples[suit_sound::kLeftArm].accel_raw[0] = -16384;  // inverted -> raised
  samples[suit_sound::kLeftArm].gyro_raw[0] = 26200;    // ~200 dps -> whirs
  samples[suit_sound::kLeftArm].valid = true;

  int16_t left[kChunk];
  uint32_t now = 0;
  bool saw_whir_and_charge = false;
  for (int i = 0; i < 100; ++i) {  // 2 s: intensity saturated, charge full
    now += 20;
    suit_sound::update_motion(state, samples, now, kDt);
    suit_sound::render_left(state, kRate, left, kChunk);
    for (size_t j = 0; j < kChunk; ++j) {
      TEST_ASSERT_TRUE(left[j] >= -32767 && left[j] <= 32767);
    }
    if (state.left.intensity.level > 0.9f && state.left.blaster.charge > 0.9f) {
      saw_whir_and_charge = true;
    }
  }
  TEST_ASSERT_TRUE_MESSAGE(saw_whir_and_charge,
                           "both voices should be fully active on one channel");
}

static void test_all_idle_is_all_silent() {
  suit_sound::State state;
  ImuSample samples[4];  // everything invalid -> zero intensity
  int16_t buf[kChunk];
  suit_sound::update_motion(state, samples, 20, kDt);
  suit_sound::render_left(state, kRate, buf, kChunk);
  TEST_ASSERT_EQUAL_INT16(0, peak(buf, kChunk));
  suit_sound::render_right(state, kRate, buf, kChunk);
  TEST_ASSERT_EQUAL_INT16(0, peak(buf, kChunk));
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_left_limbs_drive_left_channel_only);
  RUN_TEST(test_right_leg_drives_right_channel);
  RUN_TEST(test_raised_left_arm_charges_left_channel);
  RUN_TEST(test_raised_right_arm_charges_right_channel);
  RUN_TEST(test_uncalibrated_arm_never_charges);
  RUN_TEST(test_simultaneous_limbs_sound_both_channels);
  RUN_TEST(test_charge_over_whir_mix_stays_bounded);
  RUN_TEST(test_all_idle_is_all_silent);
  return UNITY_END();
}
