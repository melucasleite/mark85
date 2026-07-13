#include "imu_array.h"

#include <Arduino.h>
#include <Wire.h>

namespace {

// MPU-6050 registers
constexpr uint8_t kRegPwrMgmt1 = 0x6B;
constexpr uint8_t kRegWhoAmI = 0x75;
constexpr uint8_t kRegAccelXoutH = 0x3B;

// PWR_MGMT_1: clock source = X-axis gyro PLL (recommended over internal osc)
constexpr uint8_t kClockPllXGyro = 0x01;

constexpr uint8_t kBurstLen = 14;  // accel(6) + temp(2) + gyro(6)

}  // namespace

bool ImuArray::begin() {
  Wire.begin(config::kI2cSdaPin, config::kI2cSclPin, config::kI2cFrequencyHz);

  if (!probe(config::kMuxAddress)) {
    return false;
  }
  for (uint8_t i = 0; i < config::kImuCount; ++i) {
    imu_ok_[i] = init_imu(i);
  }
  return true;
}

void ImuArray::read(uint8_t index, ImuSample& out) {
  out.valid = false;
  if (index >= config::kImuCount || !imu_ok_[index]) {
    return;
  }
  if (!select_channel(config::kImuMuxChannels[index])) {
    return;
  }

  Wire.beginTransmission(config::kImuAddress);
  Wire.write(kRegAccelXoutH);
  if (Wire.endTransmission(false) != 0) {
    return;
  }
  if (Wire.requestFrom(config::kImuAddress, kBurstLen) != kBurstLen) {
    return;
  }

  uint8_t raw[kBurstLen];
  for (uint8_t i = 0; i < kBurstLen; ++i) {
    raw[i] = static_cast<uint8_t>(Wire.read());
  }
  for (uint8_t axis = 0; axis < 3; ++axis) {
    out.accel_raw[axis] =
        static_cast<int16_t>((raw[axis * 2] << 8) | raw[axis * 2 + 1]);
    // Bytes 6-7 are temperature; gyro starts at byte 8.
    out.gyro_raw[axis] =
        static_cast<int16_t>((raw[8 + axis * 2] << 8) | raw[9 + axis * 2]);
  }
  out.valid = true;
}

bool ImuArray::imu_ok(uint8_t index) const {
  return index < config::kImuCount && imu_ok_[index];
}

bool ImuArray::select_channel(uint8_t channel) {
  Wire.beginTransmission(config::kMuxAddress);
  Wire.write(static_cast<uint8_t>(1u << channel));
  return Wire.endTransmission() == 0;
}

bool ImuArray::probe(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

bool ImuArray::write_imu_reg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(config::kImuAddress);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool ImuArray::init_imu(uint8_t index) {
  if (!select_channel(config::kImuMuxChannels[index])) {
    return false;
  }

  Wire.beginTransmission(config::kImuAddress);
  Wire.write(kRegWhoAmI);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  if (Wire.requestFrom(config::kImuAddress, static_cast<uint8_t>(1)) != 1) {
    return false;
  }
  if (static_cast<uint8_t>(Wire.read()) != config::kImuWhoAmI) {
    return false;
  }

  // Wake from sleep (device boots asleep) and select a stable clock.
  // Default full-scale ranges (+/-2 g, +/-250 dps) are left as-is.
  return write_imu_reg(kRegPwrMgmt1, kClockPllXGyro);
}
