// Mock test: exists only to prove the host-native test pipeline runs,
// locally and on CI. Replace with real domain tests as domains land.
#include <unity.h>

void setUp() {}
void tearDown() {}

static void test_host_test_runner_executes() {
  TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(test_host_test_runner_executes);
  return UNITY_END();
}
