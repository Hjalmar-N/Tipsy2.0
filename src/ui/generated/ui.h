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
  String subtitle;
  String categoryId;
  String availabilityText;
  bool available = false;
  bool selected = false;
  bool disabled = false;
};

struct UiRenderPumpAssignmentItem {
  std::uint8_t pumpIndex = 0;
  String ingredientId;
  String ingredientDisplayName;
  bool enabled = true;
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
  std::array<UiRenderPumpAssignmentItem, tipsy::config::kPumpCount> pumpAssignments {};
};

using DrinkSelectedCallback = void (*)(const char* drinkId);
using StartSelectedDrinkCallback = void (*)(std::uint16_t alcoholAmountMl);
using AdminOpenedCallback = void (*)();
using PrimePumpsCallback = void (*)();
using FlushCleaningCallback = void (*)();
using PumpAssignmentEditedCallback =
    void (*)(std::uint8_t pumpIndex, const char* ingredientId, const char* ingredientDisplayName,
             bool enabled);

void ui_init();
void ui_init_preview_main_menu();
void ui_apply_model(const UiRenderModel& model);
const UiRenderModel& ui_current_model();
void ui_bind_drink_selected(DrinkSelectedCallback callback);
void ui_bind_start_selected_drink(StartSelectedDrinkCallback callback);
void ui_bind_admin_opened(AdminOpenedCallback callback);
void ui_bind_prime_pumps(PrimePumpsCallback callback);
void ui_bind_flush_cleaning(FlushCleaningCallback callback);
void ui_bind_pump_assignment_edited(PumpAssignmentEditedCallback callback);
void ui_trigger_select_drink(const char* drinkId);
void ui_trigger_start_selected_drink(std::uint16_t alcoholAmountMl);
void ui_trigger_admin_opened();
void ui_trigger_prime_pumps();
void ui_trigger_flush_cleaning();
void ui_trigger_pump_assignment_edited(std::uint8_t pumpIndex, const char* ingredientId,
                                       const char* ingredientDisplayName, bool enabled);

}  // namespace tipsy::ui::generated
