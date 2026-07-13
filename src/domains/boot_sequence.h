#pragma once

#include <cstdint>

// Boot phase timing: splash screen first, then the live monitor.
// Pure logic, host-testable: time arrives as a parameter.

namespace boot_sequence {

inline constexpr uint32_t kSplashDurationMs = 5000;

enum class Phase : uint8_t {
  kSplash,
  kMonitor,
};

// Phase at `now_ms`, given the boot timestamp. Robust against millis()
// wraparound (unsigned subtraction).
Phase phase_at(uint32_t boot_ms, uint32_t now_ms);

}  // namespace boot_sequence
