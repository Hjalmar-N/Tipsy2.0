#include "app/services/SettingsService.h"

#include <ArduinoJson.h>

#include "storage/StoragePaths.h"

namespace tipsy::app {

SettingsService::SettingsService(tipsy::storage::JsonStorage& jsonStorage)
    : jsonStorage_(jsonStorage) {}

bool SettingsService::load() {
  if (!ensureDefaultFiles()) {
    return false;
  }

  DynamicJsonDocument settingsDoc(3072);
  DynamicJsonDocument pumpMapDoc(2048);

  if (!jsonStorage_.readJson(tipsy::storage::paths::kSettings, settingsDoc)) {
    lastError_ = jsonStorage_.lastError();
    return false;
  }

  if (!jsonStorage_.readJson(tipsy::storage::paths::kPumpMap, pumpMapDoc)) {
    lastError_ = jsonStorage_.lastError();
    return false;
  }

  if (!loadSettingsDocument(settingsDoc.as<JsonObjectConst>())) {
    return false;
  }

  if (!loadPumpMap(pumpMapDoc["pumpAssignments"].as<JsonArrayConst>())) {
    return false;
  }

  lastError_ = String();
  return true;
}

bool SettingsService::save() const {
  if (!savePumpMap()) {
    return false;
  }

  return saveSettingsDocument();
}

const tipsy::domain::SystemSettings& SettingsService::current() const {
  return settings_;
}

tipsy::domain::SystemSettings& SettingsService::editable() {
  return settings_;
}

const tipsy::domain::PumpAssignment* SettingsService::findPumpAssignmentByIngredient(
    const String& ingredientId) const {
  for (const auto& assignment : settings_.pumpAssignments) {
    if (assignment.ingredientId == ingredientId) {
      return &assignment;
    }
  }

  return nullptr;
}

bool SettingsService::updatePumpAssignment(std::uint8_t pumpIndex, const String& ingredientId,
                                           const String& ingredientDisplayName, bool enabled) {
  if (pumpIndex >= settings_.pumpAssignments.size()) {
    lastError_ = "Invalid pump index.";
    return false;
  }

  auto& assignment = settings_.pumpAssignments[pumpIndex];
  assignment.pumpIndex = pumpIndex;
  assignment.ingredientId = ingredientId;
  assignment.ingredientDisplayName = ingredientDisplayName;
  assignment.enabled = enabled;
  return savePumpMap();
}

bool SettingsService::updatePumpCalibration(
    std::uint8_t pumpIndex, const tipsy::domain::PumpCalibration& calibration) {
  if (pumpIndex >= settings_.pumpCalibrations.size()) {
    lastError_ = "Invalid pump index.";
    return false;
  }

  settings_.pumpCalibrations[pumpIndex] = calibration;
  settings_.pumpCalibrations[pumpIndex].pumpIndex = pumpIndex;
  return saveSettingsDocument();
}

const String& SettingsService::lastError() const {
  return lastError_;
}

void SettingsService::loadDefaults() {
  settings_ = {};
  settings_.schemaVersion = 1;
  settings_.machineId = "tipsy2_s3";
  settings_.venueName = "Tipsy Demo Bar";
  settings_.mockModeEnabled = true;
  settings_.adminLockEnabled = false;
  settings_.manualPourMaxMl = 250.0F;

  settings_.pumpAssignments[0] = {0, true, "gin", "Gin"};
  settings_.pumpAssignments[1] = {1, true, "tonic", "Tonic Water"};
  settings_.pumpAssignments[2] = {2, true, "vodka", "Vodka"};
  settings_.pumpAssignments[3] = {3, true, "rum", "Rum"};
  settings_.pumpAssignments[4] = {4, true, "cola", "Cola"};
  settings_.pumpAssignments[5] = {5, true, "orange_juice", "Orange Juice"};

  for (std::uint8_t pumpIndex = 0; pumpIndex < settings_.pumpCalibrations.size(); ++pumpIndex) {
    settings_.pumpCalibrations[pumpIndex].pumpIndex = pumpIndex;
    settings_.pumpCalibrations[pumpIndex].enabled = true;
    settings_.pumpCalibrations[pumpIndex].mlPerSecond = 10.0F + pumpIndex;
    settings_.pumpCalibrations[pumpIndex].pwmDuty = 512;
    settings_.pumpCalibrations[pumpIndex].primeDurationMs = 1000;
    settings_.pumpCalibrations[pumpIndex].settleDurationMs = 250;
  }
}

bool SettingsService::ensureDefaultFiles() {
  loadDefaults();

  DynamicJsonDocument pumpMapDoc(2048);
  pumpMapDoc["schemaVersion"] = 1;
  JsonArray assignments = pumpMapDoc["pumpAssignments"].to<JsonArray>();
  for (const auto& assignment : settings_.pumpAssignments) {
    JsonObject entry = assignments.add<JsonObject>();
    entry["pumpIndex"] = assignment.pumpIndex;
    entry["enabled"] = assignment.enabled;
    entry["ingredientId"] = assignment.ingredientId;
    entry["ingredientDisplayName"] = assignment.ingredientDisplayName;
  }

  DynamicJsonDocument settingsDoc(3072);
  settingsDoc["schemaVersion"] = settings_.schemaVersion;
  settingsDoc["machineId"] = settings_.machineId;
  settingsDoc["venueName"] = settings_.venueName;
  settingsDoc["mockModeEnabled"] = settings_.mockModeEnabled;
  settingsDoc["adminLockEnabled"] = settings_.adminLockEnabled;
  settingsDoc["manualPourMaxMl"] = settings_.manualPourMaxMl;
  JsonArray calibrations = settingsDoc["pumpCalibrations"].to<JsonArray>();
  for (const auto& calibration : settings_.pumpCalibrations) {
    JsonObject entry = calibrations.add<JsonObject>();
    entry["pumpIndex"] = calibration.pumpIndex;
    entry["enabled"] = calibration.enabled;
    entry["mlPerSecond"] = calibration.mlPerSecond;
    entry["pwmDuty"] = calibration.pwmDuty;
    entry["primeDurationMs"] = calibration.primeDurationMs;
    entry["settleDurationMs"] = calibration.settleDurationMs;
  }

  if (!jsonStorage_.ensureFile(tipsy::storage::paths::kPumpMap, pumpMapDoc)) {
    lastError_ = jsonStorage_.lastError();
    return false;
  }

  if (!jsonStorage_.ensureFile(tipsy::storage::paths::kSettings, settingsDoc)) {
    lastError_ = jsonStorage_.lastError();
    return false;
  }

  lastError_ = String();
  return true;
}

bool SettingsService::loadPumpMap(const JsonArrayConst& assignments) {
  for (auto& assignment : settings_.pumpAssignments) {
    assignment = {};
  }

  for (JsonObjectConst assignmentJson : assignments) {
    const std::uint8_t pumpIndex = assignmentJson["pumpIndex"] | 255;
    if (pumpIndex >= settings_.pumpAssignments.size()) {
      lastError_ = "Pump map contains invalid pump index.";
      return false;
    }

    auto& assignment = settings_.pumpAssignments[pumpIndex];
    assignment.pumpIndex = pumpIndex;
    assignment.enabled = assignmentJson["enabled"] | true;
    assignment.ingredientId = assignmentJson["ingredientId"] | "";
    assignment.ingredientDisplayName = assignmentJson["ingredientDisplayName"] | "";
  }

  return true;
}

bool SettingsService::loadSettingsDocument(const JsonObjectConst& settingsObject) {
  settings_.schemaVersion = settingsObject["schemaVersion"] | 1;
  settings_.machineId = settingsObject["machineId"] | "tipsy2_s3";
  settings_.venueName = settingsObject["venueName"] | "Tipsy Demo Bar";
  settings_.mockModeEnabled = settingsObject["mockModeEnabled"] | true;
  settings_.adminLockEnabled = settingsObject["adminLockEnabled"] | false;
  settings_.manualPourMaxMl = settingsObject["manualPourMaxMl"] | 250.0F;

  for (auto& calibration : settings_.pumpCalibrations) {
    calibration = {};
  }

  JsonArrayConst calibrations = settingsObject["pumpCalibrations"].as<JsonArrayConst>();
  for (JsonObjectConst calibrationJson : calibrations) {
    const std::uint8_t pumpIndex = calibrationJson["pumpIndex"] | 255;
    if (pumpIndex >= settings_.pumpCalibrations.size()) {
      lastError_ = "Settings contain invalid calibration index.";
      return false;
    }

    auto& calibration = settings_.pumpCalibrations[pumpIndex];
    calibration.pumpIndex = pumpIndex;
    calibration.enabled = calibrationJson["enabled"] | true;
    calibration.mlPerSecond = calibrationJson["mlPerSecond"] | 10.0F;
    calibration.pwmDuty = calibrationJson["pwmDuty"] | 512;
    calibration.primeDurationMs = calibrationJson["primeDurationMs"] | 1000;
    calibration.settleDurationMs = calibrationJson["settleDurationMs"] | 250;
  }

  return true;
}

bool SettingsService::savePumpMap() const {
  DynamicJsonDocument doc(2048);
  doc["schemaVersion"] = settings_.schemaVersion;
  JsonArray assignments = doc["pumpAssignments"].to<JsonArray>();

  for (const auto& assignment : settings_.pumpAssignments) {
    JsonObject entry = assignments.add<JsonObject>();
    entry["pumpIndex"] = assignment.pumpIndex;
    entry["enabled"] = assignment.enabled;
    entry["ingredientId"] = assignment.ingredientId;
    entry["ingredientDisplayName"] = assignment.ingredientDisplayName;
  }

  const bool ok = jsonStorage_.writeJson(tipsy::storage::paths::kPumpMap, doc);
  lastError_ = ok ? String() : jsonStorage_.lastError();
  return ok;
}

bool SettingsService::saveSettingsDocument() const {
  DynamicJsonDocument doc(3072);
  doc["schemaVersion"] = settings_.schemaVersion;
  doc["machineId"] = settings_.machineId;
  doc["venueName"] = settings_.venueName;
  doc["mockModeEnabled"] = settings_.mockModeEnabled;
  doc["adminLockEnabled"] = settings_.adminLockEnabled;
  doc["manualPourMaxMl"] = settings_.manualPourMaxMl;

  JsonArray calibrations = doc["pumpCalibrations"].to<JsonArray>();
  for (const auto& calibration : settings_.pumpCalibrations) {
    JsonObject entry = calibrations.add<JsonObject>();
    entry["pumpIndex"] = calibration.pumpIndex;
    entry["enabled"] = calibration.enabled;
    entry["mlPerSecond"] = calibration.mlPerSecond;
    entry["pwmDuty"] = calibration.pwmDuty;
    entry["primeDurationMs"] = calibration.primeDurationMs;
    entry["settleDurationMs"] = calibration.settleDurationMs;
  }

  const bool ok = jsonStorage_.writeJson(tipsy::storage::paths::kSettings, doc);
  lastError_ = ok ? String() : jsonStorage_.lastError();
  return ok;
}

}  // namespace tipsy::app
