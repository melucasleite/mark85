#pragma once

#include <cstdint>

// Limb naming shared by the telemetry screen and serial logs.
// Pure logic, host-testable.

namespace imu_telemetry {

// "L-ARM", "R-ARM", "L-LEG", "R-LEG"; "?" for out-of-range indices.
const char* limb_name(uint8_t index);

}  // namespace imu_telemetry
