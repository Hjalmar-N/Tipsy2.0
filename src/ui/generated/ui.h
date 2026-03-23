#pragma once

#include <Arduino.h>
#include <array>
#include <cstddef>
#include <lvgl.h>

#include "config/AppConfig.h"

namespace tipsy::ui::generated {

// Placeholder-friendly render model owned by the generated UI layer.
struct UiRenderDrinkItem {
  String id;
  String displayName;
  String availabilityText;
  bool available = false;
  bool selected = false;
  bool disabled = false;
};

// Temporary screen contract for the first testable UI slice.
// Real SquareLine widgets should later bind to these fields instead of changing app logic.
struct UiRenderModel {
  String headerTitle;
  String headerSubtitle;
  String machineStateLabel;
  String machineStateText;
  String statusLabel;
  String statusText;
  String selectedDrinkLabel;
  String selectedDrinkText;
  String feedbackTitle;
  String feedbackText;
  String primaryActionLabel;
  bool hasSelectedDrink = false;
  bool canStartSelectedDrink = false;
  bool showPouringFeedback = false;
  bool showCompleteFeedback = false;
  bool showErrorFeedback = false;
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
String ui_debug_screen_text();

}  // namespace tipsy::ui::generated
