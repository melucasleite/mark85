#include "servos.h"

#include <Arduino.h>

bool Servos::begin() {
  for (uint8_t i = 0; i < config::kServoCount; ++i) {
    if (ledcSetup(config::kServoLedcChannels[i], config::kServoPwmHz,
                  config::kServoPwmResolutionBits) == 0) {
      return false;
    }
    ledcAttachPin(config::kServoPins[i], config::kServoLedcChannels[i]);
  }
  ready_ = true;
  for (uint8_t i = 0; i < config::kServoCount; ++i) {
    set_angle(i, 0.0f);
  }
  return true;
}

void Servos::set_angle(uint8_t index, float degrees) {
  if (!ready_ || index >= config::kServoCount) {
    return;
  }
  if (degrees < 0.0f) degrees = 0.0f;
  if (degrees > 180.0f) degrees = 180.0f;

  const float pulse_us =
      config::kServoMinPulseUs +
      (degrees / 180.0f) *
          (config::kServoMaxPulseUs - config::kServoMinPulseUs);
  const uint32_t max_duty = (1u << config::kServoPwmResolutionBits) - 1u;
  const uint32_t duty = static_cast<uint32_t>(
      pulse_us * max_duty / config::kServoPeriodUs);
  ledcWrite(config::kServoLedcChannels[index], duty);
}
