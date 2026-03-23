#include "app/MachineController.h"

namespace tipsy::app {

MachineController::MachineController(RecipeService& recipeService,
                                     IngredientService& ingredientService,
                                     SettingsService& settingsService,
                                     tipsy::pumps::PumpController& pumpController)
    : recipeService_(recipeService),
      ingredientService_(ingredientService),
      settingsService_(settingsService),
      pumpController_(pumpController) {}

void MachineController::begin() {
  state_ = MachineState::Idle;
  lastStatus_ = MachineStatus::ok(state_, "Machine ready.");
}

void MachineController::update() {
  if (state_ == MachineState::Pouring && pumpController_.activeTaskCount() == 0) {
    lastStatus_ = MachineStatus::ok(MachineState::Complete, "Pour complete.");
    state_ = MachineState::Complete;
  }
}

MachineStatus MachineController::selectDrink(const String& drinkId) {
  const auto* recipe = recipeService_.findById(drinkId);
  if (recipe == nullptr) {
    return fail(MachineStatusCode::DrinkNotFound, "Selected drink was not found.");
  }

  if (!recipe->enabled) {
    return fail(MachineStatusCode::DrinkDisabled, "Selected drink is disabled.");
  }

  for (std::size_t i = 0; i < recipe->ingredientCount; ++i) {
    MachineStatus validationStatus;
    if (!isIngredientConfiguredForDrink(recipe->ingredients[i].ingredientId, validationStatus)) {
      return fail(MachineStatusCode::DrinkUnavailable,
                  "Selected drink is currently unavailable.");
    }
  }

  selectedDrinkId_ = drinkId;
  return setState(MachineState::DrinkSelection,
                  MachineStatus::ok(MachineState::DrinkSelection, "Drink selected."));
}

MachineStatus MachineController::startDrink(const String& drinkId, std::uint8_t speedPercent) {
  if (!canStartPouring()) {
    return busyStatus();
  }

  if (state_ == MachineState::Maintenance) {
    return fail(MachineStatusCode::MaintenanceActive,
                "Machine is in maintenance mode.");
  }

  const auto* recipe = recipeService_.findById(drinkId);
  if (recipe == nullptr) {
    return fail(MachineStatusCode::DrinkNotFound, "Drink was not found.");
  }

  if (!recipe->enabled) {
    return fail(MachineStatusCode::DrinkDisabled, "Drink is disabled.");
  }

  // For this machine generation we intentionally pour recipe ingredients simultaneously.
  // The pump layer already supports multiple timed tasks, and simultaneous pours reduce
  // total serve time. If recipe sequencing becomes necessary later, this method is the
  // single place to switch to a queued step-by-step strategy.
  for (std::size_t i = 0; i < recipe->ingredientCount; ++i) {
    MachineStatus validationStatus;
    if (!isIngredientAvailable(recipe->ingredients[i].ingredientId, validationStatus)) {
      return setState(MachineState::Error, validationStatus);
    }

    std::uint8_t pumpIndex = 0;
    if (!resolvePumpForIngredient(recipe->ingredients[i].ingredientId, pumpIndex, validationStatus)) {
      return setState(MachineState::Error, validationStatus);
    }
  }

  for (std::size_t i = 0; i < recipe->ingredientCount; ++i) {
    std::uint8_t pumpIndex = 0;
    MachineStatus validationStatus;
    resolvePumpForIngredient(recipe->ingredients[i].ingredientId, pumpIndex, validationStatus);

    const bool started = pumpController_.startPour(pumpIndex, recipe->ingredients[i].amountMl,
                                                   speedPercent, settingsService_.current());
    if (!started) {
      pumpController_.stopAll();
      return setState(MachineState::Error,
                      MachineStatus::error(MachineStatusCode::PourStartFailed,
                                           MachineState::Error,
                                           "Unable to start drink pour."));
    }
  }

  selectedDrinkId_ = drinkId;
  return setState(MachineState::Pouring,
                  MachineStatus::ok(MachineState::Pouring, "Drink pour started."));
}

MachineStatus MachineController::enterManualPourMode() {
  if (state_ == MachineState::Pouring) {
    return busyStatus();
  }

  return setState(MachineState::ManualPour,
                  MachineStatus::ok(MachineState::ManualPour, "Manual pour mode ready."));
}

MachineStatus MachineController::startManualPour(const String& ingredientId, float volumeMl,
                                                 std::uint8_t speedPercent) {
  if (!canStartPouring()) {
    return busyStatus();
  }

  if (state_ == MachineState::Maintenance) {
    return fail(MachineStatusCode::MaintenanceActive,
                "Machine is in maintenance mode.");
  }

  if (volumeMl <= 0.0F || volumeMl > settingsService_.current().manualPourMaxMl) {
    return fail(MachineStatusCode::VolumeOutOfRange,
                "Manual pour volume is out of range.");
  }

  MachineStatus validationStatus;
  if (!isIngredientAvailable(ingredientId, validationStatus)) {
    return setState(MachineState::Error, validationStatus);
  }

  std::uint8_t pumpIndex = 0;
  if (!resolvePumpForIngredient(ingredientId, pumpIndex, validationStatus)) {
    return setState(MachineState::Error, validationStatus);
  }

  if (!pumpController_.startPour(pumpIndex, volumeMl, speedPercent, settingsService_.current())) {
    return setState(MachineState::Error,
                    MachineStatus::error(MachineStatusCode::PourStartFailed,
                                         MachineState::Error,
                                         "Unable to start manual pour."));
  }

  return setState(MachineState::Pouring,
                  MachineStatus::ok(MachineState::Pouring, "Manual pour started."));
}

MachineStatus MachineController::enterAdminSettings() {
  if (state_ == MachineState::Pouring) {
    return busyStatus();
  }

  return setState(MachineState::AdminSettings,
                  MachineStatus::ok(MachineState::AdminSettings, "Admin settings open."));
}

MachineStatus MachineController::updatePumpAssignment(std::uint8_t pumpIndex,
                                                      const String& ingredientId,
                                                      const String& ingredientDisplayName,
                                                      bool enabled) {
  if (state_ != MachineState::AdminSettings) {
    return MachineStatus::error(MachineStatusCode::InvalidState, state_,
                                "Pump assignments can only be edited in admin settings.");
  }

  String resolvedDisplayName = ingredientDisplayName;
  if (!ingredientId.isEmpty()) {
    const auto* ingredient = ingredientService_.findById(ingredientId);
    if (ingredient == nullptr) {
      return setState(MachineState::Error,
                      MachineStatus::error(MachineStatusCode::IngredientNotFound,
                                           MachineState::Error,
                                           "Cannot assign an unknown ingredient to a pump."));
    }

    if (resolvedDisplayName.isEmpty()) {
      resolvedDisplayName = ingredient->displayName;
    }
  }

  if (!settingsService_.updatePumpAssignment(pumpIndex, ingredientId, resolvedDisplayName,
                                             enabled)) {
    return setState(MachineState::Error,
                    MachineStatus::error(MachineStatusCode::SettingsSaveFailed,
                                         MachineState::Error,
                                         "Failed to update pump assignment."));
  }

  return setState(MachineState::AdminSettings,
                  MachineStatus::ok(MachineState::AdminSettings,
                                    "Pump assignment updated."));
}

MachineStatus MachineController::saveSettings() {
  if (!settingsService_.save()) {
    return setState(MachineState::Error,
                    MachineStatus::error(MachineStatusCode::SettingsSaveFailed,
                                         MachineState::Error,
                                         "Failed to save settings."));
  }

  return setState(MachineState::AdminSettings,
                  MachineStatus::ok(MachineState::AdminSettings, "Settings saved."));
}

MachineStatus MachineController::enterMaintenanceMode() {
  if (state_ == MachineState::Pouring) {
    pumpController_.stopAll();
  }

  return setState(MachineState::Maintenance,
                  MachineStatus::ok(MachineState::Maintenance, "Maintenance mode enabled."));
}

MachineStatus MachineController::exitMaintenanceMode() {
  return setState(MachineState::Idle,
                  MachineStatus::ok(MachineState::Idle, "Maintenance mode disabled."));
}

MachineStatus MachineController::acknowledgeComplete() {
  return setState(MachineState::Idle,
                  MachineStatus::ok(MachineState::Idle, "Machine ready for next order."));
}

MachineStatus MachineController::stopAll() {
  pumpController_.stopAll();
  selectedDrinkId_ = String();
  return setState(MachineState::Idle,
                  MachineStatus::ok(MachineState::Idle, "All pours stopped."));
}

bool MachineController::isDrinkAvailable(const String& drinkId) const {
  const auto* recipe = recipeService_.findById(drinkId);
  if (recipe == nullptr || !recipe->enabled) {
    return false;
  }

  for (std::size_t i = 0; i < recipe->ingredientCount; ++i) {
    MachineStatus status;
    if (!isIngredientConfiguredForDrink(recipe->ingredients[i].ingredientId, status)) {
      return false;
    }
  }

  return true;
}

MachineState MachineController::getCurrentState() const {
  return state_;
}

const MachineStatus& MachineController::getLastStatus() const {
  return lastStatus_;
}

const String& MachineController::getSelectedDrinkId() const {
  return selectedDrinkId_;
}

MachineStatus MachineController::setState(MachineState state, const MachineStatus& status) {
  state_ = state;
  lastStatus_ = status;
  return lastStatus_;
}

MachineStatus MachineController::fail(MachineStatusCode code, const String& message) {
  return setState(MachineState::Error, MachineStatus::error(code, MachineState::Error, message));
}

MachineStatus MachineController::busyStatus() const {
  return MachineStatus::error(MachineStatusCode::Busy, state_,
                              "Machine is busy with another action.");
}

bool MachineController::canStartPouring() const {
  return state_ == MachineState::Idle || state_ == MachineState::DrinkSelection ||
         state_ == MachineState::ManualPour || state_ == MachineState::Complete ||
         state_ == MachineState::Error;
}

bool MachineController::isIngredientAvailable(const String& ingredientId,
                                              MachineStatus& status) const {
  const auto* ingredient = ingredientService_.findById(ingredientId);
  if (ingredient == nullptr) {
    status = MachineStatus::error(MachineStatusCode::IngredientNotFound,
                                  MachineState::Error,
                                  "Ingredient was not found.");
    return false;
  }

  if (!ingredient->enabled) {
    status = MachineStatus::error(MachineStatusCode::IngredientDisabled,
                                  MachineState::Error,
                                  "Ingredient is disabled.");
    return false;
  }

  status = MachineStatus::ok(state_);
  return true;
}

bool MachineController::isIngredientConfiguredForDrink(const String& ingredientId,
                                                       MachineStatus& status) const {
  if (!isIngredientAvailable(ingredientId, status)) {
    return false;
  }

  const auto* assignment = settingsService_.findPumpAssignmentByIngredient(ingredientId);
  if (assignment == nullptr || !assignment->enabled || !assignment->isAssigned()) {
    status = MachineStatus::error(MachineStatusCode::DrinkUnavailable,
                                  MachineState::Idle,
                                  "Drink ingredient is not mapped to an enabled pump.");
    return false;
  }

  status = MachineStatus::ok(state_);
  return true;
}

bool MachineController::resolvePumpForIngredient(const String& ingredientId, std::uint8_t& pumpIndex,
                                                 MachineStatus& status) const {
  const auto* assignment = settingsService_.findPumpAssignmentByIngredient(ingredientId);
  if (assignment == nullptr || !assignment->enabled || !assignment->isAssigned()) {
    status = MachineStatus::error(MachineStatusCode::IngredientNotMapped,
                                  MachineState::Error,
                                  "Ingredient is not mapped to an enabled pump.");
    return false;
  }

  pumpIndex = assignment->pumpIndex;
  if (pumpController_.isPumpActive(pumpIndex)) {
    status = MachineStatus::error(MachineStatusCode::PumpUnavailable,
                                  MachineState::Error,
                                  "Requested pump is already active.");
    return false;
  }

  status = MachineStatus::ok(state_);
  return true;
}

}  // namespace tipsy::app
