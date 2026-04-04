#include "app/MachineController.h"

namespace tipsy::app {

namespace {

constexpr float kPrimeVolumeMl = 20.0F;
constexpr float kFlushVolumeMl = 80.0F;
constexpr std::uint8_t kServiceSpeedPercent = 100;
constexpr std::uint8_t kUnassignedPumpIndex = 255;

}  // namespace

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
  activeAction_ = MachineAction::None;
  lastStatus_ = MachineStatus::ok(state_, "Machine ready.");
}

void MachineController::update() {
  if (state_ == MachineState::Pouring && pumpController_.activeTaskCount() == 0) {
    String message = "Pour complete.";
    switch (activeAction_) {
      case MachineAction::PrimePumps:
        message = "Pump priming complete.";
        break;
      case MachineAction::FlushCleaning:
        message = "Flush / Cleaning complete.";
        break;
      case MachineAction::ManualPour:
        message = "Manual pour complete.";
        break;
      case MachineAction::DrinkPour:
      case MachineAction::None:
      default:
        break;
    }
    lastStatus_ = MachineStatus::ok(MachineState::Complete, message);
    state_ = MachineState::Complete;
    activeAction_ = MachineAction::None;
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

MachineStatus MachineController::startSelectedDrink(std::uint16_t alcoholOverrideMl,
                                                    std::uint8_t speedPercent) {
  if (selectedDrinkId_.isEmpty()) {
    return fail(MachineStatusCode::NoDrinkSelected, "Select a drink before starting a pour.");
  }

  return startDrink(selectedDrinkId_, alcoholOverrideMl, speedPercent);
}

MachineStatus MachineController::startDrink(const String& drinkId,
                                           std::uint16_t alcoholOverrideMl,
                                           std::uint8_t speedPercent) {
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

  if (alcoholOverrideMl > 0 && !recipeHasAlcoholOverrideTarget(*recipe)) {
    return fail(MachineStatusCode::InvalidState,
                "Selected recipe does not support alcohol strength override.");
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

    const float amountMl = resolveIngredientAmountMl(*recipe, i, alcoholOverrideMl);

    const bool started =
        pumpController_.startPour(pumpIndex, amountMl, speedPercent, settingsService_.current());
    if (!started) {
      pumpController_.stopAll();
      return setState(MachineState::Error,
                      MachineStatus::error(MachineStatusCode::PourStartFailed,
                                           MachineState::Error,
                                           "Unable to start drink pour."));
    }
  }

  selectedDrinkId_ = drinkId;
  activeAction_ = MachineAction::DrinkPour;
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

  activeAction_ = MachineAction::ManualPour;
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
  if (state_ == MachineState::Pouring || state_ == MachineState::Maintenance) {
    return MachineStatus::error(MachineStatusCode::InvalidState, state_,
                                "Pump assignments cannot be edited while machine is busy.");
  }

  // Intercept special cycle command from UI to ensure backend owns ingredient options.
  if (ingredientId == "__CYCLE__") {
    const auto* assignment = settingsService_.current().findAssignment(pumpIndex);
    if (assignment == nullptr) {
      return fail(MachineStatusCode::InvalidState, "Pump not found.");
    }

    const String& currentId = assignment->ingredientId;
    const auto& ingredients = ingredientService_.all();
    const std::size_t searchCount = ingredientService_.count();
    std::size_t nextIndex = searchCount; // Default to 'Unassigned' if not found

    for (std::size_t i = 0; i < searchCount; ++i) {
      if (ingredients[i].id == currentId) {
        nextIndex = (i + 1 < searchCount) ? (i + 1) : searchCount;
        break;
      }
    }
    
    // Jump from unassigned to first ingredient available
    if (currentId.isEmpty() && searchCount > 0) {
      nextIndex = 0;
    }

    if (nextIndex == searchCount) {
      return updatePumpAssignment(pumpIndex, "", "", false);
    } else {
      return updatePumpAssignment(pumpIndex, ingredients[nextIndex].id,
                                  ingredients[nextIndex].displayName, true);
    }
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

  const MachineState targetState =
      state_ == MachineState::AdminSettings ? MachineState::AdminSettings : state_;
  return setState(targetState, MachineStatus::ok(targetState, "Pump assignment updated."));
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

MachineStatus MachineController::startPrimePumps() {
  if (!canStartPouring()) {
    return busyStatus();
  }

  if (state_ == MachineState::Maintenance) {
    return fail(MachineStatusCode::MaintenanceActive,
                "Machine is in maintenance mode.");
  }

  std::uint8_t startedCount = 0;
  const auto& settings = settingsService_.current();
  for (std::uint8_t pumpIndex = 0; pumpIndex < settings.pumpAssignments.size(); ++pumpIndex) {
    const auto* assignment = settings.findAssignment(pumpIndex);
    if (assignment == nullptr || !assignment->enabled || !assignment->isAssigned()) {
      continue;
    }

    if (!pumpController_.startPour(pumpIndex, kPrimeVolumeMl, kServiceSpeedPercent, settings)) {
      pumpController_.stopAll();
      return setState(MachineState::Error,
                      MachineStatus::error(MachineStatusCode::PourStartFailed,
                                           MachineState::Error,
                                           "Unable to start pump priming."));
    }
    ++startedCount;
  }

  if (startedCount == 0) {
    return fail(MachineStatusCode::IngredientNotMapped,
                "No mapped pumps available to prime.");
  }

  activeAction_ = MachineAction::PrimePumps;
  return setState(MachineState::Pouring,
                  MachineStatus::ok(MachineState::Pouring, "Priming pumps..."));
}

MachineStatus MachineController::startFlushCleaning() {
  if (!canStartPouring()) {
    return busyStatus();
  }

  if (state_ == MachineState::Maintenance) {
    return fail(MachineStatusCode::MaintenanceActive,
                "Machine is in maintenance mode.");
  }

  std::uint8_t startedCount = 0;
  const auto& settings = settingsService_.current();
  for (std::uint8_t pumpIndex = 0; pumpIndex < settings.pumpAssignments.size(); ++pumpIndex) {
    const auto* assignment = settings.findAssignment(pumpIndex);
    if (assignment == nullptr || !assignment->enabled || !assignment->isAssigned()) {
      continue;
    }

    if (!pumpController_.startPour(pumpIndex, kFlushVolumeMl, kServiceSpeedPercent, settings)) {
      pumpController_.stopAll();
      return setState(MachineState::Error,
                      MachineStatus::error(MachineStatusCode::PourStartFailed,
                                           MachineState::Error,
                                           "Unable to start flush / cleaning."));
    }
    ++startedCount;
  }

  if (startedCount == 0) {
    return fail(MachineStatusCode::IngredientNotMapped,
                "No mapped pumps available for flush / cleaning.");
  }

  activeAction_ = MachineAction::FlushCleaning;
  return setState(MachineState::Pouring,
                  MachineStatus::ok(MachineState::Pouring, "Flush / Cleaning started."));
}

MachineStatus MachineController::enterMaintenanceMode() {
  if (state_ == MachineState::Pouring) {
    pumpController_.stopAll();
  }

  activeAction_ = MachineAction::None;
  return setState(MachineState::Maintenance,
                  MachineStatus::ok(MachineState::Maintenance, "Maintenance mode enabled."));
}

MachineStatus MachineController::exitMaintenanceMode() {
  return setState(MachineState::Idle,
                  MachineStatus::ok(MachineState::Idle, "Maintenance mode disabled."));
}

MachineStatus MachineController::acknowledgeComplete() {
  activeAction_ = MachineAction::None;
  return setState(MachineState::Idle,
                  MachineStatus::ok(MachineState::Idle, "Machine ready for next order."));
}

MachineStatus MachineController::stopAll() {
  pumpController_.stopAll();
  selectedDrinkId_ = String();
  activeAction_ = MachineAction::None;
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

MachinePourPlan MachineController::buildPourPlan(const String& drinkId,
                                                 std::uint16_t alcoholOverrideMl,
                                                 std::uint8_t speedPercent) const {
  MachinePourPlan plan {};
  plan.drinkId = drinkId;
  plan.status = MachineStatus::ok(state_);

  const auto* recipe = recipeService_.findById(drinkId);
  if (recipe == nullptr) {
    plan.status = MachineStatus::error(MachineStatusCode::DrinkNotFound, state_,
                                       "Drink was not found.");
    return plan;
  }

  plan.drinkName = recipe->displayName;
  plan.categoryId = recipe->categoryId;
  plan.isShot = recipe->ingredientCount == 1;

  if (!recipe->enabled) {
    plan.status = MachineStatus::error(MachineStatusCode::DrinkDisabled, state_,
                                       "Drink is disabled.");
    return plan;
  }

  if (alcoholOverrideMl > 0 && !recipeHasAlcoholOverrideTarget(*recipe)) {
    plan.status = MachineStatus::error(MachineStatusCode::InvalidState, state_,
                                       "Recipe does not support alcohol override.");
    return plan;
  }

  bool available = true;
  bool wouldRoute = speedPercent <= 100 && canStartPouring() && state_ != MachineState::Maintenance;
  std::uint32_t maxDurationMs = 0;

  for (std::size_t i = 0; i < recipe->ingredientCount && i < plan.ingredients.size(); ++i) {
    MachinePourIngredient& item = plan.ingredients[plan.ingredientCount];
    const auto& recipeIngredient = recipe->ingredients[i];
    item.ingredientId = recipeIngredient.ingredientId;
    item.amountMl = resolveIngredientAmountMl(*recipe, i, alcoholOverrideMl);
    item.isAlcohol = ingredientUsesAlcoholOverride(*recipe, i);
    item.pumpIndex = kUnassignedPumpIndex;

    const auto* ingredient = ingredientService_.findById(recipeIngredient.ingredientId);
    if (ingredient != nullptr) {
      item.ingredientDisplayName = ingredient->displayName;
      if (!ingredient->enabled) {
        available = false;
        wouldRoute = false;
        if (plan.status.success) {
          plan.status = MachineStatus::error(MachineStatusCode::IngredientDisabled, state_,
                                             "A required ingredient is disabled.");
        }
      }
    } else {
      item.ingredientDisplayName = recipeIngredient.ingredientId;
      available = false;
      wouldRoute = false;
      if (plan.status.success) {
        plan.status = MachineStatus::error(MachineStatusCode::IngredientNotFound, state_,
                                           "A required ingredient was not found.");
      }
    }

    const auto* assignment = settingsService_.findPumpAssignmentByIngredient(recipeIngredient.ingredientId);
    if (assignment != nullptr && assignment->enabled && assignment->isAssigned()) {
      item.mapped = true;
      item.pumpIndex = assignment->pumpIndex;

      const auto* calibration = settingsService_.current().findCalibration(assignment->pumpIndex);
      if (calibration != nullptr && calibration->enabled && calibration->isValid()) {
        const std::uint32_t durationMs = calibration->estimatePourTimeMs(item.amountMl);
        if (durationMs > maxDurationMs) {
          maxDurationMs = durationMs;
        }
      } else {
        available = false;
        wouldRoute = false;
        if (plan.status.success) {
          plan.status = MachineStatus::error(MachineStatusCode::PumpUnavailable, state_,
                                             "A mapped pump calibration is invalid.");
        }
      }

      if (pumpController_.isPumpActive(assignment->pumpIndex)) {
        wouldRoute = false;
        if (plan.status.success) {
          plan.status = MachineStatus::error(MachineStatusCode::PumpUnavailable, state_,
                                             "A required pump is already active.");
        }
      }
    } else {
      available = false;
      wouldRoute = false;
      if (plan.status.success) {
        plan.status = MachineStatus::error(MachineStatusCode::IngredientNotMapped, state_,
                                           "A required ingredient is not mapped.");
      }
    }

    if (item.isAlcohol && plan.alcoholAmountMl == 0) {
      plan.alcoholAmountMl = static_cast<std::uint16_t>(item.amountMl);
    }

    ++plan.ingredientCount;
  }

  if (plan.alcoholAmountMl == 0 && plan.ingredientCount > 0) {
    plan.alcoholAmountMl = static_cast<std::uint16_t>(plan.ingredients[0].amountMl);
  }

  plan.available = available;
  plan.wouldRoute = available && wouldRoute;
  if (plan.status.success && !plan.available) {
    plan.status = MachineStatus::error(MachineStatusCode::DrinkUnavailable, state_,
                                       "Drink is unavailable with the current mapping.");
  }

  if (maxDurationMs > 0) {
    plan.estimatedPourTimeSec =
        static_cast<std::uint16_t>((maxDurationMs + 999U) / 1000U);
  }

  return plan;
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

bool MachineController::recipeHasAlcoholOverrideTarget(const tipsy::domain::DrinkRecipe& recipe) const {
  if (recipe.ingredientCount == 0) {
    return false;
  }

  if (recipe.ingredientCount == 1) {
    return true;
  }

  for (std::size_t i = 0; i < recipe.ingredientCount; ++i) {
    if (recipe.ingredients[i].isAlcohol) {
      return true;
    }
  }

  return true;
}

bool MachineController::ingredientUsesAlcoholOverride(const tipsy::domain::DrinkRecipe& recipe,
                                                      std::size_t ingredientIndex) const {
  if (ingredientIndex >= recipe.ingredientCount) {
    return false;
  }

  if (recipe.ingredientCount == 1) {
    return true;
  }

  const bool hasExplicitAlcoholTarget = recipeHasAlcoholOverrideTarget(recipe);
  if (hasExplicitAlcoholTarget) {
    return recipe.ingredients[ingredientIndex].isAlcohol;
  }

  return ingredientIndex == 0;
}

float MachineController::resolveIngredientAmountMl(const tipsy::domain::DrinkRecipe& recipe,
                                                   std::size_t ingredientIndex,
                                                   std::uint16_t alcoholOverrideMl) const {
  if (ingredientIndex >= recipe.ingredientCount) {
    return 0.0F;
  }

  if (alcoholOverrideMl > 0 && ingredientUsesAlcoholOverride(recipe, ingredientIndex)) {
    return static_cast<float>(alcoholOverrideMl);
  }

  return recipe.ingredients[ingredientIndex].amountMl;
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
