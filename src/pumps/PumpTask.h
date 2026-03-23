#pragma once

#include <cstdint>

namespace tipsy::pumps {

// Runtime representation of one active or pending pump pour.
struct PumpTask {
  bool active = false;
  std::uint8_t pumpIndex = 0;
  float targetVolumeMl = 0.0F;
  std::uint8_t speedPercent = 0;
  float flowMlPerSecond = 0.0F;
  std::uint32_t startedAtMs = 0;
  std::uint32_t durationMs = 0;

  std::uint32_t endAtMs() const {
    return startedAtMs + durationMs;
  }
};

}  // namespace tipsy::pumps

