#pragma once

#include <Arduino.h>
#include <cstdint>

namespace tipsy::ui {

// Small read-model for rendering pump mapping rows without direct settings access.
struct UiPumpAssignmentItem {
  std::uint8_t pumpIndex = 0;
  String ingredientId;
  String ingredientDisplayName;
  bool enabled = true;
};

}  // namespace tipsy::ui
