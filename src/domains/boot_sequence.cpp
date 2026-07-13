#include "boot_sequence.h"

namespace boot_sequence {

Phase phase_at(uint32_t boot_ms, uint32_t now_ms) {
  return (now_ms - boot_ms) >= kSplashDurationMs ? Phase::kMonitor
                                                 : Phase::kSplash;
}

}  // namespace boot_sequence
