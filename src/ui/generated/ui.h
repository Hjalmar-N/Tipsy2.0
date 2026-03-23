#pragma once

#include <Arduino.h>
#include <array>
#include <cstddef>

#include "config/AppConfig.h"

namespace tipsy::ui::generated {

// Placeholder-friendly render model owned by the generated UI layer.
struct UiRenderDrinkItem {
  String id;
  String displayName;
  bool available = false;
  bool selected = false;
};

// The adapter fills this view model, and real SquareLine widgets can bind to it later.
struct UiRenderModel {
  String machineStateText;
  String statusText;
  String selectedDrinkText;
  bool hasSelectedDrink = false;
  bool canStartSelectedDrink = false;
  std::array<UiRenderDrinkItem, tipsy::config::kMaxDrinkCount> drinks {};
  std::size_t drinkCount = 0;
};

using DrinkSelectedCallback = void (*)(const char* drinkId);
using StartSelectedDrinkCallback = void (*)();

void ui_init();
void ui_apply_model(const UiRenderModel& model);
const UiRenderModel& ui_current_model();
void ui_bind_drink_selected(DrinkSelectedCallback callback);
void ui_bind_start_selected_drink(StartSelectedDrinkCallback callback);
void ui_trigger_select_drink(const char* drinkId);
void ui_trigger_start_selected_drink();

}  // namespace tipsy::ui::generated
