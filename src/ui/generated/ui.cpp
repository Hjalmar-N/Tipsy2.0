#include "ui/generated/ui.h"

namespace tipsy::ui::generated {

namespace {

UiRenderModel currentModel {};
DrinkSelectedCallback drinkSelectedCallback = nullptr;
StartSelectedDrinkCallback startSelectedDrinkCallback = nullptr;

}  // namespace

void ui_init() {
  currentModel = UiRenderModel {};
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

}  // namespace tipsy::ui::generated
