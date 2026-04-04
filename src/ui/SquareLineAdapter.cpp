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

void onStartSelectedDrink(std::uint16_t alcoholAmountMl) {
  if (boundUiBridge == nullptr) {
    return;
  }

  tipsy::ui::events::handleStartSelectedDrink(*boundUiBridge, alcoholAmountMl);
}

void onAdminOpened() {
  if (boundUiBridge == nullptr) {
    return;
  }

  tipsy::ui::events::handleAdminOpened(*boundUiBridge);
}

void onPrimePumps() {
  if (boundUiBridge == nullptr) {
    return;
  }

  tipsy::ui::events::handlePrimePumpsRequested(*boundUiBridge);
}

void onFlushCleaning() {
  if (boundUiBridge == nullptr) {
    return;
  }

  tipsy::ui::events::handleFlushCleaningRequested(*boundUiBridge);
}

void onPumpAssignmentEdited(std::uint8_t pumpIndex, const char* ingredientId,
                            const char* ingredientDisplayName, bool enabled) {
  if (boundUiBridge == nullptr) {
    return;
  }

  tipsy::ui::events::handlePumpAssignmentEdited(*boundUiBridge, pumpIndex, ingredientId,
                                                ingredientDisplayName, enabled);
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
  generated::ui_bind_admin_opened(&onAdminOpened);
  generated::ui_bind_prime_pumps(&onPrimePumps);
  generated::ui_bind_flush_cleaning(&onFlushCleaning);
  generated::ui_bind_pump_assignment_edited(&onPumpAssignmentEdited);
}

void SquareLineAdapter::update() {
  if (uiBridge_ == nullptr) {
    return;
  }

  const UiState state = uiBridge_->currentState();
  const generated::UiRenderModel model = buildRenderModel(state);
  if (hasRenderedModel_ && renderModelsEqual(lastRenderedModel_, model)) {
    return;
  }

  generated::ui_apply_model(model);
  lastRenderedModel_ = model;
  hasRenderedModel_ = true;
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
      model.feedbackText = "Drink is being poured.";
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
  model.headerTitle = "Tipsy";
  model.headerSubtitle = String();
  model.machineStateLabel = "Machine";
  model.machineStateText = machineStateText(state.machineState);
  model.statusLabel = "Status";
  model.statusText = state.machineState == tipsy::app::MachineState::Idle ? "Ready"
                                                                          : state.statusMessage;
  model.selectedDrinkLabel = "Selected";
  model.primaryActionLabel = "Pour Drink";
  model.selectedDrinkText = "None";
  model.hasSelectedDrink = !state.selectedDrinkId.isEmpty();

  model.drinkCount = state.drinkCount;

  for (std::size_t i = 0; i < state.drinkCount && i < model.drinks.size(); ++i) {
    model.drinks[i].id = state.drinks[i].id;
    model.drinks[i].displayName = state.drinks[i].displayName;
    model.drinks[i].subtitle = state.drinks[i].subtitle;
    model.drinks[i].categoryId = state.drinks[i].categoryId;
    model.drinks[i].availabilityText = state.drinks[i].available ? "Available" : "Unavailable";
    model.drinks[i].available = state.drinks[i].available;
    model.drinks[i].selected = state.drinks[i].selected;
    model.drinks[i].disabled = !state.drinks[i].available;

    if (state.drinks[i].selected) {
      model.selectedDrinkText = state.drinks[i].displayName;
      model.hasSelectedDrink = true;
    }
  }

  for (std::size_t i = 0; i < state.pumpAssignments.size() && i < model.pumpAssignments.size(); ++i) {
    model.pumpAssignments[i].pumpIndex = state.pumpAssignments[i].pumpIndex;
    model.pumpAssignments[i].ingredientId = state.pumpAssignments[i].ingredientId;
    model.pumpAssignments[i].ingredientDisplayName =
        state.pumpAssignments[i].ingredientDisplayName;
    model.pumpAssignments[i].enabled = state.pumpAssignments[i].enabled;
  }

  model.canStartSelectedDrink =
      model.hasSelectedDrink &&
      (state.machineState == tipsy::app::MachineState::DrinkSelection ||
       state.machineState == tipsy::app::MachineState::Complete ||
       state.machineState == tipsy::app::MachineState::Idle);

  applyStateFeedback(state, model);

  return model;
}

bool SquareLineAdapter::renderModelsEqual(const generated::UiRenderModel& lhs,
                                          const generated::UiRenderModel& rhs) {
  if (lhs.headerTitle != rhs.headerTitle ||
      lhs.headerSubtitle != rhs.headerSubtitle ||
      lhs.machineStateLabel != rhs.machineStateLabel ||
      lhs.machineStateText != rhs.machineStateText ||
      lhs.statusLabel != rhs.statusLabel ||
      lhs.statusText != rhs.statusText ||
      lhs.selectedDrinkLabel != rhs.selectedDrinkLabel ||
      lhs.selectedDrinkText != rhs.selectedDrinkText ||
      lhs.feedbackTitle != rhs.feedbackTitle ||
      lhs.feedbackText != rhs.feedbackText ||
      lhs.primaryActionLabel != rhs.primaryActionLabel ||
      lhs.hasSelectedDrink != rhs.hasSelectedDrink ||
      lhs.canStartSelectedDrink != rhs.canStartSelectedDrink ||
      lhs.showPouringFeedback != rhs.showPouringFeedback ||
      lhs.showCompleteFeedback != rhs.showCompleteFeedback ||
      lhs.showErrorFeedback != rhs.showErrorFeedback ||
      lhs.drinkCount != rhs.drinkCount) {
    return false;
  }

  for (std::size_t i = 0; i < lhs.drinkCount && i < lhs.drinks.size(); ++i) {
    const auto& left = lhs.drinks[i];
    const auto& right = rhs.drinks[i];
    if (left.id != right.id ||
        left.displayName != right.displayName ||
        left.subtitle != right.subtitle ||
        left.categoryId != right.categoryId ||
        left.availabilityText != right.availabilityText ||
        left.available != right.available ||
        left.selected != right.selected ||
        left.disabled != right.disabled) {
      return false;
    }
  }

  for (std::size_t i = 0; i < lhs.pumpAssignments.size(); ++i) {
    const auto& left = lhs.pumpAssignments[i];
    const auto& right = rhs.pumpAssignments[i];
    if (left.pumpIndex != right.pumpIndex ||
        left.ingredientId != right.ingredientId ||
        left.ingredientDisplayName != right.ingredientDisplayName ||
        left.enabled != right.enabled) {
      return false;
    }
  }

  return true;
}

}  // namespace tipsy::ui
