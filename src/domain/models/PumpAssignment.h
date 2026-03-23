#pragma once

#include <Arduino.h>
#include <cstdint>

namespace tipsy::domain {

// Maps a physical pump channel to a logical ingredient.
struct PumpAssignment {
  std::uint8_t pumpIndex = 0;
  bool enabled = true;
  String ingredientId;
  String ingredientDisplayName;

  bool isAssigned() const;
};

}  // namespace tipsy::domain
