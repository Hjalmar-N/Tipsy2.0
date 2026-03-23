#include <Arduino.h>

#include "app/Application.h"
#include "app/MachineController.h"
#include "app/services/IngredientService.h"
#include "app/services/RecipeService.h"
#include "app/services/SettingsService.h"
#include "hal/HardwareFactory.h"
#include "pumps/PumpController.h"
#include "storage/FileSystemManager.h"
#include "storage/JsonStorage.h"
#include "storage/StorageManager.h"
#include "ui/UiBridge.h"
#include "ui/UiManager.h"

namespace {

tipsy::hal::HardwareBundle hardware = tipsy::hal::HardwareFactory::create();
tipsy::storage::FileSystemManager fileSystemManager(*hardware.fileSystem);
tipsy::storage::JsonStorage jsonStorage(fileSystemManager);
tipsy::storage::StorageManager storageManager(fileSystemManager, jsonStorage);
tipsy::pumps::PumpController pumpController(*hardware.pumpDriver, *hardware.timeProvider);
tipsy::app::RecipeService recipeService(jsonStorage);
tipsy::app::IngredientService ingredientService(jsonStorage);
tipsy::app::SettingsService settingsService(jsonStorage);
tipsy::app::MachineController machineController(recipeService, ingredientService, settingsService,
                                                pumpController);
tipsy::ui::UiBridge uiBridge(machineController);
tipsy::ui::UiManager uiManager(uiBridge);
tipsy::app::Application application(storageManager, recipeService, ingredientService,
                                    settingsService, machineController, pumpController, uiManager);
bool applicationReady = false;

}  // namespace

void setup() {
  Serial.begin(115200);

  const bool pumpsReady = pumpController.begin();
  applicationReady = pumpsReady && application.begin();

  if (!applicationReady) {
    Serial.print("Tipsy2.0 startup failed: ");
    if (!pumpsReady) {
      Serial.println("Pump controller initialization failed.");
    } else {
      Serial.println(application.lastError());
    }
  }
}

void loop() {
  if (!applicationReady) {
    return;
  }

  application.update();
}
