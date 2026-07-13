#pragma once

#include <cstdint>

#include "../config.h"
#include "../domains/imu_types.h"

// 4x MPU-6050 behind the HW-617 (PCA9548A) I2C multiplexer.
// Owns all I2C traffic to the mux and the IMUs; nothing outside this driver
// touches Wire for these devices.

class ImuArray {
 public:
  // Initializes the I2C bus, probes the mux, then wakes and verifies each
  // IMU on its channel. Returns false only if the mux itself is unreachable;
  // individual missing IMUs are tolerated and reported via imu_ok().
  bool begin();

  // Reads one IMU's accel+gyro into `out`. Selects the mux channel, does the
  // burst read, and sets out.valid accordingly. Safe to call for an IMU that
  // failed init: the sample simply comes back invalid.
  void read(uint8_t index, ImuSample& out);

  bool imu_ok(uint8_t index) const;

 private:
  bool select_channel(uint8_t channel);
  bool probe(uint8_t address);
  bool write_imu_reg(uint8_t reg, uint8_t value);
  bool init_imu(uint8_t index);

  bool imu_ok_[config::kImuCount] = {};
};
