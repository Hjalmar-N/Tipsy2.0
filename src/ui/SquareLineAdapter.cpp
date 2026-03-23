#include "ui/SquareLineAdapter.h"

#include "app/models/MachineState.h"
#include "ui/UiEvents.h"
#include "ui/generated/ui.h"

namespace tipsy::ui {

namespace {

UiBridge* boundUiBridge = nullptr;

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

String SquareLineAdapter::machineStateText(tipsy::app::MachineState state) const {
  switch (state) {
    case tipsy::app::MachineState::Idle:
      return "Idle";
    case tipsy::app::MachineState::DrinkSelection:
      return "Ready to start";
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

void SquareLineAdapter::applyStateFeedback(const UiState& state,
                                           generated::UiRenderModel& model) const {
  switch (state.machineState) {
    case tipsy::app::MachineState::Pouring:
      model.feedbackTitle = "Pouring";
      model.feedbackText = "Drink is being poured in mock mode.";
      model.showPouringFeedback = true;
      break;
    case tipsy::app::MachineState::Complete:
      model.feedbackTitle = "Complete";
      model.feedbackText = "Drink finished. Ready for the next order.";
      model.showCompleteFeedback = true;
      break;
    case tipsy::app::MachineState::Error:
      model.feedbackTitle = "Attention";
      model.feedbackText = state.statusMessage.isEmpty() ? "Machine needs attention."
                                                         : state.statusMessage;
      model.showErrorFeedback = true;
      break;
    default:
      if (state.hasSelectedDrink && model.canStartSelectedDrink) {
        model.feedbackTitle = "Ready";
        model.feedbackText = "Selected drink can be started.";
      } else if (!state.hasSelectedDrink) {
        model.feedbackTitle = "Selection";
        model.feedbackText = "Choose an available drink to continue.";
      }
      break;
  }
}

generated::UiRenderModel SquareLineAdapter::buildRenderModel(const UiState& state) const {
  generated::UiRenderModel model {};
  model.headerTitle = "Tipsy2.0";
  model.headerSubtitle = "Cocktail machine mock UI";
  model.machineStateLabel = "Machine";
  model.machineStateText = machineStateText(state.machineState);
  model.statusLabel = "Status";
  model.statusText = state.statusMessage;
  model.selectedDrinkLabel = "Selected";
  model.primaryActionLabel = "Start Drink";
  model.selectedDrinkText = "None";
  model.hasSelectedDrink = !state.selectedDrinkId.isEmpty();

  model.drinkCount = state.drinkCount;

  for (std::size_t i = 0; i < state.drinkCount && i < model.drinks.size(); ++i) {
    model.drinks[i].id = state.drinks[i].id;
    model.drinks[i].displayName = state.drinks[i].displayName;
    model.drinks[i].availabilityText = state.drinks[i].available ? "Available" : "Unavailable";
    model.drinks[i].available = state.drinks[i].available;
    model.drinks[i].selected = state.drinks[i].selected;
    model.drinks[i].disabled = !state.drinks[i].available;

    if (state.drinks[i].selected) {
      model.selectedDrinkText = state.drinks[i].displayName;
      model.hasSelectedDrink = true;
    }
  }

  model.canStartSelectedDrink =
      model.hasSelectedDrink &&
      (state.machineState == tipsy::app::MachineState::DrinkSelection ||
       state.machineState == tipsy::app::MachineState::Complete ||
       state.machineState == tipsy::app::MachineState::Idle);

  applyStateFeedback(state, model);

  return model;
}

}  // namespace tipsy::ui
