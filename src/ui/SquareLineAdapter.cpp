#include "ui/SquareLineAdapter.h"

#include "app/models/MachineState.h"
#include "ui/UiEvents.h"
#include "ui/generated/ui.h"

namespace tipsy::ui {

namespace {

UiBridge* boundUiBridge = nullptr;

const char* machineStateText(tipsy::app::MachineState state) {
  switch (state) {
    case tipsy::app::MachineState::Idle:
      return "Idle";
    case tipsy::app::MachineState::DrinkSelection:
      return "Drink selected";
    case tipsy::app::MachineState::ManualPour:
      return "Manual pour";
    case tipsy::app::MachineState::AdminSettings:
      return "Admin settings";
    case tipsy::app::MachineState::Pouring:
      return "Pouring";
    case tipsy::app::MachineState::Complete:
      return "Complete";
    case tipsy::app::MachineState::Error:
      return "Error";
    case tipsy::app::MachineState::Maintenance:
      return "Maintenance";
    default:
      return "Unknown";
  }
}

void onDrinkSelected(const char* drinkId) {
  if (boundUiBridge == nullptr) {
    return;
  }

  tipsy::ui::events::handleDrinkSelected(*boundUiBridge, drinkId);
}

void onStartSelectedDrink() {
  if (boundUiBridge == nullptr) {
    return;
  }

  tipsy::ui::events::handleStartSelectedDrink(*boundUiBridge);
}

}  // namespace

void SquareLineAdapter::bind(UiBridge& uiBridge) {
  uiBridge_ = &uiBridge;
}

void SquareLineAdapter::begin() {
  boundUiBridge = uiBridge_;
  generated::ui_init();
  generated::ui_bind_drink_selected(&onDrinkSelected);
  generated::ui_bind_start_selected_drink(&onStartSelectedDrink);
}

void SquareLineAdapter::update() {
  if (uiBridge_ == nullptr) {
    return;
  }

  const UiState state = uiBridge_->currentState();
  generated::ui_apply_model(buildRenderModel(state));
}

generated::UiRenderModel SquareLineAdapter::buildRenderModel(const UiState& state) const {
  generated::UiRenderModel model {};
  model.machineStateText = String(machineStateText(state.machineState));
  model.statusText = state.statusMessage;
  model.selectedDrinkText = "None";

  model.drinkCount = state.drinkCount;

  for (std::size_t i = 0; i < state.drinkCount && i < model.drinks.size(); ++i) {
    model.drinks[i].id = state.drinks[i].id;
    model.drinks[i].displayName = state.drinks[i].displayName;
    model.drinks[i].available = state.drinks[i].available;
    model.drinks[i].selected = state.drinks[i].selected;

    if (state.drinks[i].selected) {
      model.selectedDrinkText = state.drinks[i].displayName;
      model.hasSelectedDrink = true;
    }
  }

  model.canStartSelectedDrink = state.machineState == tipsy::app::MachineState::DrinkSelection &&
                                model.hasSelectedDrink;

  return model;
}

}  // namespace tipsy::ui
