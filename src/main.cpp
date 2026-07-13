#include <Arduino.h>

#include "config.h"
#include "domains/boot_sequence.h"
#include "domains/imu_telemetry.h"
#include "domains/imu_types.h"
#include "drivers/display.h"
#include "drivers/imu_array.h"

namespace {

ImuArray imu_array;
Display display;

ImuSample samples[config::kImuCount];
uint32_t boot_ms = 0;
bool monitor_started = false;
uint32_t last_sample_ms = 0;
uint32_t last_draw_ms = 0;

void draw_telemetry() {
  char line[imu_telemetry::kLineBufLen];
  for (uint8_t i = 0; i < config::kImuCount; ++i) {
    imu_telemetry::format_header(i, samples[i].valid, line, sizeof(line));
    display.draw_card_header(i, line, samples[i].valid);
    imu_telemetry::format_accel_line(samples[i], line, sizeof(line));
    display.draw_card_value(i, 0, line);
    imu_telemetry::format_gyro_line(samples[i], line, sizeof(line));
    display.draw_card_value(i, 1, line);
  }
}

}  // namespace

void setup() {
  Serial.begin(config::kSerialBaud);

  const bool display_ok = display.begin();
  Serial.printf("[display] %s\n", display_ok ? "ok" : "FAILED");

  display.draw_splash();
  boot_ms = millis();

  // IMU init runs while the splash is on screen.
  const bool mux_ok = imu_array.begin();
  Serial.printf("[imu] mux %s\n", mux_ok ? "ok" : "FAILED");
  for (uint8_t i = 0; i < config::kImuCount; ++i) {
    Serial.printf("[imu] %u %s: %s\n", i, imu_telemetry::limb_name(i),
                  imu_array.imu_ok(i) ? "ok" : "FAILED");
  }
}

void loop() {
  const uint32_t now = millis();

  if (boot_sequence::phase_at(boot_ms, now) == boot_sequence::Phase::kSplash) {
    return;
  }
  if (!monitor_started) {
    monitor_started = true;
    display.begin_monitor_screen();
  }

  if (now - last_sample_ms >= config::kImuSamplePeriodMs) {
    last_sample_ms = now;
    for (uint8_t i = 0; i < config::kImuCount; ++i) {
      imu_array.read(i, samples[i]);
    }
  }

  if (now - last_draw_ms >= config::kDisplayPeriodMs) {
    last_draw_ms = now;
    draw_telemetry();
  }
}
