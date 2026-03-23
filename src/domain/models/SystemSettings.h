#pragma once

#include <Arduino.h>
#include <array>
#include <cstdint>

#include "config/AppConfig.h"
#include "domain/models/PumpAssignment.h"
#include "domain/models/PumpCalibration.h"

namespace tipsy::domain {

// Persistent admin and machine settings stored in JSON.
struct SystemSettings {
  std::uint16_t schemaVersion = 1;
  String machineId = "tipsy2_s3";
  String venueName = "Tipsy Demo Bar";
  bool mockModeEnabled = true;
  bool adminLockEnabled = false;
  float manualPourMaxMl = 250.0F;
  std::array<PumpAssignment, config::kPumpCount> pumpAssignments {};
  std::array<PumpCalibration, config::kPumpCount> pumpCalibrations {};

  const PumpAssignment* findAssignment(std::uint8_t pumpIndex) const;
  const PumpCalibration* findCalibration(std::uint8_t pumpIndex) const;
};

}  // namespace tipsy::domain
