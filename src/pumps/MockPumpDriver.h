#pragma once

#include <array>
#include <cstdint>

#include "config/AppConfig.h"
#include "pumps/IPumpDriver.h"

namespace tipsy::pumps {

// Mock driver used while UI and control flow are developed before real hardware is wired.
class MockPumpDriver final : public IPumpDriver {
 public:
  bool begin() override;
  bool setPumpSpeed(std::uint8_t pumpIndex, std::uint8_t speedPercent) override;
  void stopPump(std::uint8_t pumpIndex) override;
  void stopAll() override;

 private:
  std::array<std::uint8_t, tipsy::config::kPumpCount> speedPercents_ {};
};

}  // namespace tipsy::pumps

