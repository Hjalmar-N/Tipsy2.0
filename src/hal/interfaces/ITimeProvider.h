#pragma once

#include <cstdint>

namespace tipsy::hal {

// Time abstraction used by non-blocking services and future scheduling.
class ITimeProvider {
 public:
  virtual ~ITimeProvider() = default;

  virtual std::uint32_t millis32() const = 0;
};

}  // namespace tipsy::hal

