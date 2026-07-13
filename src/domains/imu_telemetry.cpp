#include "imu_telemetry.h"

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

}  // namespace imu_telemetry
