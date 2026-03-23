#pragma once

#include "app/MachineController.h"
#include "ui/UiState.h"

namespace tipsy::ui {

// Translates future LVGL/SquareLine callbacks into application-layer commands.
class UiBridge {
 public:
  explicit UiBridge(tipsy::app::MachineController& machineController);

  void begin();
  void syncFromMachine();

  tipsy::app::MachineStatus onSelectDrink(const String& drinkId);
  tipsy::app::MachineStatus onStartManualPour(const String& ingredientId, float volumeMl,
                                              std::uint8_t speedPercent);
  tipsy::app::MachineStatus onOpenAdmin();
  tipsy::app::MachineStatus onEditPumpAssignment(std::uint8_t pumpIndex,
                                                 const String& ingredientId,
                                                 const String& ingredientDisplayName,
                                                 bool enabled);
  UiState currentState() const;

 private:
  void updateUiState(const tipsy::app::MachineStatus& status);

  tipsy::app::MachineController& machineController_;
  UiState uiState_ {};
};

}  // namespace tipsy::ui
