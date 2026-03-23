#pragma once

#include <cstdint>

namespace tipsy::app {

// High-level application states that the UI can render without knowing business details.
enum class MachineState : std::uint8_t {
  Idle = 0,
  DrinkSelection = 1,
  ManualPour = 2,
  AdminSettings = 3,
  Pouring = 4,
  Complete = 5,
  Error = 6,
  Maintenance = 7,
};

}  // namespace tipsy::app

