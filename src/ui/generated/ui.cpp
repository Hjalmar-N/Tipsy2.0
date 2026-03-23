#include "ui/generated/ui.h"

namespace tipsy::ui::generated {

namespace {

UiRenderModel currentModel {};
DrinkSelectedCallback drinkSelectedCallback = nullptr;
StartSelectedDrinkCallback startSelectedDrinkCallback = nullptr;

}  // namespace

void ui_init() {
  currentModel = UiRenderModel {};
  currentModel.headerTitle = "Tipsy2.0";
  currentModel.headerSubtitle = "Mock service screen";
  currentModel.machineStateLabel = "Machine";
  currentModel.statusLabel = "Status";
  currentModel.selectedDrinkLabel = "Selected";
  currentModel.primaryActionLabel = "Start";
  // Replaced by SquareLine export later.
}

void ui_apply_model(const UiRenderModel& model) {
  currentModel = model;
  // Future work: push this render model into real SquareLine widgets.
}

const UiRenderModel& ui_current_model() {
  return currentModel;
}

void ui_bind_drink_selected(DrinkSelectedCallback callback) {
  drinkSelectedCallback = callback;
}

void ui_bind_start_selected_drink(StartSelectedDrinkCallback callback) {
  startSelectedDrinkCallback = callback;
}

void ui_trigger_select_drink(const char* drinkId) {
  if (drinkSelectedCallback != nullptr && drinkId != nullptr) {
    drinkSelectedCallback(drinkId);
  }
}

void ui_trigger_start_selected_drink() {
  if (startSelectedDrinkCallback != nullptr) {
    startSelectedDrinkCallback();
  }
}

String ui_debug_screen_text() {
  String text;
  text.reserve(512);

  text += currentModel.headerTitle;
  text += "\n";

  if (!currentModel.headerSubtitle.isEmpty()) {
    text += currentModel.headerSubtitle;
    text += "\n";
  }

  text += currentModel.machineStateLabel;
  text += ": ";
  text += currentModel.machineStateText;
  text += "\n";

  text += currentModel.statusLabel;
  text += ": ";
  text += currentModel.statusText;
  text += "\n";

  text += currentModel.selectedDrinkLabel;
  text += ": ";
  text += currentModel.selectedDrinkText;
  text += "\n";

  text += "Action: ";
  text += currentModel.primaryActionLabel;
  text += currentModel.canStartSelectedDrink ? " [enabled]" : " [disabled]";
  text += "\n";

  if (!currentModel.feedbackTitle.isEmpty()) {
    text += currentModel.feedbackTitle;
    text += ": ";
    text += currentModel.feedbackText;
    text += "\n";
  }

  text += "Drinks:\n";
  for (std::size_t i = 0; i < currentModel.drinkCount; ++i) {
    const auto& item = currentModel.drinks[i];
    text += item.selected ? " * " : " - ";
    text += item.displayName;
    text += " (";
    text += item.availabilityText;
    text += ")";
    text += "\n";
  }

  return text;
}

}  // namespace tipsy::ui::generated
