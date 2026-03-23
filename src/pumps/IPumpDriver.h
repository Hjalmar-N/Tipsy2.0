#pragma once

#include <cstdint>

namespace tipsy::pumps {

// Hardware-facing driver interface for pump speed control.
class IPumpDriver {
 public:
  virtual ~IPumpDriver() = default;

  virtual bool begin() = 0;
  virtual bool setPumpSpeed(std::uint8_t pumpIndex, std::uint8_t speedPercent) = 0;
  virtual void stopPump(std::uint8_t pumpIndex) = 0;
  virtual void stopAll() = 0;
};

}  // namespace tipsy::pumps

