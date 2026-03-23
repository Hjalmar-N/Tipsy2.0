#include "pumps/MockPumpDriver.h"

namespace tipsy::pumps {

bool MockPumpDriver::begin() {
  stopAll();
  return true;
}

bool MockPumpDriver::setPumpSpeed(std::uint8_t pumpIndex, std::uint8_t speedPercent) {
  if (pumpIndex >= speedPercents_.size() || speedPercent > 100) {
    return false;
  }

  speedPercents_[pumpIndex] = speedPercent;
  return true;
}

void MockPumpDriver::stopPump(std::uint8_t pumpIndex) {
  if (pumpIndex >= speedPercents_.size()) {
    return;
  }

  speedPercents_[pumpIndex] = 0;
}

void MockPumpDriver::stopAll() {
  for (std::uint8_t pumpIndex = 0; pumpIndex < speedPercents_.size(); ++pumpIndex) {
    speedPercents_[pumpIndex] = 0;
  }
}

}  // namespace tipsy::pumps

