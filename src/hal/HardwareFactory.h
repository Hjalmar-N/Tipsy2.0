#pragma once

#include "hal/HardwareBundle.h"

namespace tipsy::hal {

// Centralizes construction of mock or real hardware implementations.
class HardwareFactory {
 public:
  static HardwareBundle create();
};

}  // namespace tipsy::hal

