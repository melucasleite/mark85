#pragma once

#include <cstddef>
#include <cstdint>

#include "imu_types.h"

// Formats IMU samples into fixed-width display lines.
// Pure logic, host-testable: no Arduino, no drivers, no I/O.

namespace imu_telemetry {

// Display line budget: 240 px / 12 px per char at text size 2.
inline constexpr size_t kLineChars = 20;
inline constexpr size_t kLineBufLen = kLineChars + 1;

// MPU-6050 LSB scale factors at default full-scale ranges.
inline constexpr float kAccelLsbPerG = 16384.0f;  // +/-2 g
inline constexpr float kGyroLsbPerDps = 131.0f;   // +/-250 dps

// "L-ARM", "R-ARM", "L-LEG", "R-LEG"; "?" for out-of-range indices.
const char* limb_name(uint8_t index);

// "0 L-ARM" or "0 L-ARM OFFLINE"
void format_header(uint8_t index, bool valid, char* buf, size_t len);

// "A +0.02 +0.98 -1.00" (g), or dashes when the sample is invalid
void format_accel_line(const ImuSample& sample, char* buf, size_t len);

// "G  +10 -250   +0" (dps), or dashes when the sample is invalid
void format_gyro_line(const ImuSample& sample, char* buf, size_t len);

}  // namespace imu_telemetry
