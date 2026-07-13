#include <Arduino.h>

#include "config.h"
#include "domains/blaster_charge.h"
#include "domains/boot_sequence.h"
#include "domains/imu_telemetry.h"
#include "domains/imu_types.h"
#include "domains/suit_sound.h"
#include "drivers/audio_out.h"
#include "drivers/display.h"
#include "drivers/imu_array.h"
#include "drivers/rgb_leds.h"

namespace {

ImuArray imu_array;
Display display;
RgbLeds rgb_leds;
AudioOut audio_left(0, config::kAudioLeftBclkPin, config::kAudioLeftLrclkPin,
                    config::kAudioLeftDoutPin);
AudioOut audio_right(1, config::kAudioRightBclkPin,
                     config::kAudioRightLrclkPin, config::kAudioRightDoutPin);

ImuSample samples[config::kImuCount];
suit_sound::State sound;
uint32_t boot_ms = 0;
bool monitor_started = false;
uint32_t last_sample_ms = 0;
uint32_t last_draw_ms = 0;
uint32_t last_led_ms = 0;

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
    render(sound, config::kAudioSampleRateHz, feed.buf,
           config::kAudioChunkSamples);
    feed.len = config::kAudioChunkSamples;
    feed.off = 0;
  }
  feed.off += out.write(feed.buf + feed.off, feed.len - feed.off);
}

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

  const bool audio_left_ok = audio_left.begin();
  Serial.printf("[audio L] %s\n", audio_left_ok ? "ok" : "FAILED");
  const bool audio_right_ok = audio_right.begin();
  Serial.printf("[audio R] %s\n", audio_right_ok ? "ok" : "FAILED");

  const bool rgb_ok = rgb_leds.begin();
  Serial.printf("[rgb] %s\n", rgb_ok ? "ok" : "FAILED");

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
    suit_sound::update_motion(sound, samples, now,
                              config::kImuSamplePeriodMs / 1000.0f);
  }

  pump_audio(feed_left, audio_left, suit_sound::render_left);
  pump_audio(feed_right, audio_right, suit_sound::render_right);

  if (now - last_led_ms >= config::kLedPeriodMs) {
    last_led_ms = now;
    RgbColor jewel[blaster_charge::kLedCount];
    blaster_charge::render_leds(sound.blaster.charge, now, jewel);
    rgb_leds.show_left(jewel, blaster_charge::kLedCount);
  }

  if (now - last_draw_ms >= config::kDisplayPeriodMs) {
    last_draw_ms = now;
    draw_telemetry();
  }
}
