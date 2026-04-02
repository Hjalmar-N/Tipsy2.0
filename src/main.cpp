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
tipsy::ui::UiBridge uiBridge(machineController, recipeService, settingsService);
tipsy::ui::UiManager uiManager(uiBridge);
tipsy::app::Application application(storageManager, recipeService, ingredientService,
                                    settingsService, machineController, pumpController, uiManager);
bool applicationReady = false;

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(5000); // allow host to connect to native USB-Serial/JTAG

  log_printf("--- Tipsy 2.0 Boot Sequence ---\n");

  // I2C scan completed — no devices found on SDA=8/SCL=9.
  // See i2c_scan2_clean.log for full results.

  log_printf("1. Initializing Pump Controller...\n");
  const bool pumpsReady = pumpController.begin();
  
  log_printf("2. Initializing Application (LittleFS, Display, UI)...\n");
  applicationReady = pumpsReady && application.begin();

  if (!applicationReady) {
    log_printf("Tipsy2.0 startup failed: ");
    if (!pumpsReady) {
      log_printf("Pump controller initialization failed.\n");
    } else {
      log_printf("%s\n", application.lastError().c_str());
    }
  } else {
    log_printf("Tipsy 2.0 booted successfully!\n");
  }
}

void loop() {
  // Heartbeat every 5 seconds to confirm device is alive
  static uint32_t lastHeartbeat = 0;
  uint32_t now = millis();
  if (now - lastHeartbeat >= 5000) {
    log_printf("[heartbeat] %lu ms, appReady=%d\n", now, applicationReady ? 1 : 0);
    lastHeartbeat = now;
  }

  if (!applicationReady) {
    return;
  }

  static uint32_t lastMillis = millis();
  uint32_t currentMillis = millis();
  uint32_t deltaMs = currentMillis - lastMillis;
  lastMillis = currentMillis;

  if (deltaMs == 0) {
    deltaMs = 1;
  }

  uiManager.tick(deltaMs);
  application.update();
}

