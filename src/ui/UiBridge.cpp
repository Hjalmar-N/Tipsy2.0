#include "ui/UiBridge.h"

namespace tipsy::ui {

UiBridge::UiBridge(tipsy::app::MachineController& machineController,
                   tipsy::app::RecipeService& recipeService)
    : machineController_(machineController), recipeService_(recipeService) {}

void UiBridge::begin() {
  syncFromMachine();
}

void UiBridge::syncFromMachine() {
  updateUiState(machineController_.getLastStatus());
}

tipsy::app::MachineStatus UiBridge::onSelectDrink(const String& drinkId) {
  const auto status = machineController_.selectDrink(drinkId);
  updateUiState(status);
  return status;
}

tipsy::app::MachineStatus UiBridge::onStartManualPour(const String& ingredientId, float volumeMl,
                                                      std::uint8_t speedPercent) {
  const auto status = machineController_.startManualPour(ingredientId, volumeMl, speedPercent);
  updateUiState(status);
  return status;
}

tipsy::app::MachineStatus UiBridge::onOpenAdmin() {
  const auto status = machineController_.enterAdminSettings();
  updateUiState(status);
  return status;
}

tipsy::app::MachineStatus UiBridge::onEditPumpAssignment(std::uint8_t pumpIndex,
                                                         const String& ingredientId,
                                                         const String& ingredientDisplayName,
                                                         bool enabled) {
  const auto status = machineController_.updatePumpAssignment(
      pumpIndex, ingredientId, ingredientDisplayName, enabled);
  updateUiState(status);
  return status;
}

bool UiBridge::isDrinkAvailable(const String& drinkId) const {
  return machineController_.isDrinkAvailable(drinkId);
}

UiState UiBridge::currentState() const {
  return uiState_;
}

void UiBridge::updateUiState(const tipsy::app::MachineStatus& status) {
  uiState_.machineState = status.state;
  uiState_.statusCode = status.code;
  uiState_.statusMessage = status.message;
  uiState_.selectedDrinkId = machineController_.getSelectedDrinkId();
  uiState_.adminOpen = status.state == tipsy::app::MachineState::AdminSettings;
  refreshDrinkList();
}

void UiBridge::refreshDrinkList() {
  uiState_.drinkCount = 0;

  const auto& recipes = recipeService_.all();
  for (std::size_t i = 0; i < recipeService_.count() && i < uiState_.drinks.size(); ++i) {
    const auto& recipe = recipes[i];
    auto& item = uiState_.drinks[i];
    item.id = recipe.id;
    item.displayName = recipe.displayName;
    item.available = machineController_.isDrinkAvailable(recipe.id);
    item.selected = recipe.id == machineController_.getSelectedDrinkId();
    ++uiState_.drinkCount;
  }
}

}  // namespace tipsy::ui
