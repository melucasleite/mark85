#include <unity.h>

#include <cstdlib>
#include <cstring>

#include "domains/boot_sound.h"

using boot_sound::fill;
using boot_sound::finished;
using boot_sound::kDurationMs;
using boot_sound::State;

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

static void test_arpeggio_audible_early() {
  State s;
  int16_t buf[kChunk];
  int16_t loudest = 0;
  // First 0.5 s covers the arpeggio notes.
  for (int i = 0; i < static_cast<int>(kRate / 2 / kChunk); ++i) {
    fill(s, kRate, buf, kChunk);
    const int16_t p = peak(buf, kChunk);
    if (p > loudest) loudest = p;
  }
  TEST_ASSERT_TRUE(loudest > 8000);
}

static void test_silent_after_duration() {
  State s;
  int16_t buf[kChunk];
  // Render well past the end.
  const int chunks = static_cast<int>(
      (kDurationMs + 500) * (kRate / 1000.0f) / kChunk) + 1;
  for (int i = 0; i < chunks; ++i) {
    fill(s, kRate, buf, kChunk);
  }
  TEST_ASSERT_TRUE(finished(s, kRate));
  fill(s, kRate, buf, kChunk);
  TEST_ASSERT_EQUAL_INT16(0, peak(buf, kChunk));
}

static void test_not_finished_midway() {
  State s;
  int16_t buf[kChunk];
  for (int i = 0; i < 10; ++i) {
    fill(s, kRate, buf, kChunk);
  }
  TEST_ASSERT_FALSE(finished(s, kRate));
}

static void test_deterministic() {
  State a;
  State b;
  int16_t buf_a[kChunk];
  int16_t buf_b[kChunk];
  for (int i = 0; i < 40; ++i) {
    fill(a, kRate, buf_a, kChunk);
    fill(b, kRate, buf_b, kChunk);
    TEST_ASSERT_EQUAL_MEMORY(buf_a, buf_b, sizeof(buf_a));
  }
}

static void test_bounded_throughout() {
  State s;
  int16_t buf[kChunk];
  const int chunks =
      static_cast<int>(kDurationMs * (kRate / 1000.0f) / kChunk) + 1;
  for (int i = 0; i < chunks; ++i) {
    fill(s, kRate, buf, kChunk);
    for (size_t j = 0; j < kChunk; ++j) {
      TEST_ASSERT_TRUE(buf[j] >= -32767 && buf[j] <= 32767);
    }
  }
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_arpeggio_audible_early);
  RUN_TEST(test_silent_after_duration);
  RUN_TEST(test_not_finished_midway);
  RUN_TEST(test_deterministic);
  RUN_TEST(test_bounded_throughout);
  return UNITY_END();
}
