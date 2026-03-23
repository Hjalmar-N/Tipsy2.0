#pragma once

#include <cstdint>

#include "domain/models/SystemSettings.h"
#include "storage/JsonStorage.h"

namespace tipsy::app {

// Storage-backed machine settings service for pump mapping and calibration data.
class SettingsService {
 public:
  explicit SettingsService(tipsy::storage::JsonStorage& jsonStorage);

  bool load();
  bool save() const;
  const tipsy::domain::SystemSettings& current() const;
  tipsy::domain::SystemSettings& editable();
  const tipsy::domain::PumpAssignment* findPumpAssignmentByIngredient(
      const String& ingredientId) const;
  bool updatePumpAssignment(std::uint8_t pumpIndex, const String& ingredientId,
                            const String& ingredientDisplayName, bool enabled);
  bool updatePumpCalibration(std::uint8_t pumpIndex,
                             const tipsy::domain::PumpCalibration& calibration);
  const String& lastError() const;

 private:
  void loadDefaults();
  bool ensureDefaultFiles();
  bool loadPumpMap(const JsonArrayConst& assignments);
  bool loadSettingsDocument(const JsonObjectConst& settingsObject);
  bool savePumpMap() const;
  bool saveSettingsDocument() const;

  tipsy::storage::JsonStorage& jsonStorage_;
  tipsy::domain::SystemSettings settings_ {};
  mutable String lastError_;
};

}  // namespace tipsy::app

