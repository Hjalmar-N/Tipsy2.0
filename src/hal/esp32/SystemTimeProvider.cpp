#include "hal/esp32/SystemTimeProvider.h"

#include <Arduino.h>

namespace tipsy::hal {

std::uint32_t SystemTimeProvider::millis32() const {
  return millis();
}

}  // namespace tipsy::hal

