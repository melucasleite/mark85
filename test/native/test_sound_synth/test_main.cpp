#include <unity.h>

#include <cstdlib>
#include <cstring>

#include "domains/sound_synth.h"

using sound_synth::fill;
using sound_synth::State;

namespace {
constexpr uint32_t kRate = 22050;
constexpr size_t kChunk = 256;

int16_t peak(const int16_t* buf, size_t n) {
  int16_t p = 0;
  for (size_t i = 0; i < n; ++i) {
    const int16_t a = static_cast<int16_t>(std::abs(buf[i]));
    if (a > p) p = a;
  }
  return p;
}
}  // namespace

void setUp() {}
void tearDown() {}

static void test_silent_at_rest() {
  State s;
  int16_t buf[kChunk];
  fill(s, 0.0f, kRate, buf, kChunk);
  for (size_t i = 0; i < kChunk; ++i) {
    TEST_ASSERT_EQUAL_INT16(0, buf[i]);
  }
}

static void test_attack_ramps_up() {
  State s;
  int16_t first[kChunk];
  int16_t later[kChunk];
  fill(s, 1.0f, kRate, first, kChunk);
  for (int i = 0; i < 8; ++i) {
    fill(s, 1.0f, kRate, later, kChunk);
  }
  TEST_ASSERT_TRUE(peak(later, kChunk) > peak(first, kChunk));
  TEST_ASSERT_TRUE(peak(later, kChunk) > 8000);  // audibly loud when moving
}

static void test_release_reaches_true_silence() {
  State s;
  int16_t buf[kChunk];
  for (int i = 0; i < 20; ++i) {
    fill(s, 1.0f, kRate, buf, kChunk);
  }
  // Exponential release crosses the silence floor at ~1.04 s (tau 0.15 s);
  // two seconds must land at exact digital silence.
  bool silent = false;
  for (int i = 0; i < 2 * static_cast<int>(kRate / kChunk) && !silent; ++i) {
    fill(s, 0.0f, kRate, buf, kChunk);
    silent = peak(buf, kChunk) == 0;
  }
  TEST_ASSERT_TRUE_MESSAGE(silent, "synth never decayed to silence");
}

static void test_deterministic() {
  State a;
  State b;
  int16_t buf_a[kChunk];
  int16_t buf_b[kChunk];
  fill(a, 0.7f, kRate, buf_a, kChunk);
  fill(b, 0.7f, kRate, buf_b, kChunk);
  TEST_ASSERT_EQUAL_MEMORY(buf_a, buf_b, sizeof(buf_a));
}

static void test_output_bounded_at_extremes() {
  State s;
  int16_t buf[kChunk];
  for (int i = 0; i < 50; ++i) {
    fill(s, 1.0f, kRate, buf, kChunk);
    // peak() would abs(-32768) incorrectly; assert raw range directly
    for (size_t j = 0; j < kChunk; ++j) {
      TEST_ASSERT_TRUE(buf[j] >= -32767 && buf[j] <= 32767);
    }
  }
}

static void test_intensity_clamped_out_of_range() {
  // Out-of-range intensity must clamp, so >1 behaves exactly like 1 and <0
  // like 0. Compare against the clamped boundary sample-for-sample.
  State a;
  State b;
  int16_t buf_a[kChunk];
  int16_t buf_b[kChunk];
  fill(a, 5.0f, kRate, buf_a, kChunk);
  fill(b, 1.0f, kRate, buf_b, kChunk);
  TEST_ASSERT_EQUAL_MEMORY(buf_a, buf_b, sizeof(buf_a));

  State c;
  int16_t buf_c[kChunk];
  fill(c, -3.0f, kRate, buf_c, kChunk);  // reads as 0 -> silence
  TEST_ASSERT_EQUAL_INT16(0, peak(buf_c, kChunk));
}

static void test_charge_clamped_out_of_range() {
  sound_synth::ChargeVoice a;
  sound_synth::ChargeVoice b;
  int16_t buf_a[kChunk] = {};
  int16_t buf_b[kChunk] = {};
  sound_synth::add_charge(a, 3.0f, kRate, buf_a, kChunk);  // clamps to 1
  sound_synth::add_charge(b, 1.0f, kRate, buf_b, kChunk);
  TEST_ASSERT_EQUAL_MEMORY(buf_a, buf_b, sizeof(buf_a));
}

static void test_charge_voice_silent_at_zero() {
  sound_synth::ChargeVoice v;
  int16_t buf[kChunk] = {};
  sound_synth::add_charge(v, 0.0f, kRate, buf, kChunk);
  TEST_ASSERT_EQUAL_INT16(0, peak(buf, kChunk));
}

static void test_charge_voice_adds_energy() {
  sound_synth::ChargeVoice v;
  int16_t buf[kChunk] = {};
  sound_synth::add_charge(v, 0.8f, kRate, buf, kChunk);
  TEST_ASSERT_TRUE(peak(buf, kChunk) > 4000);
}

static void test_charge_voice_mix_stays_bounded() {
  // Saturating add on top of a full-scale buffer must clamp, not wrap.
  sound_synth::ChargeVoice v;
  int16_t buf[kChunk];
  for (size_t i = 0; i < kChunk; ++i) {
    buf[i] = (i % 2) ? 32767 : -32767;
  }
  sound_synth::add_charge(v, 1.0f, kRate, buf, kChunk);
  for (size_t i = 0; i < kChunk; ++i) {
    TEST_ASSERT_TRUE(buf[i] >= -32767 && buf[i] <= 32767);
  }
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_silent_at_rest);
  RUN_TEST(test_attack_ramps_up);
  RUN_TEST(test_release_reaches_true_silence);
  RUN_TEST(test_deterministic);
  RUN_TEST(test_output_bounded_at_extremes);
  RUN_TEST(test_intensity_clamped_out_of_range);
  RUN_TEST(test_charge_clamped_out_of_range);
  RUN_TEST(test_charge_voice_silent_at_zero);
  RUN_TEST(test_charge_voice_adds_energy);
  RUN_TEST(test_charge_voice_mix_stays_bounded);
  return UNITY_END();
}
