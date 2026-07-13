#pragma once

#include <cstddef>
#include <cstdint>

// One I2S TX channel feeding an NS4168 mono Class-D amp. Two instances exist:
// left speaker on I2S0, right speaker on I2S1. DMA-backed and non-blocking;
// the caller keeps the queue fed from loop().

class AudioOut {
 public:
  AudioOut(uint8_t i2s_port, uint8_t bclk_pin, uint8_t lrclk_pin,
           uint8_t dout_pin);

  // Installs the I2S driver and routes the pins. False on any driver error.
  bool begin();

  // Queues up to `count` mono int16 samples into the DMA buffers without
  // blocking; returns how many were accepted (0 when the queue is full).
  size_t write(const int16_t* samples, size_t count);

 private:
  uint8_t port_;
  uint8_t bclk_pin_;
  uint8_t lrclk_pin_;
  uint8_t dout_pin_;
  bool ready_ = false;
};
