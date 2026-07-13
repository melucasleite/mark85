#include <unity.h>

#include <cstring>

#include "domains/telemetry_view.h"

using telemetry_view::format_flap_line;
using telemetry_view::format_uptime_line;
using telemetry_view::kLineBufLen;
using telemetry_view::kLineChars;

void setUp() {}
void tearDown() {}

static void test_flap_line_formats_angle() {
  char buf[kLineBufLen];
  format_flap_line(0.0f, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("FLAPS 000", buf);
  format_flap_line(73.4f, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("FLAPS 073", buf);
  format_flap_line(180.0f, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("FLAPS 180", buf);
}

static void test_flap_line_clamps_out_of_range() {
  char buf[kLineBufLen];
  format_flap_line(-15.0f, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("FLAPS 000", buf);
  format_flap_line(250.0f, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("FLAPS 180", buf);
}

static void test_uptime_line_formats_hms() {
  char buf[kLineBufLen];
  format_uptime_line(0, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("UP 00:00:00", buf);
  format_uptime_line(3723000, buf, sizeof(buf));  // 1h 2m 3s
  TEST_ASSERT_EQUAL_STRING("UP 01:02:03", buf);
  format_uptime_line(86399000, buf, sizeof(buf));  // 23:59:59
  TEST_ASSERT_EQUAL_STRING("UP 23:59:59", buf);
}

static void test_uptime_hours_wrap_at_100() {
  char buf[kLineBufLen];
  format_uptime_line(360000000u, buf, sizeof(buf));  // exactly 100 h
  TEST_ASSERT_EQUAL_STRING("UP 00:00:00", buf);
}

static void test_lines_never_exceed_display_width() {
  char buf[kLineBufLen];
  const float flaps[] = {-999.0f, 0.0f, 90.5f, 180.0f, 9999.0f};
  for (const float f : flaps) {
    format_flap_line(f, buf, sizeof(buf));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(kLineChars, std::strlen(buf));
  }
  const uint32_t times[] = {0u, 3723000u, 86399000u, 0xFFFFFFFFu};
  for (const uint32_t t : times) {
    format_uptime_line(t, buf, sizeof(buf));
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(kLineChars, std::strlen(buf));
  }
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_flap_line_formats_angle);
  RUN_TEST(test_flap_line_clamps_out_of_range);
  RUN_TEST(test_uptime_line_formats_hms);
  RUN_TEST(test_uptime_hours_wrap_at_100);
  RUN_TEST(test_lines_never_exceed_display_width);
  return UNITY_END();
}
