#pragma once

#include "hal/interfaces/ITimeProvider.h"

namespace tipsy::hal {

// Wraps Arduino millis() for non-blocking timing.
class SystemTimeProvider final : public ITimeProvider {
 public:
  std::uint32_t millis32() const override;
};

}  // namespace tipsy::hal

