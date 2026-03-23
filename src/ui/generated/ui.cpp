#include "ui/generated/ui.h"

namespace tipsy::ui::generated {

namespace {

UiRenderModel currentModel {};
DrinkSelectedCallback drinkSelectedCallback = nullptr;
StartSelectedDrinkCallback startSelectedDrinkCallback = nullptr;

lv_obj_t* screenRoot = nullptr;
lv_obj_t* contentColumn = nullptr;
lv_obj_t* titleLabel = nullptr;
lv_obj_t* statusLabel = nullptr;
lv_obj_t* pourDrinkButton = nullptr;
lv_obj_t* pourDrinkButtonLabel = nullptr;
lv_obj_t* recipesButton = nullptr;
lv_obj_t* settingsButton = nullptr;

void updateVisibleTexts() {
  if (titleLabel != nullptr) {
    lv_label_set_text(titleLabel,
                      currentModel.headerTitle.isEmpty() ? "Tipsy" : currentModel.headerTitle.c_str());
  }

  if (statusLabel != nullptr) {
    const char* statusText = currentModel.statusText.isEmpty() ? "Ready" : currentModel.statusText.c_str();
    lv_label_set_text(statusLabel, statusText);
  }

  if (pourDrinkButtonLabel != nullptr) {
    lv_label_set_text(
        pourDrinkButtonLabel,
        currentModel.primaryActionLabel.isEmpty() ? "Pour Drink" : currentModel.primaryActionLabel.c_str());
  }

  if (pourDrinkButton != nullptr) {
    if (currentModel.canStartSelectedDrink) {
      lv_obj_remove_state(pourDrinkButton, LV_STATE_DISABLED);
    } else {
      lv_obj_add_state(pourDrinkButton, LV_STATE_DISABLED);
    }
  }
}

lv_obj_t* createMenuButton(lv_obj_t* parent, const char* text,
                           lv_event_cb_t callback) {
  lv_obj_t* button = lv_button_create(parent);
  lv_obj_set_width(button, 220);
  lv_obj_set_height(button, 52);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, nullptr);

  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_center(label);
  return button;
}

void handlePourDrinkClicked(lv_event_t* event) {
  (void)event;
  ui_trigger_start_selected_drink();
}

void handleRecipesClicked(lv_event_t* event) {
  (void)event;
  // Placeholder until a real recipe screen exists.
}

void handleSettingsClicked(lv_event_t* event) {
  (void)event;
  // Placeholder until a real settings screen exists.
}

}  // namespace

void ui_init() {
  currentModel = UiRenderModel {};
  currentModel.headerTitle = "Tipsy";
  currentModel.headerSubtitle = String();
  currentModel.machineStateLabel = "Machine";
  currentModel.statusLabel = "Status";
  currentModel.statusText = "Ready";
  currentModel.selectedDrinkLabel = "Selected";
  currentModel.primaryActionLabel = "Pour Drink";

  screenRoot = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(screenRoot, lv_color_hex(0x101418), 0);
  lv_obj_set_style_bg_opa(screenRoot, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(screenRoot, 0, 0);
  lv_obj_set_style_pad_all(screenRoot, 0, 0);

  contentColumn = lv_obj_create(screenRoot);
  lv_obj_set_width(contentColumn, LV_PCT(100));
  lv_obj_set_height(contentColumn, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(contentColumn, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(contentColumn, 0, 0);
  lv_obj_set_style_pad_top(contentColumn, 28, 0);
  lv_obj_set_style_pad_bottom(contentColumn, 20, 0);
  lv_obj_set_style_pad_left(contentColumn, 20, 0);
  lv_obj_set_style_pad_right(contentColumn, 20, 0);
  lv_obj_set_style_pad_row(contentColumn, 14, 0);
  lv_obj_set_layout(contentColumn, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(contentColumn, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(contentColumn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_align(contentColumn, LV_ALIGN_TOP_MID, 0, 0);

  titleLabel = lv_label_create(contentColumn);
  lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xF2F4F8), 0);

  statusLabel = lv_label_create(contentColumn);
  lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xA6B0BF), 0);

  pourDrinkButton = createMenuButton(contentColumn, "Pour Drink", handlePourDrinkClicked);
  pourDrinkButtonLabel = lv_obj_get_child(pourDrinkButton, 0);
  recipesButton = createMenuButton(contentColumn, "Recipes", handleRecipesClicked);
  settingsButton = createMenuButton(contentColumn, "Settings", handleSettingsClicked);

  lv_screen_load(screenRoot);
  updateVisibleTexts();
}

void ui_apply_model(const UiRenderModel& model) {
  currentModel = model;
  updateVisibleTexts();
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
