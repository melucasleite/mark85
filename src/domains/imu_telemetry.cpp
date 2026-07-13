#include "imu_telemetry.h"

#include <cstdio>

namespace imu_telemetry {

const char* limb_name(uint8_t index) {
  switch (index) {
    case 0: return "L-ARM";
    case 1: return "R-ARM";
    case 2: return "L-LEG";
    case 3: return "R-LEG";
    default: return "?";
  }
}

void format_header(uint8_t index, bool valid, char* buf, size_t len) {
  std::snprintf(buf, len, "%u %s%s", static_cast<unsigned>(index),
                limb_name(index), valid ? "" : " OFFLINE");
}

void format_accel_line(const ImuSample& sample, char* buf, size_t len) {
  if (!sample.valid) {
    std::snprintf(buf, len, "A    --    --    --");
    return;
  }
  std::snprintf(buf, len, "A %+5.2f %+5.2f %+5.2f",
                static_cast<double>(sample.accel_raw[0] / kAccelLsbPerG),
                static_cast<double>(sample.accel_raw[1] / kAccelLsbPerG),
                static_cast<double>(sample.accel_raw[2] / kAccelLsbPerG));
}

void format_gyro_line(const ImuSample& sample, char* buf, size_t len) {
  if (!sample.valid) {
    std::snprintf(buf, len, "G    --    --    --");
    return;
  }
  std::snprintf(buf, len, "G %+4d %+4d %+4d",
                static_cast<int>(sample.gyro_raw[0] / kGyroLsbPerDps),
                static_cast<int>(sample.gyro_raw[1] / kGyroLsbPerDps),
                static_cast<int>(sample.gyro_raw[2] / kGyroLsbPerDps));
}

}  // namespace imu_telemetry
