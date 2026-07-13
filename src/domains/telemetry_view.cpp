#include "telemetry_view.h"

#include <cstdio>

namespace telemetry_view {

void format_flap_line(float degrees, char* buf, size_t len) {
  if (degrees < 0.0f) degrees = 0.0f;
  if (degrees > 180.0f) degrees = 180.0f;
  std::snprintf(buf, len, "FLAPS %03d", static_cast<int>(degrees));
}

void format_uptime_line(uint32_t uptime_ms, char* buf, size_t len) {
  const uint32_t total_s = uptime_ms / 1000u;
  const uint32_t h = (total_s / 3600u) % 100u;
  const uint32_t m = (total_s / 60u) % 60u;
  const uint32_t s = total_s % 60u;
  std::snprintf(buf, len, "UP %02u:%02u:%02u",
                static_cast<unsigned>(h), static_cast<unsigned>(m),
                static_cast<unsigned>(s));
}

}  // namespace telemetry_view
