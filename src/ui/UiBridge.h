#pragma once

#include <cstdint>

#include "app/MachineController.h"
#include "app/services/RecipeService.h"
#include "app/services/SettingsService.h"
#include "ui/UiState.h"

namespace tipsy::ui {

// Translates future LVGL/SquareLine callbacks into application-layer commands.
class UiBridge {
 public:
  UiBridge(tipsy::app::MachineController& machineController,
           tipsy::app::RecipeService& recipeService,
           tipsy::app::SettingsService& settingsService);

  void begin();
  void syncFromMachine();

  tipsy::app::MachineStatus onSelectDrink(const String& drinkId);
  tipsy::app::MachineStatus onStartSelectedDrink(std::uint16_t alcoholOverrideMl = 0,
                                                 std::uint8_t speedPercent = 100);
  tipsy::app::MachineStatus onStartManualPour(const String& ingredientId, float volumeMl,
                                              std::uint8_t speedPercent);
  tipsy::app::MachineStatus onOpenAdmin();
  tipsy::app::MachineStatus onPrimePumps();
  tipsy::app::MachineStatus onFlushCleaning();
  tipsy::app::MachineStatus onEditPumpAssignment(std::uint8_t pumpIndex,
                                                 const String& ingredientId,
                                                 const String& ingredientDisplayName,
                                                 bool enabled);
  bool isDrinkAvailable(const String& drinkId) const;
  UiState currentState() const;

 private:
  void refreshDrinkList();
  void updateUiState(const tipsy::app::MachineStatus& status);

  tipsy::app::MachineController& machineController_;
  tipsy::app::RecipeService& recipeService_;
  tipsy::app::SettingsService& settingsService_;
  UiState uiState_ {};
};

}  // namespace tipsy::ui
