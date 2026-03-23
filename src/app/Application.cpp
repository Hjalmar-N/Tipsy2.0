#include "app/Application.h"

namespace tipsy::app {

Application::Application(tipsy::storage::StorageManager& storageManager,
                         RecipeService& recipeService,
                         IngredientService& ingredientService,
                         SettingsService& settingsService,
                         MachineController& machineController,
                         tipsy::pumps::PumpController& pumpController,
                         tipsy::ui::UiManager& uiManager)
    : storageManager_(storageManager),
      recipeService_(recipeService),
      ingredientService_(ingredientService),
      settingsService_(settingsService),
      machineController_(machineController),
      pumpController_(pumpController),
      uiManager_(uiManager) {}

bool Application::begin() {
  if (!storageManager_.begin()) {
    lastError_ = storageManager_.lastError();
    return false;
  }

  if (!settingsService_.load()) {
    lastError_ = settingsService_.lastError();
    return false;
  }

  if (!ingredientService_.load()) {
    lastError_ = ingredientService_.lastError();
    return false;
  }

  if (!recipeService_.load()) {
    lastError_ = recipeService_.lastError();
    return false;
  }

  machineController_.begin();

  const bool ok = uiManager_.begin();
  lastError_ = ok ? String() : String("UI initialization failed.");
  return ok;
}

void Application::update() {
  uiManager_.update();
  machineController_.update();
  pumpController_.update();
}

const String& Application::lastError() const {
  return lastError_;
}

}  // namespace tipsy::app
