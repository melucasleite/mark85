#pragma once

// Firmware version, semver: MAJOR = breaking hardware/wiring or protocol
// change, MINOR = new feature, PATCH = fix. Bumped in the same PR as the
// feature; the merge commit on main gets the matching git tag.
inline constexpr char kFirmwareVersion[] = "v0.7.0";
