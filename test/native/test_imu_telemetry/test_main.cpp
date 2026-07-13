#include <unity.h>

#include "domains/imu_telemetry.h"

void setUp() {}
void tearDown() {}

static void test_limb_names() {
  TEST_ASSERT_EQUAL_STRING("L-ARM", imu_telemetry::limb_name(0));
  TEST_ASSERT_EQUAL_STRING("R-ARM", imu_telemetry::limb_name(1));
  TEST_ASSERT_EQUAL_STRING("L-LEG", imu_telemetry::limb_name(2));
  TEST_ASSERT_EQUAL_STRING("R-LEG", imu_telemetry::limb_name(3));
  TEST_ASSERT_EQUAL_STRING("?", imu_telemetry::limb_name(4));
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_limb_names);
  return UNITY_END();
}
