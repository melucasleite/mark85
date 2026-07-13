#pragma once

#include <cstddef>
#include <cstdint>

// Single source of truth for all pin assignments and hardware constants.
// GPIO numbers are ESP32-S3 GPIOs (PlatformIO uses GPIO numbering, not the
// Nano Dx/Ax labels). Keep in sync with the README pin table.

namespace config {

// --- I2C bus: HW-617 mux + 4x MPU-6050 ---
inline constexpr uint8_t kI2cSdaPin = 11;  // A4
inline constexpr uint8_t kI2cSclPin = 12;  // A5
inline constexpr uint32_t kI2cFrequencyHz = 400000;

inline constexpr uint8_t kMuxAddress = 0x70;  // HW-617 (PCA9548A)
inline constexpr uint8_t kImuAddress = 0x68;  // MPU-6050, AD0 low
inline constexpr uint8_t kImuWhoAmI = 0x68;

inline constexpr uint8_t kImuCount = 4;
// Mux channel per limb, indexed 0..3: L-ARM, R-ARM, L-LEG, R-LEG
inline constexpr uint8_t kImuMuxChannels[kImuCount] = {0, 1, 2, 3};

// --- SPI display: Waveshare 1.69" ST7789V2, 240x280 ---
inline constexpr uint8_t kLcdMosiPin = 38;      // D11
inline constexpr uint8_t kLcdSckPin = 48;       // D13
inline constexpr uint8_t kLcdCsPin = 21;        // D10
inline constexpr uint8_t kLcdDcPin = 18;        // D9
inline constexpr uint8_t kLcdRstPin = 17;       // D8
inline constexpr uint8_t kLcdBacklightPin = 10; // D7
inline constexpr uint16_t kLcdWidth = 240;
inline constexpr uint16_t kLcdHeight = 280;
inline constexpr uint32_t kLcdSpiHz = 40000000;

// --- I2S audio outputs: left = I2S0, right = I2S1 ---
inline constexpr uint8_t kAudioLeftBclkPin = 1;     // A0
inline constexpr uint8_t kAudioLeftLrclkPin = 2;    // A1
inline constexpr uint8_t kAudioLeftDoutPin = 3;     // A2
inline constexpr uint8_t kAudioRightBclkPin = 4;    // A3
inline constexpr uint8_t kAudioRightLrclkPin = 13;  // A6
inline constexpr uint8_t kAudioRightDoutPin = 14;   // A7
inline constexpr uint32_t kAudioSampleRateHz = 22050;
inline constexpr size_t kAudioChunkSamples = 128;

// --- Servos (LEDC PWM, 50 Hz) ---
inline constexpr uint8_t kServoCount = 4;
inline constexpr uint8_t kServoPins[kServoCount] = {5, 6, 7, 8};  // D2-D5
inline constexpr uint8_t kServoLedcChannels[kServoCount] = {0, 1, 2, 3};
inline constexpr uint32_t kServoPwmHz = 50;
inline constexpr uint8_t kServoPwmResolutionBits = 14;
inline constexpr uint32_t kServoPeriodUs = 20000;  // 1 / 50 Hz
// MG996R: stay inside 600-2400 us — many units bind at their mechanical
// end-stops on the full 500-2500 range and stall-current spike.
inline constexpr uint32_t kServoMinPulseUs = 600;   // 0 degrees
inline constexpr uint32_t kServoMaxPulseUs = 2400;  // 180 degrees

// --- Addressable RGB (HW-159, 7-pixel jewel: index 0 center, 1-6 ring) ---
inline constexpr uint8_t kRgbLeftPin = 9;    // D6
inline constexpr uint8_t kRgbRightPin = 47;  // D12
inline constexpr uint8_t kRgbLedCount = 7;

// --- Scheduling ---
inline constexpr uint32_t kImuSamplePeriodMs = 20;  // 50 Hz sensor reads
inline constexpr uint32_t kDisplayPeriodMs = 100;   // 10 Hz screen refresh
inline constexpr uint32_t kLedPeriodMs = 33;        // ~30 Hz LED animation
inline constexpr uint32_t kServoUpdatePeriodMs = 20;  // 50 Hz angle updates

// --- Serial ---
inline constexpr uint32_t kSerialBaud = 115200;

}  // namespace config
