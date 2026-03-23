#pragma once

#include <cstdint>

namespace tipsy::domain {

// Placeholder pump flow and PWM tuning values persisted for each pump.
struct PumpCalibration {
  std::uint8_t pumpIndex = 0;
  bool enabled = true;
  float mlPerSecond = 10.0F;
  std::uint16_t pwmDuty = 512;
  std::uint16_t primeDurationMs = 1000;
  std::uint16_t settleDurationMs = 250;

  std::uint32_t estimatePourTimeMs(float amountMl) const;
  bool isValid() const;
};

}  // namespace tipsy::domain
