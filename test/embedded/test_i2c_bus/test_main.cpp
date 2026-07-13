// On-target I2C smoke test: verifies the HW-617 mux is reachable and that
// each wired IMU answers WHO_AM_I through its channel. Also scans the full
// bus and reports every responding address, which makes it a wiring
// diagnostic: run with `pio test -e arduino_nano_esp32 -v`.
#include <Arduino.h>
#include <Wire.h>
#include <unity.h>

#include <cstdio>

#include "config.h"

namespace {

constexpr uint8_t kRegWhoAmI = 0x75;
// Diagnostic speed: slow enough to work with weak/absent pullups.
constexpr uint32_t kDiagI2cHz = 100000;

bool probe(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool select_channel(uint8_t channel) {
  Wire.beginTransmission(config::kMuxAddress);
  Wire.write(static_cast<uint8_t>(1u << channel));
  return Wire.endTransmission() == 0;
}

}  // namespace

void setUp() {}
void tearDown() {}

static void test_report_idle_line_levels() {
  char msg[56];
  std::snprintf(msg, sizeof(msg), "idle levels: SDA=%d SCL=%d",
                digitalRead(config::kI2cSdaPin),
                digitalRead(config::kI2cSclPin));
  TEST_MESSAGE(msg);
}

static void test_scan_reports_all_devices() {
  char msg[48];
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; ++addr) {
    if (probe(addr)) {
      std::snprintf(msg, sizeof(msg), "device responds at 0x%02X", addr);
      TEST_MESSAGE(msg);
      ++found;
    }
  }
  std::snprintf(msg, sizeof(msg), "scan complete: %u device(s)", found);
  TEST_MESSAGE(msg);
  TEST_ASSERT_GREATER_THAN_UINT8(0, found);
}

static void test_scan_with_pins_swapped() {
  // If devices appear here but not in the normal scan, the SDA and SCL
  // wires are physically swapped (GY-521 header order is VCC GND SCL SDA).
  Wire.end();
  Wire.begin(config::kI2cSclPin, config::kI2cSdaPin, kDiagI2cHz);
  char msg[48];
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; ++addr) {
    if (probe(addr)) {
      std::snprintf(msg, sizeof(msg), "SWAPPED: device at 0x%02X", addr);
      TEST_MESSAGE(msg);
      ++found;
    }
  }
  std::snprintf(msg, sizeof(msg), "swapped scan: %u device(s)", found);
  TEST_MESSAGE(msg);
  Wire.end();
  Wire.begin(config::kI2cSdaPin, config::kI2cSclPin, kDiagI2cHz);
}

static void test_mux_responds_at_configured_address() {
  TEST_ASSERT_TRUE_MESSAGE(probe(config::kMuxAddress),
                           "HW-617 mux not answering at 0x70");
}

static void test_left_arm_imu_who_am_i() {
  TEST_ASSERT_TRUE_MESSAGE(select_channel(config::kImuMuxChannels[0]),
                           "mux channel select failed");
  Wire.beginTransmission(config::kImuAddress);
  Wire.write(kRegWhoAmI);
  TEST_ASSERT_EQUAL_MESSAGE(0, Wire.endTransmission(false),
                            "IMU register address NACKed");
  TEST_ASSERT_EQUAL_MESSAGE(
      1, Wire.requestFrom(config::kImuAddress, static_cast<uint8_t>(1)),
      "IMU read returned no data");
  TEST_ASSERT_EQUAL_HEX8(config::kImuWhoAmI,
                         static_cast<uint8_t>(Wire.read()));
}

void setup() {
  delay(2000);  // allow USB CDC to enumerate so test output is captured
  Wire.begin(config::kI2cSdaPin, config::kI2cSclPin, kDiagI2cHz);

  UNITY_BEGIN();
  RUN_TEST(test_report_idle_line_levels);
  RUN_TEST(test_scan_reports_all_devices);
  RUN_TEST(test_scan_with_pins_swapped);
  RUN_TEST(test_mux_responds_at_configured_address);
  RUN_TEST(test_left_arm_imu_who_am_i);
  UNITY_END();
}

void loop() {}
