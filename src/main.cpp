#include <Arduino.h>

#include "config.h"
#include "domains/blaster_charge.h"
#include "domains/boot_sequence.h"
#include "domains/boot_sound.h"
#include "domains/imu_telemetry.h"
#include "domains/imu_types.h"
#include "domains/pose_calibration.h"
#include "domains/servo_sweep.h"
#include "domains/suit_sound.h"
#include "domains/telemetry_view.h"
#include "drivers/audio_out.h"
#include "drivers/display.h"
#include "drivers/imu_array.h"
#include "drivers/rgb_leds.h"
#include "drivers/servos.h"
#include "version.h"

namespace {

ImuArray imu_array;
Display display;
RgbLeds rgb_leds;
Servos servos;
AudioOut audio_left(0, config::kAudioLeftBclkPin, config::kAudioLeftLrclkPin,
                    config::kAudioLeftDoutPin);
AudioOut audio_right(1, config::kAudioRightBclkPin,
                     config::kAudioRightLrclkPin, config::kAudioRightDoutPin);

ImuSample samples[config::kImuCount];
suit_sound::State sound;
boot_sound::State boot_jingle_left, boot_jingle_right;
// Neutral-pose gravity is captured per limb while the splash shows.
pose_calibration::Calibrator calibrators[config::kImuCount];
uint32_t boot_ms = 0;
bool monitor_started = false;
// Boot-time subsystem health, shown on the telemetry screen.
bool sys_mux_ok = false;
bool sys_audio_ok = false;
bool sys_rgb_ok = false;
bool sys_servos_ok = false;
float servo_angle_deg = 0.0f;
uint32_t last_sample_ms = 0;
uint32_t last_draw_ms = 0;
uint32_t last_led_ms = 0;
uint32_t last_servo_ms = 0;

// Chunk staging between the synth and a DMA queue that may accept partial
// writes; carries the remainder into the next loop pass.
struct AudioFeed {
  int16_t buf[config::kAudioChunkSamples];
  size_t len = 0;
  size_t off = 0;
};
AudioFeed feed_left, feed_right;

template <typename RenderFn>
void pump_audio(AudioFeed& feed, AudioOut& out, RenderFn render) {
  if (feed.off == feed.len) {
    render(feed.buf, config::kAudioChunkSamples);
    feed.len = config::kAudioChunkSamples;
    feed.off = 0;
  }
  feed.off += out.write(feed.buf + feed.off, feed.len - feed.off);
}

void render_suit_left(int16_t* buf, size_t count) {
  suit_sound::render_left(sound, config::kAudioSampleRateHz, buf, count);
}
void render_suit_right(int16_t* buf, size_t count) {
  suit_sound::render_right(sound, config::kAudioSampleRateHz, buf, count);
}
void render_boot_left(int16_t* buf, size_t count) {
  boot_sound::fill(boot_jingle_left, config::kAudioSampleRateHz, buf, count);
}
void render_boot_right(int16_t* buf, size_t count) {
  boot_sound::fill(boot_jingle_right, config::kAudioSampleRateHz, buf, count);
}

void draw_telemetry(uint32_t now) {
  for (uint8_t i = 0; i < config::kImuCount; ++i) {
    display.draw_limb_status(i, samples[i].valid);
  }
  display.draw_meter(0, sound.left.intensity.level);
  display.draw_meter(1, sound.right.intensity.level);
  display.draw_meter(2, sound.left.blaster.charge);
  display.draw_meter(3, sound.right.blaster.charge);
  display.draw_cal_status(sound.left.arm_down.valid,
                          sound.right.arm_down.valid);
  display.draw_system_status(sys_mux_ok, sys_audio_ok, sys_rgb_ok,
                             sys_servos_ok);

  char line[telemetry_view::kLineBufLen];
  telemetry_view::format_flap_line(servo_angle_deg, line, sizeof(line));
  display.draw_flap_line(line);
  telemetry_view::format_uptime_line(now, line, sizeof(line));
  display.draw_uptime_line(line);
}

}  // namespace

void setup() {
  Serial.begin(config::kSerialBaud);
  Serial.printf("[fw] %s\n", kFirmwareVersion);

  const bool display_ok = display.begin();
  Serial.printf("[display] %s\n", display_ok ? "ok" : "FAILED");

  display.draw_splash(kFirmwareVersion);
  boot_ms = millis();

  const bool audio_left_ok = audio_left.begin();
  Serial.printf("[audio L] %s\n", audio_left_ok ? "ok" : "FAILED");
  const bool audio_right_ok = audio_right.begin();
  Serial.printf("[audio R] %s\n", audio_right_ok ? "ok" : "FAILED");
  sys_audio_ok = audio_left_ok && audio_right_ok;

  sys_rgb_ok = rgb_leds.begin();
  Serial.printf("[rgb] %s\n", sys_rgb_ok ? "ok" : "FAILED");

  sys_servos_ok = servos.begin();
  Serial.printf("[servos] %s\n", sys_servos_ok ? "ok" : "FAILED");

  // IMU init runs while the splash is on screen.
  sys_mux_ok = imu_array.begin();
  Serial.printf("[imu] mux %s\n", sys_mux_ok ? "ok" : "FAILED");
  for (uint8_t i = 0; i < config::kImuCount; ++i) {
    Serial.printf("[imu] %u %s: %s\n", i, imu_telemetry::limb_name(i),
                  imu_array.imu_ok(i) ? "ok" : "FAILED");
  }
}

void loop() {
  const uint32_t now = millis();
  const bool in_splash =
      boot_sequence::phase_at(boot_ms, now) == boot_sequence::Phase::kSplash;

  if (now - last_sample_ms >= config::kImuSamplePeriodMs) {
    last_sample_ms = now;
    for (uint8_t i = 0; i < config::kImuCount; ++i) {
      imu_array.read(i, samples[i]);
    }
    if (in_splash) {
      // The splash doubles as the calibration window: the wearer stands in
      // the neutral pose while each limb's gravity direction is averaged.
      for (uint8_t i = 0; i < config::kImuCount; ++i) {
        pose_calibration::feed(calibrators[i], samples[i]);
      }
    } else {
      suit_sound::update_motion(sound, samples, now,
                                config::kImuSamplePeriodMs / 1000.0f);
    }
  }

  if (in_splash) {
    // Boot jingle plays over the splash while calibration collects samples,
    // and the grid floor scrolls toward the viewer.
    pump_audio(feed_left, audio_left, render_boot_left);
    pump_audio(feed_right, audio_right, render_boot_right);
    if (now - last_led_ms >= config::kLedPeriodMs) {
      last_led_ms = now;
      display.draw_splash_floor(now);
    }
    return;
  }
  if (!monitor_started) {
    monitor_started = true;
    sound.left.arm_down =
        pose_calibration::finalize(calibrators[suit_sound::kLeftArm]);
    sound.right.arm_down =
        pose_calibration::finalize(calibrators[suit_sound::kRightArm]);
    Serial.printf("[cal] L-ARM %s, R-ARM %s\n",
                  sound.left.arm_down.valid ? "ok" : "FAILED (blaster off)",
                  sound.right.arm_down.valid ? "ok" : "FAILED (blaster off)");
    display.begin_monitor_screen();
  }

  pump_audio(feed_left, audio_left, render_suit_left);
  pump_audio(feed_right, audio_right, render_suit_right);

  if (now - last_led_ms >= config::kLedPeriodMs) {
    last_led_ms = now;
    RgbColor jewel[blaster_charge::kLedCount];
    blaster_charge::render_leds(sound.left.blaster.charge, now, jewel);
    rgb_leds.show_left(jewel, blaster_charge::kLedCount);
    blaster_charge::render_leds(sound.right.blaster.charge, now, jewel);
    rgb_leds.show_right(jewel, blaster_charge::kLedCount);
  }

  if (now - last_servo_ms >= config::kServoUpdatePeriodMs) {
    last_servo_ms = now;
    // Bench-test sweep; will become flap control.
    servo_angle_deg = servo_sweep::angle_at(now);
    for (uint8_t i = 0; i < config::kServoCount; ++i) {
      servos.set_angle(i, servo_angle_deg);
    }
  }

  if (now - last_draw_ms >= config::kDisplayPeriodMs) {
    last_draw_ms = now;
    draw_telemetry(now);
  }
}
