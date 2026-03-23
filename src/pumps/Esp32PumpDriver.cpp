#include "pumps/Esp32PumpDriver.h"

namespace tipsy::pumps {

bool Esp32PumpDriver::begin() {
  // TODO: Configure one LEDC channel per pump once the final ESP32-S3 pin map is defined.
  stopAll();
  return true;
}

bool Esp32PumpDriver::setPumpSpeed(std::uint8_t pumpIndex, std::uint8_t speedPercent) {
  if (pumpIndex >= speedPercents_.size() || speedPercent > 100) {
    return false;
  }

  speedPercents_[pumpIndex] = speedPercent;

  // TODO: Convert speedPercent to LEDC duty and write it to the mapped PWM channel.
  // Example later: ledcWrite(channel, duty);
  return true;
}

void Esp32PumpDriver::stopPump(std::uint8_t pumpIndex) {
  if (pumpIndex >= speedPercents_.size()) {
    return;
  }

  speedPercents_[pumpIndex] = 0;

  // TODO: Write 0 duty to the corresponding LEDC channel.
}

void Esp32PumpDriver::stopAll() {
  for (std::uint8_t pumpIndex = 0; pumpIndex < speedPercents_.size(); ++pumpIndex) {
    stopPump(pumpIndex);
  }
}

}  // namespace tipsy::pumps

