#pragma once

#include <Arduino.h>

#include "app/MachineController.h"
#include "app/services/IngredientService.h"
#include "app/services/RecipeService.h"
#include "app/services/SettingsService.h"
#include "pumps/PumpController.h"
#include "storage/StorageManager.h"
#include "ui/UiManager.h"

namespace tipsy::app {

// Top-level coordinator that wires modules and runs the non-blocking main loop.
class Application {
 public:
  Application(tipsy::storage::StorageManager& storageManager,
              RecipeService& recipeService,
              IngredientService& ingredientService,
              SettingsService& settingsService,
              MachineController& machineController,
              tipsy::pumps::PumpController& pumpController,
              tipsy::ui::UiManager& uiManager);

  bool begin();
  void update();
  const String& lastError() const;

 private:
  tipsy::storage::StorageManager& storageManager_;
  RecipeService& recipeService_;
  IngredientService& ingredientService_;
  SettingsService& settingsService_;
  MachineController& machineController_;
  tipsy::pumps::PumpController& pumpController_;
  tipsy::ui::UiManager& uiManager_;
  String lastError_;
};

}  // namespace tipsy::app
