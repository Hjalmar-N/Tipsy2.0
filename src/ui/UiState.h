#pragma once

#include <Arduino.h>
#include <array>
#include <cstddef>

#include "app/models/MachineState.h"
#include "app/models/MachineStatus.h"
#include "config/AppConfig.h"
#include "ui/UiDrinkItem.h"
#include "ui/UiPumpAssignmentItem.h"

namespace tipsy::ui {

// Lightweight UI-facing snapshot that LVGL/SquareLine screens can render without app internals.
struct UiState {
  tipsy::app::MachineState machineState = tipsy::app::MachineState::Idle;
  tipsy::app::MachineStatusCode statusCode = tipsy::app::MachineStatusCode::Ok;
  String statusMessage;
  String selectedDrinkId;
  bool hasSelectedDrink = false;
  bool adminOpen = false;
  std::array<UiDrinkItem, tipsy::config::kMaxDrinkCount> drinks {};
  std::size_t drinkCount = 0;
  std::array<UiPumpAssignmentItem, tipsy::config::kPumpCount> pumpAssignments {};
};

}  // namespace tipsy::ui
