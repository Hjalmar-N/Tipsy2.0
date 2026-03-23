#pragma once

#include <Arduino.h>

#include "app/models/MachineState.h"
#include "app/models/MachineStatus.h"

namespace tipsy::ui {

// Lightweight UI-facing snapshot that LVGL/SquareLine screens can render without app internals.
struct UiState {
  tipsy::app::MachineState machineState = tipsy::app::MachineState::Idle;
  tipsy::app::MachineStatusCode statusCode = tipsy::app::MachineStatusCode::Ok;
  String statusMessage;
  String selectedDrinkId;
  bool adminOpen = false;
};

}  // namespace tipsy::ui

