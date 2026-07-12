#include <unity.h>

#include <cstring>

#include "domains/imu_telemetry.h"

using imu_telemetry::kLineBufLen;
using imu_telemetry::kLineChars;

void setUp() {}
void tearDown() {}

static ImuSample make_sample(int16_t ax, int16_t ay, int16_t az, int16_t gx,
                             int16_t gy, int16_t gz) {
  ImuSample s;
  s.accel_raw[0] = ax;
  s.accel_raw[1] = ay;
  s.accel_raw[2] = az;
  s.gyro_raw[0] = gx;
  s.gyro_raw[1] = gy;
  s.gyro_raw[2] = gz;
  s.valid = true;
  return s;
}

static void test_limb_names() {
  TEST_ASSERT_EQUAL_STRING("L-ARM", imu_telemetry::limb_name(0));
  TEST_ASSERT_EQUAL_STRING("R-ARM", imu_telemetry::limb_name(1));
  TEST_ASSERT_EQUAL_STRING("L-LEG", imu_telemetry::limb_name(2));
  TEST_ASSERT_EQUAL_STRING("R-LEG", imu_telemetry::limb_name(3));
  TEST_ASSERT_EQUAL_STRING("?", imu_telemetry::limb_name(4));
}

static void test_header_online() {
  char buf[kLineBufLen];
  imu_telemetry::format_header(2, true, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("2 L-LEG", buf);
}

static void test_header_offline() {
  char buf[kLineBufLen];
  imu_telemetry::format_header(1, false, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("1 R-ARM OFFLINE", buf);
}

static void test_accel_line_converts_to_g() {
  // 16384 LSB = 1 g at the +/-2 g range
  const ImuSample s = make_sample(16384, 0, -16384, 0, 0, 0);
  char buf[kLineBufLen];
  imu_telemetry::format_accel_line(s, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("A +1.00 +0.00 -1.00", buf);
}

static void test_gyro_line_converts_to_dps() {
  // 131 LSB = 1 dps at the +/-250 dps range
  const ImuSample s = make_sample(0, 0, 0, 1310, -32750, 0);
  char buf[kLineBufLen];
  imu_telemetry::format_gyro_line(s, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("G  +10 -250   +0", buf);
}

static void test_invalid_sample_renders_dashes() {
  ImuSample s;  // valid = false by default
  char buf[kLineBufLen];
  imu_telemetry::format_accel_line(s, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("A    --    --    --", buf);
  imu_telemetry::format_gyro_line(s, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("G    --    --    --", buf);
}

static void test_lines_never_exceed_display_width() {
  // Extreme raw values must still fit the 20-column display.
  const ImuSample extremes[] = {
      make_sample(-32768, 32767, -32768, 32767, -32768, 32767),
      make_sample(32767, -32768, 32767, -32768, 32767, -32768),
  };
  char buf[kLineBufLen];
  for (const ImuSample& s : extremes) {
    imu_telemetry::format_accel_line(s, buf, sizeof(buf));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(kLineChars, std::strlen(buf));
    imu_telemetry::format_gyro_line(s, buf, sizeof(buf));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(kLineChars, std::strlen(buf));
  }
  for (uint8_t i = 0; i < 4; ++i) {
    imu_telemetry::format_header(i, false, buf, sizeof(buf));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(kLineChars, std::strlen(buf));
  }
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_limb_names);
  RUN_TEST(test_header_online);
  RUN_TEST(test_header_offline);
  RUN_TEST(test_accel_line_converts_to_g);
  RUN_TEST(test_gyro_line_converts_to_dps);
  RUN_TEST(test_invalid_sample_renders_dashes);
  RUN_TEST(test_lines_never_exceed_display_width);
  return UNITY_END();
}
