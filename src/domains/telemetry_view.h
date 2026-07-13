#pragma once

#include <cstddef>
#include <cstdint>

// Formats the dynamic text lines of the telemetry screen.
// Pure logic, host-testable: no Arduino, no drivers, no I/O.

namespace telemetry_view {

// Display line budget: 240 px / 12 px per char at text size 2.
inline constexpr size_t kLineChars = 20;
inline constexpr size_t kLineBufLen = kLineChars + 1;

// "FLAPS 073" — current servo angle, clamped to 0..180.
void format_flap_line(float degrees, char* buf, size_t len);

// "UP 01:02:03" — uptime as hh:mm:ss, hours wrapping at 100.
void format_uptime_line(uint32_t uptime_ms, char* buf, size_t len);

}  // namespace telemetry_view
