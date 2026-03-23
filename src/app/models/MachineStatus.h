#pragma once

#include <Arduino.h>
#include <cstdint>

#include "app/models/MachineState.h"

namespace tipsy::app {

enum class MachineStatusCode : std::uint8_t {
  Ok = 0,
  Busy,
  InvalidState,
  DrinkNotFound,
  DrinkDisabled,
  IngredientNotFound,
  IngredientDisabled,
  IngredientNotMapped,
  PumpUnavailable,
  VolumeOutOfRange,
  SettingsSaveFailed,
  MaintenanceActive,
  PourStartFailed,
};

// Lightweight status/result payload intended for later UI polling or callbacks.
struct MachineStatus {
  bool success = true;
  MachineStatusCode code = MachineStatusCode::Ok;
  MachineState state = MachineState::Idle;
  String message;

  static MachineStatus ok(MachineState state, const String& message = String());
  static MachineStatus error(MachineStatusCode code, MachineState state, const String& message);
};

}  // namespace tipsy::app
