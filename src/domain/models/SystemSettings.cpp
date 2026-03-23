#include "domain/models/SystemSettings.h"

namespace tipsy::domain {

const PumpAssignment* SystemSettings::findAssignment(std::uint8_t pumpIndex) const {
  if (pumpIndex >= pumpAssignments.size()) {
    return nullptr;
  }

  return &pumpAssignments[pumpIndex];
}

const PumpCalibration* SystemSettings::findCalibration(std::uint8_t pumpIndex) const {
  if (pumpIndex >= pumpCalibrations.size()) {
    return nullptr;
  }

  return &pumpCalibrations[pumpIndex];
}

}  // namespace tipsy::domain
