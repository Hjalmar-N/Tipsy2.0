#include "ui/generated/ui.h"

namespace tipsy::ui::generated {

namespace {

enum class ScreenView : std::uint8_t {
  Boot = 0,
  MainMenu,
  DrinkDetail,
  Settings,
};

enum class DrinkCategory : std::uint8_t {
  All = 0,
  Drinks,
  Shots,
};

enum class SettingsSubview : std::uint8_t {
  Main = 0,
  PumpMapping,
  Calibration,
  ServiceMode,
};

enum class MockPourStage : std::uint8_t {
  Idle = 0,
  Pouring,
  Complete,
};

struct MockMixerIngredient {
  const char* id;
  const char* name;
  std::uint16_t amountMl;
};

struct MockIngredientOption {
  const char* id;
  const char* name;
};

struct MockRecipeDetail {
  const char* itemId;
  const char* alcoholIngredientId;
  const char* alcoholName;
  bool isShot = false;
  std::array<MockMixerIngredient, 3> mixers {};
  std::size_t mixerCount = 0;
};

struct InternalPourIngredient {
  const char* ingredientId;
  const char* displayName;
  std::uint16_t amountMl;
  bool isAlcohol = false;
};

struct InternalPourRequest {
  String itemId;
  String itemName;
  bool isShot = false;
  std::uint16_t alcoholAmountMl = 0;
  std::array<InternalPourIngredient, 4> ingredients {};
  std::size_t ingredientCount = 0;
  std::uint16_t estimatedPourTimeSec = 0;
  bool canRouteToBackend = false;
};

constexpr MockRecipeDetail kRecipeDetails[] = {
    {"mojito", "rum", "White Rum", false, {{{"mint_mix", "Mint mix", 120}, {"soda", "Soda", 80}, {"lime", "Lime", 20}}}, 3},
    {"gin_tonic", "gin", "Gin", false, {{{"tonic", "Tonic", 150}, {"", "", 0}, {"", "", 0}}}, 1},
    {"whiskey_sour", "whiskey", "Whiskey", false, {{{"sour_mix", "Sour mix", 90}, {"lime", "Lime", 20}, {"", "", 0}}}, 2},
    {"vodka_soda", "vodka", "Vodka", false, {{{"soda", "Soda", 150}, {"", "", 0}, {"", "", 0}}}, 1},
    {"margarita", "tequila", "Tequila", false, {{{"citrus_mix", "Citrus mix", 80}, {"lime", "Lime", 20}, {"", "", 0}}}, 2},
    {"rum_cola", "rum", "Rum", false, {{{"cola", "Cola", 150}, {"", "", 0}, {"", "", 0}}}, 1},
    {"tequila_shot", "tequila", "Tequila", true, {{{"", "", 0}, {"", "", 0}, {"", "", 0}}}, 0},
    {"vodka_shot", "vodka", "Vodka", true, {{{"", "", 0}, {"", "", 0}, {"", "", 0}}}, 0},
    {"whiskey_shot", "whiskey", "Whiskey", true, {{{"", "", 0}, {"", "", 0}, {"", "", 0}}}, 0},
};

constexpr MockIngredientOption kIngredientOptions[] = {
    {"", "Unassigned"},
    {"gin", "Gin"},
    {"vodka", "Vodka"},
    {"rum", "Rum"},
    {"tequila", "Tequila"},
    {"whiskey", "Whiskey"},
    {"tonic", "Tonic"},
    {"cola", "Cola"},
    {"soda", "Soda"},
    {"mint_mix", "Mint Mix"},
    {"lime", "Lime"},
    {"sour_mix", "Sour Mix"},
    {"citrus_mix", "Citrus Mix"},
};

constexpr std::size_t kPumpCount = 6;
constexpr std::size_t kIngredientOptionCount = sizeof(kIngredientOptions) / sizeof(kIngredientOptions[0]);

UiRenderModel currentModel {};
DrinkSelectedCallback drinkSelectedCallback = nullptr;
StartSelectedDrinkCallback startSelectedDrinkCallback = nullptr;
AdminOpenedCallback adminOpenedCallback = nullptr;
PrimePumpsCallback primePumpsCallback = nullptr;
FlushCleaningCallback flushCleaningCallback = nullptr;
PumpAssignmentEditedCallback pumpAssignmentEditedCallback = nullptr;

ScreenView currentView = ScreenView::Boot;
DrinkCategory activeCategory = DrinkCategory::All;
const UiRenderDrinkItem* selectedItem = nullptr;
std::uint8_t selectedStrengthIndex = 1;
lv_timer_t* bootTimer = nullptr;
bool previewModeEnabled = false;

lv_obj_t* rootScreen = nullptr;
lv_obj_t* previewModeBadge = nullptr;

lv_obj_t* bootScreen = nullptr;
lv_obj_t* bootLogoLabel = nullptr;
lv_obj_t* bootGlowLine = nullptr;

lv_obj_t* menuScreen = nullptr;
lv_obj_t* menuTitleLabel = nullptr;
lv_obj_t* menuStatusLabel = nullptr;
lv_obj_t* allTabButton = nullptr;
lv_obj_t* drinksTabButton = nullptr;
lv_obj_t* shotsTabButton = nullptr;
lv_obj_t* menuGrid = nullptr;
lv_obj_t* menuEmptyLabel = nullptr;
lv_obj_t* settingsButton = nullptr;

lv_obj_t* detailScreen = nullptr;
lv_obj_t* detailBackButton = nullptr;
lv_obj_t* detailTitleLabel = nullptr;
lv_obj_t* detailStatusLabel = nullptr;
lv_obj_t* detailSubtitleLabel = nullptr;
lv_obj_t* detailRecipeCard = nullptr;
lv_obj_t* detailRecipeTitleLabel = nullptr;
lv_obj_t* detailRecipeAlcoholLabel = nullptr;
lv_obj_t* detailRecipeMixerLabels[3] = {};
lv_obj_t* strengthButtons[3] = {};
lv_obj_t* strengthPrimaryLabels[3] = {};
lv_obj_t* strengthSecondaryLabels[3] = {};
lv_obj_t* detailSummaryLabel = nullptr;
lv_obj_t* detailEstimatedTimeLabel = nullptr;
lv_obj_t* pourButton = nullptr;

lv_obj_t* settingsScreen = nullptr;
lv_obj_t* settingsBackButton = nullptr;
lv_obj_t* settingsSubtitleLabel = nullptr;
lv_obj_t* settingsMainList = nullptr;
lv_obj_t* settingsStatusLabel = nullptr;
lv_obj_t* settingsSaveButton = nullptr;
lv_obj_t* adminLockSwitch = nullptr;
lv_obj_t* settingsDetailCard = nullptr;
lv_obj_t* settingsDetailTitleLabel = nullptr;
lv_obj_t* settingsDetailBodyLabel = nullptr;
lv_obj_t* pumpMappingList = nullptr;
lv_obj_t* pumpMappingValueLabels[kPumpCount] = {};

SettingsSubview currentSettingsSubview = SettingsSubview::Main;
bool adminLockEnabled = false;
bool settingsDirty = false;
String settingsStatusMessage;
MockPourStage mockPourStage = MockPourStage::Idle;
String transientStatusText;
lv_timer_t* mockPourTimer = nullptr;
InternalPourRequest activePourRequest {};
bool hasActivePourRequest = false;

const char* strengthClLabel(std::uint8_t index) {
  switch (index) {
    case 0:
      return "4 cl";
    case 1:
      return "6 cl";
    case 2:
      return "8 cl";
    default:
      return "6 cl";
  }
}

const char* strengthMlLabel(std::uint8_t index) {
  switch (index) {
    case 0:
      return "40 ml";
    case 1:
      return "60 ml";
    case 2:
      return "80 ml";
    default:
      return "60 ml";
  }
}

String effectiveStatusText() {
  if (!transientStatusText.isEmpty()) {
    return transientStatusText;
  }

  if (!currentModel.statusText.isEmpty()) {
    return currentModel.statusText;
  }

  return "Ready";
}

DrinkCategory categoryFromItem(const UiRenderDrinkItem& item) {
  return item.categoryId == "shot" ? DrinkCategory::Shots : DrinkCategory::Drinks;
}

bool itemMatchesCategory(const UiRenderDrinkItem& item, DrinkCategory category) {
  return category == DrinkCategory::All || categoryFromItem(item) == category;
}

const char* ingredientDisplayName(const char* ingredientId) {
  for (const auto& ingredient : kIngredientOptions) {
    if (String(ingredient.id) == ingredientId) {
      return ingredient.name;
    }
  }

  return "Unknown";
}

std::size_t ingredientOptionIndex(const char* ingredientId) {
  for (std::size_t i = 0; i < kIngredientOptionCount; ++i) {
    if (String(kIngredientOptions[i].id) == ingredientId) {
      return i;
    }
  }

  return 0;
}



void refreshPumpMappingList();
void refreshVisibleStatus();
std::uint16_t selectedAlcoholAmountMl();
std::uint16_t estimatePourTimeSeconds();

bool backendKnowsDrink(const char* itemId) {
  if (itemId == nullptr) {
    return false;
  }

  for (std::size_t i = 0; i < currentModel.drinkCount && i < currentModel.drinks.size(); ++i) {
    if (currentModel.drinks[i].id == itemId) {
      return true;
    }
  }

  return false;
}

bool backendDrinkAvailable(const char* itemId) {
  if (itemId == nullptr) {
    return false;
  }

  for (std::size_t i = 0; i < currentModel.drinkCount && i < currentModel.drinks.size(); ++i) {
    if (currentModel.drinks[i].id == itemId) {
      return currentModel.drinks[i].available;
    }
  }

  return false;
}

InternalPourRequest buildPourRequest() {
  InternalPourRequest request {};
  if (selectedItem == nullptr) {
    return request;
  }

  request.itemId = selectedItem->id;
  request.itemName = selectedItem->displayName;
  request.isShot = false;
  request.alcoholAmountMl = selectedAlcoholAmountMl();
  request.canRouteToBackend =
      backendKnowsDrink(selectedItem->id.c_str()) &&
      backendDrinkAvailable(selectedItem->id.c_str());

  return request;
}

void clearMockPourTimer() {
  if (mockPourTimer != nullptr) {
    lv_timer_delete(mockPourTimer);
    mockPourTimer = nullptr;
  }
}

void clearBootTimer() {
  if (bootTimer != nullptr) {
    lv_timer_delete(bootTimer);
    bootTimer = nullptr;
  }
}

void handleMockPourTimer(lv_timer_t* timer) {
  (void)timer;

  if (mockPourStage == MockPourStage::Pouring) {
    mockPourStage = MockPourStage::Complete;
    transientStatusText =
        hasActivePourRequest ? String(activePourRequest.itemName) + " complete."
                             : String("Pour complete.");
    if (mockPourTimer != nullptr) {
      lv_timer_set_period(mockPourTimer, 1400);
    }
    refreshVisibleStatus();
    return;
  }

  mockPourStage = MockPourStage::Idle;
  transientStatusText = "";
  hasActivePourRequest = false;
  clearMockPourTimer();
  refreshVisibleStatus();
}

std::uint16_t selectedAlcoholAmountMl() {
  switch (selectedStrengthIndex) {
    case 0:
      return 40;
    case 1:
      return 60;
    case 2:
      return 80;
    default:
      return 60;
  }
}

std::uint16_t estimatePourTimeSeconds() {
  return 0;
}

void setScreenVisibility(lv_obj_t* screen, bool visible) {
  if (screen == nullptr) {
    return;
  }

  if (visible) {
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(screen, LV_OBJ_FLAG_HIDDEN);
  }
}

void setObjectVisibility(lv_obj_t* object, bool visible) {
  if (object == nullptr) {
    return;
  }

  if (visible) {
    lv_obj_remove_flag(object, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(object, LV_OBJ_FLAG_HIDDEN);
  }
}

void showView(ScreenView view) {
  currentView = view;
  setScreenVisibility(bootScreen, view == ScreenView::Boot);
  setScreenVisibility(menuScreen, view == ScreenView::MainMenu);
  setScreenVisibility(detailScreen, view == ScreenView::DrinkDetail);
  setScreenVisibility(settingsScreen, view == ScreenView::Settings);
}

void refreshSettingsScreen();

void applyTabStyle(lv_obj_t* button, bool active) {
  if (button == nullptr) {
    return;
  }

  if (active) {
    lv_obj_set_style_bg_color(button, lv_color_hex(0xF4B662), 0);
    lv_obj_set_style_text_color(button, lv_color_hex(0x11151B), 0);
  } else {
    lv_obj_set_style_bg_color(button, lv_color_hex(0x222934), 0);
    lv_obj_set_style_text_color(button, lv_color_hex(0xF4F6F8), 0);
  }
}

void updateMenuHeader() {
  if (menuTitleLabel != nullptr) {
    lv_label_set_text(menuTitleLabel, "Tipsy");
  }

  if (menuStatusLabel != nullptr) {
    const String status = effectiveStatusText();
    lv_label_set_text(menuStatusLabel, status.c_str());
  }
}

lv_obj_t* createTopBackButton(lv_obj_t* parent, lv_event_cb_t callback) {
  lv_obj_t* button = lv_button_create(parent);
  lv_obj_set_size(button, 108, 46);
  lv_obj_set_style_radius(button, 20, 0);
  lv_obj_set_style_bg_color(button, lv_color_hex(0x202630), 0);
  lv_obj_set_style_border_width(button, 0, 0);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, nullptr);

  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, "Back");
  lv_obj_center(label);
  return button;
}

lv_obj_t* createTabButton(lv_obj_t* parent, const char* text, lv_event_cb_t callback) {
  lv_obj_t* button = lv_button_create(parent);
  lv_obj_set_height(button, 48);
  lv_obj_set_style_radius(button, 18, 0);
  lv_obj_set_style_border_width(button, 0, 0);
  lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, nullptr);

  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_center(label);
  return button;
}

lv_obj_t* createActionRow(lv_obj_t* parent, const char* title, const char* subtitle,
                          lv_event_cb_t callback) {
  lv_obj_t* row = lv_button_create(parent);
  lv_obj_set_width(row, LV_PCT(100));
  lv_obj_set_height(row, 92);
  lv_obj_set_style_radius(row, 24, 0);
  lv_obj_set_style_bg_color(row, lv_color_hex(0x151B23), 0);
  lv_obj_set_style_border_color(row, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_opa(row, LV_OPA_10, 0);
  lv_obj_set_style_border_width(row, 2, 0);
  lv_obj_set_style_pad_left(row, 18, 0);
  lv_obj_set_style_pad_right(row, 18, 0);
  lv_obj_set_style_pad_top(row, 14, 0);
  lv_obj_set_style_pad_bottom(row, 14, 0);
  lv_obj_set_style_pad_row(row, 4, 0);
  lv_obj_set_layout(row, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  if (callback != nullptr) {
    lv_obj_add_event_cb(row, callback, LV_EVENT_CLICKED, nullptr);
  }

  lv_obj_t* titleLabel = lv_label_create(row);
  lv_label_set_text(titleLabel, title);
  lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xF4F6F8), 0);

  lv_obj_t* subtitleLabel = lv_label_create(row);
  lv_label_set_text(subtitleLabel, subtitle);
  lv_obj_set_style_text_color(subtitleLabel, lv_color_hex(0x95A2B3), 0);
  return row;
}

void updateDetailScreen() {
  if (detailTitleLabel != nullptr) {
    lv_label_set_text(detailTitleLabel, selectedItem == nullptr ? "Select a drink"
                                                                : selectedItem->displayName.c_str());
  }

  if (detailStatusLabel != nullptr) {
    const String status = effectiveStatusText();
    lv_label_set_text(detailStatusLabel, status.c_str());
  }

  if (detailSubtitleLabel != nullptr) {
    lv_label_set_text(detailSubtitleLabel,
                      "Choose alcohol strength only. Mixers stay fixed.");
  }

  if (detailRecipeCard != nullptr) {
    lv_obj_add_flag(detailRecipeCard, LV_OBJ_FLAG_HIDDEN);
  }

  if (detailSummaryLabel != nullptr) {
    const bool isShot = selectedItem != nullptr && selectedItem->categoryId == "shot";
    String summary = "Alcohol: ";
    summary += strengthMlLabel(selectedStrengthIndex);
    summary += isShot ? " | Shot amount only" : " | Mixers stay fixed";
    lv_label_set_text(detailSummaryLabel, summary.c_str());
  }

  if (detailEstimatedTimeLabel != nullptr) {
    lv_label_set_text(detailEstimatedTimeLabel, "Duration driven by hardware.");
  }

  if (pourButton != nullptr) {
    const bool available = selectedItem != nullptr &&
                           (!backendKnowsDrink(selectedItem->id.c_str()) ||
                            backendDrinkAvailable(selectedItem->id.c_str()));
    lv_obj_set_style_bg_color(
        pourButton, available ? lv_color_hex(0xF4B662) : lv_color_hex(0x3A4048), 0);
    if (available) {
      lv_obj_clear_state(pourButton, LV_STATE_DISABLED);
    } else {
      lv_obj_add_state(pourButton, LV_STATE_DISABLED);
    }
  }

  for (std::uint8_t i = 0; i < 3; ++i) {
    if (strengthButtons[i] == nullptr) {
      continue;
    }

    const bool active = i == selectedStrengthIndex;
    lv_obj_set_style_bg_color(strengthButtons[i],
                              active ? lv_color_hex(0xF4B662) : lv_color_hex(0x222934), 0);
    lv_obj_set_style_text_color(strengthButtons[i],
                                active ? lv_color_hex(0x11151B) : lv_color_hex(0xF4F6F8), 0);

    if (strengthPrimaryLabels[i] != nullptr) {
      lv_label_set_text(strengthPrimaryLabels[i], strengthClLabel(i));
      lv_obj_set_style_text_color(strengthPrimaryLabels[i],
                                  active ? lv_color_hex(0x11151B) : lv_color_hex(0xF4F6F8), 0);
    }

    if (strengthSecondaryLabels[i] != nullptr) {
      lv_label_set_text(strengthSecondaryLabels[i], strengthMlLabel(i));
      lv_obj_set_style_text_color(strengthSecondaryLabels[i],
                                  active ? lv_color_hex(0x2E333B) : lv_color_hex(0x9AA7B7), 0);
    }
  }
}

void rebuildMenuGrid() {
  if (menuGrid == nullptr) {
    return;
  }

  lv_obj_clean(menuGrid);
  menuEmptyLabel = nullptr;

  std::size_t visibleCount = 0;
  for (std::size_t i = 0; i < currentModel.drinkCount && i < currentModel.drinks.size(); ++i) {
    const auto& item = currentModel.drinks[i];
    if (!itemMatchesCategory(item, activeCategory)) {
      continue;
    }

    if (!item.available) {
      continue;
    }

    ++visibleCount;
    const DrinkCategory itemCategory = categoryFromItem(item);

    lv_obj_t* card = lv_button_create(menuGrid);
    lv_obj_set_size(card, 290, 108);
    lv_obj_set_style_radius(card, 24, 0);
    lv_obj_set_style_bg_color(
        card, itemCategory == DrinkCategory::Shots ? lv_color_hex(0x2A2318) : lv_color_hex(0x222934), 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(card, LV_OPA_10, 0);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_pad_left(card, 16, 0);
    lv_obj_set_style_pad_right(card, 16, 0);
    lv_obj_set_style_pad_top(card, 14, 0);
    lv_obj_set_style_pad_bottom(card, 14, 0);
    lv_obj_set_style_pad_row(card, 4, 0);
    lv_obj_set_layout(card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_event_cb(
        card,
        [](lv_event_t* event) {
          selectedItem = static_cast<const UiRenderDrinkItem*>(lv_event_get_user_data(event));
          selectedStrengthIndex = 1;
          if (selectedItem != nullptr && selectedItem->available) {
            ui_trigger_select_drink(selectedItem->id.c_str());
          }
          updateDetailScreen();
          showView(ScreenView::DrinkDetail);
        },
        LV_EVENT_CLICKED, &currentModel.drinks[i]);

    lv_obj_t* nameLabel = lv_label_create(card);
    lv_label_set_text(nameLabel, item.displayName.c_str());
    lv_obj_set_style_text_color(
        nameLabel, itemCategory == DrinkCategory::Shots ? lv_color_hex(0xF6C47A) : lv_color_hex(0xF4F6F8), 0);

    lv_obj_t* subtitleLabel = lv_label_create(card);
    lv_label_set_text(subtitleLabel, item.subtitle.isEmpty() ? item.availabilityText.c_str()
                                                             : item.subtitle.c_str());
    lv_obj_set_style_text_color(
        subtitleLabel, itemCategory == DrinkCategory::Shots ? lv_color_hex(0xB58E59) : lv_color_hex(0x95A2B3), 0);
  }

  if (visibleCount == 0) {
    menuEmptyLabel = lv_label_create(menuGrid);
    lv_label_set_text(menuEmptyLabel, "No available items for this category");
    lv_obj_set_style_text_color(menuEmptyLabel, lv_color_hex(0x95A2B3), 0);
  }
}

void refreshMenuScreen() {
  updateMenuHeader();
  applyTabStyle(allTabButton, activeCategory == DrinkCategory::All);
  applyTabStyle(drinksTabButton, activeCategory == DrinkCategory::Drinks);
  applyTabStyle(shotsTabButton, activeCategory == DrinkCategory::Shots);
  rebuildMenuGrid();
}

void refreshVisibleStatus() {
  refreshMenuScreen();
  updateDetailScreen();
  refreshSettingsScreen();
}

void handleBootTimer(lv_timer_t* timer) {
  (void)timer;
  if (bootTimer != nullptr) {
    lv_timer_delete(bootTimer);
    bootTimer = nullptr;
  }
  showView(ScreenView::MainMenu);
}

void animateBootLogo() {
  if (bootLogoLabel == nullptr || bootGlowLine == nullptr) {
    return;
  }

  lv_anim_t fadeAnim;
  lv_anim_init(&fadeAnim);
  lv_anim_set_var(&fadeAnim, bootLogoLabel);
  lv_anim_set_values(&fadeAnim, 0, LV_OPA_COVER);
  lv_anim_set_time(&fadeAnim, 650);
  lv_anim_set_exec_cb(&fadeAnim,
                      [](void* obj, int32_t value) {
                        lv_obj_set_style_opa(static_cast<lv_obj_t*>(obj), static_cast<lv_opa_t>(value), 0);
                      });
  lv_anim_start(&fadeAnim);

  lv_anim_t riseAnim;
  lv_anim_init(&riseAnim);
  lv_anim_set_var(&riseAnim, bootLogoLabel);
  lv_anim_set_values(&riseAnim, 24, 0);
  lv_anim_set_time(&riseAnim, 650);
  lv_anim_set_exec_cb(&riseAnim,
                      [](void* obj, int32_t value) { lv_obj_set_y(static_cast<lv_obj_t*>(obj), value); });
  lv_anim_start(&riseAnim);

  lv_anim_t lineAnim;
  lv_anim_init(&lineAnim);
  lv_anim_set_var(&lineAnim, bootGlowLine);
  lv_anim_set_values(&lineAnim, 0, 180);
  lv_anim_set_time(&lineAnim, 700);
  lv_anim_set_exec_cb(&lineAnim,
                      [](void* obj, int32_t value) { lv_obj_set_width(static_cast<lv_obj_t*>(obj), value); });
  lv_anim_start(&lineAnim);
}

void handleSettingsButton(lv_event_t* event) {
  (void)event;
  ui_trigger_admin_opened();
  showView(ScreenView::Settings);
}

void handleBackToMenu(lv_event_t* event) {
  (void)event;
  showView(ScreenView::MainMenu);
}

void handleSettingsBack(lv_event_t* event) {
  (void)event;
  if (currentSettingsSubview != SettingsSubview::Main) {
    currentSettingsSubview = SettingsSubview::Main;
    refreshSettingsScreen();
    return;
  }

  showView(ScreenView::MainMenu);
}

void handleFilterAll(lv_event_t* event) {
  (void)event;
  activeCategory = DrinkCategory::All;
  refreshMenuScreen();
}

void handleFilterDrinks(lv_event_t* event) {
  (void)event;
  activeCategory = DrinkCategory::Drinks;
  refreshMenuScreen();
}

void handleFilterShots(lv_event_t* event) {
  (void)event;
  activeCategory = DrinkCategory::Shots;
  refreshMenuScreen();
}

void handleStrengthSelect(lv_event_t* event) {
  selectedStrengthIndex = static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(
      lv_event_get_user_data(event)));
  updateDetailScreen();
}

void handlePourAction(lv_event_t* event) {
  (void)event;
  if (selectedItem == nullptr || selectedItem->id.isEmpty()) {
    return;
  }

  // The backend owns availability, start status, and pour timing.
  // We simply pass the action down through the generated UI bridge.
  ui_trigger_select_drink(selectedItem->id.c_str());
  ui_trigger_start_selected_drink(selectedAlcoholAmountMl());
}

void openSettingsSubview(SettingsSubview subview) {
  currentSettingsSubview = subview;
  refreshSettingsScreen();
}

void handleOpenPumpMapping(lv_event_t* event) {
  (void)event;
  openSettingsSubview(SettingsSubview::PumpMapping);
}

void handleOpenCalibration(lv_event_t* event) {
  (void)event;
  openSettingsSubview(SettingsSubview::Calibration);
}

void handleOpenServiceMode(lv_event_t* event) {
  (void)event;
  openSettingsSubview(SettingsSubview::ServiceMode);
}

void handlePrimePumps(lv_event_t* event) {
  (void)event;
  settingsStatusMessage = "";
  ui_trigger_prime_pumps();
  refreshSettingsScreen();
}

void handleFlushCleaning(lv_event_t* event) {
  (void)event;
  settingsStatusMessage = "";
  ui_trigger_flush_cleaning();
  refreshSettingsScreen();
}

void handleAdminLockChanged(lv_event_t* event) {
  adminLockEnabled = lv_obj_has_state(static_cast<lv_obj_t*>(lv_event_get_target(event)),
                                      LV_STATE_CHECKED);
  settingsDirty = true;
  settingsStatusMessage = adminLockEnabled ? "Admin Lock enabled." : "Admin Lock disabled.";
  refreshSettingsScreen();
}

void handleSaveSettings(lv_event_t* event) {
  (void)event;
  settingsDirty = false;
  settingsStatusMessage = "Settings saved in mock mode.";
  refreshSettingsScreen();
}

void handleCyclePumpAssignment(lv_event_t* event) {
  const std::size_t pumpIndex = static_cast<std::size_t>(
      reinterpret_cast<std::uintptr_t>(lv_event_get_user_data(event)));
  if (pumpIndex >= currentModel.pumpAssignments.size()) {
    return;
  }

  ui_trigger_pump_assignment_edited(static_cast<std::uint8_t>(pumpIndex), "__CYCLE__",
                                    "", false);
  settingsDirty = false;
  settingsStatusMessage = "Pump assignment updated.";
  refreshSettingsScreen();
}

void updateSettingsDetailContent() {
  if (settingsDetailTitleLabel == nullptr || settingsDetailBodyLabel == nullptr) {
    return;
  }

  switch (currentSettingsSubview) {
    case SettingsSubview::PumpMapping:
      lv_label_set_text(settingsDetailTitleLabel, "Pump Mapping");
      lv_label_set_text(
          settingsDetailBodyLabel,
          "Tap a pump row to cycle ingredient assignment.\n\nOnly mapped ingredients appear on the main menu.\nPersistence can plug in later.");
      break;
    case SettingsSubview::Calibration:
      lv_label_set_text(settingsDetailTitleLabel, "Calibration");
      lv_label_set_text(
          settingsDetailBodyLabel,
          "Placeholder calibration view.\n\nFlow: 10-15 ml/sec\nPrime: 1000 ms\nSettle: 250 ms\n\nUsed for flow tuning and pump timing.");
      break;
    case SettingsSubview::ServiceMode:
      lv_label_set_text(settingsDetailTitleLabel, "Service Mode");
      lv_label_set_text(
          settingsDetailBodyLabel,
          "Placeholder diagnostics view.\n\n- Pump test\n- Storage status\n- Runtime info\n- Maintenance actions\n\nSafe mock mode only.");
      break;
    case SettingsSubview::Main:
    default:
      lv_label_set_text(settingsDetailTitleLabel, "");
      lv_label_set_text(settingsDetailBodyLabel, "");
      break;
  }
}

void refreshPumpMappingList() {
  if (pumpMappingList == nullptr) {
    return;
  }

  for (std::size_t i = 0; i < currentModel.pumpAssignments.size(); ++i) {
    if (pumpMappingValueLabels[i] == nullptr) {
      continue;
    }

    String rowText = "Pump ";
    rowText += (i + 1);
    rowText += "  ";
    rowText += currentModel.pumpAssignments[i].ingredientDisplayName.isEmpty()
                   ? String("Unassigned")
                   : currentModel.pumpAssignments[i].ingredientDisplayName;
    lv_label_set_text(pumpMappingValueLabels[i], rowText.c_str());
  }
}

void refreshSettingsScreen() {
  if (settingsSubtitleLabel != nullptr) {
    lv_label_set_text(settingsSubtitleLabel,
                      currentSettingsSubview == SettingsSubview::Main ? "Machine configuration"
                                                                      : "Settings detail");
  }

  if (settingsStatusLabel != nullptr) {
    const String status = settingsStatusMessage.isEmpty() ? effectiveStatusText()
                                                          : settingsStatusMessage;
    lv_label_set_text(settingsStatusLabel, status.c_str());
  }

  if (adminLockSwitch != nullptr) {
    if (adminLockEnabled) {
      lv_obj_add_state(adminLockSwitch, LV_STATE_CHECKED);
    } else {
      lv_obj_remove_state(adminLockSwitch, LV_STATE_CHECKED);
    }
  }

  const bool inMain = currentSettingsSubview == SettingsSubview::Main;
  const bool inPumpMapping = currentSettingsSubview == SettingsSubview::PumpMapping;
  const bool hasEditableControls = inMain || inPumpMapping;
  setObjectVisibility(settingsMainList, inMain);
  setObjectVisibility(settingsDetailCard, !inMain);
  setObjectVisibility(pumpMappingList, inPumpMapping);
  setObjectVisibility(settingsSaveButton, hasEditableControls && settingsDirty);
  updateSettingsDetailContent();
  refreshPumpMappingList();
}

void buildBootScreen() {
  bootScreen = lv_obj_create(rootScreen);
  lv_obj_set_size(bootScreen, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(bootScreen, lv_color_hex(0x101418), 0);
  lv_obj_set_style_bg_opa(bootScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(bootScreen, 0, 0);
  lv_obj_set_style_pad_all(bootScreen, 0, 0);

  bootLogoLabel = lv_label_create(bootScreen);
  lv_label_set_text(bootLogoLabel, "Tipsy");
  lv_obj_set_style_text_color(bootLogoLabel, lv_color_hex(0xF4F6F8), 0);
  lv_obj_set_style_text_letter_space(bootLogoLabel, 2, 0);
  lv_obj_align(bootLogoLabel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_opa(bootLogoLabel, 0, 0);

  bootGlowLine = lv_obj_create(bootScreen);
  lv_obj_set_height(bootGlowLine, 6);
  lv_obj_set_width(bootGlowLine, 0);
  lv_obj_set_style_radius(bootGlowLine, 3, 0);
  lv_obj_set_style_bg_color(bootGlowLine, lv_color_hex(0xF4B662), 0);
  lv_obj_set_style_border_width(bootGlowLine, 0, 0);
  lv_obj_align_to(bootGlowLine, bootLogoLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 22);
}

void buildMenuScreen() {
  menuScreen = lv_obj_create(rootScreen);
  lv_obj_set_size(menuScreen, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(menuScreen, lv_color_hex(0x101418), 0);
  lv_obj_set_style_bg_opa(menuScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(menuScreen, 0, 0);
  lv_obj_set_style_pad_left(menuScreen, 18, 0);
  lv_obj_set_style_pad_right(menuScreen, 18, 0);
  lv_obj_set_style_pad_top(menuScreen, 16, 0);
  lv_obj_set_style_pad_bottom(menuScreen, 16, 0);

  menuTitleLabel = lv_label_create(menuScreen);
  lv_obj_set_style_text_color(menuTitleLabel, lv_color_hex(0xF4F6F8), 0);
  lv_obj_align(menuTitleLabel, LV_ALIGN_TOP_LEFT, 0, 0);

  menuStatusLabel = lv_label_create(menuScreen);
  lv_obj_set_style_text_color(menuStatusLabel, lv_color_hex(0xA6B0BF), 0);
  lv_obj_align_to(menuStatusLabel, menuTitleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

  lv_obj_t* tabsRow = lv_obj_create(menuScreen);
  lv_obj_set_size(tabsRow, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(tabsRow, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(tabsRow, 0, 0);
  lv_obj_set_style_pad_all(tabsRow, 0, 0);
  lv_obj_set_style_pad_column(tabsRow, 10, 0);
  lv_obj_set_layout(tabsRow, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(tabsRow, LV_FLEX_FLOW_ROW);
  lv_obj_align_to(tabsRow, menuStatusLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  allTabButton = createTabButton(tabsRow, "All", handleFilterAll);
  lv_obj_set_width(allTabButton, 92);
  drinksTabButton = createTabButton(tabsRow, "Drinks", handleFilterDrinks);
  lv_obj_set_width(drinksTabButton, 120);
  shotsTabButton = createTabButton(tabsRow, "Shots", handleFilterShots);
  lv_obj_set_width(shotsTabButton, 110);

  menuGrid = lv_obj_create(menuScreen);
  lv_obj_set_size(menuGrid, LV_PCT(100), 420);
  lv_obj_set_style_bg_color(menuGrid, lv_color_hex(0x12171F), 0);
  lv_obj_set_style_bg_opa(menuGrid, LV_OPA_60, 0);
  lv_obj_set_style_radius(menuGrid, 24, 0);
  lv_obj_set_style_border_width(menuGrid, 0, 0);
  lv_obj_set_style_pad_all(menuGrid, 12, 0);
  lv_obj_set_style_pad_row(menuGrid, 10, 0);
  lv_obj_set_style_pad_column(menuGrid, 10, 0);
  lv_obj_set_scrollbar_mode(menuGrid, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_layout(menuGrid, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(menuGrid, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(menuGrid, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);
  lv_obj_align_to(menuGrid, tabsRow, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

  settingsButton = lv_button_create(menuScreen);
  lv_obj_set_size(settingsButton, LV_PCT(100), 68);
  lv_obj_set_style_radius(settingsButton, 26, 0);
  lv_obj_set_style_bg_color(settingsButton, lv_color_hex(0x202630), 0);
  lv_obj_set_style_border_width(settingsButton, 0, 0);
  lv_obj_add_event_cb(settingsButton, handleSettingsButton, LV_EVENT_CLICKED, nullptr);
  lv_obj_align(settingsButton, LV_ALIGN_BOTTOM_MID, 0, 0);

  lv_obj_t* settingsLabel = lv_label_create(settingsButton);
  lv_label_set_text(settingsLabel, "Settings");
  lv_obj_center(settingsLabel);
}

void buildDetailScreen() {
  detailScreen = lv_obj_create(rootScreen);
  lv_obj_set_size(detailScreen, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(detailScreen, lv_color_hex(0x101418), 0);
  lv_obj_set_style_bg_opa(detailScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(detailScreen, 0, 0);
  lv_obj_set_style_pad_left(detailScreen, 18, 0);
  lv_obj_set_style_pad_right(detailScreen, 18, 0);
  lv_obj_set_style_pad_top(detailScreen, 16, 0);
  lv_obj_set_style_pad_bottom(detailScreen, 16, 0);

  detailBackButton = createTopBackButton(detailScreen, handleBackToMenu);
  lv_obj_align(detailBackButton, LV_ALIGN_TOP_LEFT, 0, 0);

  detailTitleLabel = lv_label_create(detailScreen);
  lv_obj_set_style_text_color(detailTitleLabel, lv_color_hex(0xF4F6F8), 0);
  lv_obj_align_to(detailTitleLabel, detailBackButton, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  detailStatusLabel = lv_label_create(detailScreen);
  lv_obj_set_style_text_color(detailStatusLabel, lv_color_hex(0xA6B0BF), 0);
  lv_obj_align_to(detailStatusLabel, detailTitleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

  detailSubtitleLabel = lv_label_create(detailScreen);
  lv_label_set_long_mode(detailSubtitleLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(detailSubtitleLabel, LV_PCT(100));
  lv_obj_set_style_text_color(detailSubtitleLabel, lv_color_hex(0x95A2B3), 0);
  lv_obj_align_to(detailSubtitleLabel, detailStatusLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  detailRecipeCard = lv_obj_create(detailScreen);
  lv_obj_set_size(detailRecipeCard, LV_PCT(100), 150);
  lv_obj_set_style_bg_color(detailRecipeCard, lv_color_hex(0x141A22), 0);
  lv_obj_set_style_bg_opa(detailRecipeCard, LV_OPA_80, 0);
  lv_obj_set_style_radius(detailRecipeCard, 24, 0);
  lv_obj_set_style_border_width(detailRecipeCard, 0, 0);
  lv_obj_set_style_pad_all(detailRecipeCard, 14, 0);
  lv_obj_set_style_pad_row(detailRecipeCard, 6, 0);
  lv_obj_set_layout(detailRecipeCard, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(detailRecipeCard, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(detailRecipeCard, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);
  lv_obj_align_to(detailRecipeCard, detailSubtitleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  detailRecipeTitleLabel = lv_label_create(detailRecipeCard);
  lv_obj_set_style_text_color(detailRecipeTitleLabel, lv_color_hex(0xF4F6F8), 0);

  detailRecipeAlcoholLabel = lv_label_create(detailRecipeCard);
  lv_obj_set_style_text_color(detailRecipeAlcoholLabel, lv_color_hex(0xAAB5C3), 0);

  for (std::size_t i = 0; i < 3; ++i) {
    detailRecipeMixerLabels[i] = lv_label_create(detailRecipeCard);
    lv_obj_set_style_text_color(detailRecipeMixerLabels[i], lv_color_hex(0x95A2B3), 0);
  }

  lv_obj_t* selectorCard = lv_obj_create(detailScreen);
  lv_obj_set_size(selectorCard, LV_PCT(100), 260);
  lv_obj_set_style_bg_color(selectorCard, lv_color_hex(0x141A22), 0);
  lv_obj_set_style_bg_opa(selectorCard, LV_OPA_80, 0);
  lv_obj_set_style_radius(selectorCard, 28, 0);
  lv_obj_set_style_border_width(selectorCard, 0, 0);
  lv_obj_set_style_pad_all(selectorCard, 16, 0);
  lv_obj_align_to(selectorCard, detailRecipeCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

  lv_obj_t* selectorTitle = lv_label_create(selectorCard);
  lv_label_set_text(selectorTitle, "Alcohol Strength");
  lv_obj_set_style_text_color(selectorTitle, lv_color_hex(0xF4F6F8), 0);
  lv_obj_align(selectorTitle, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t* selectorHelp = lv_label_create(selectorCard);
  lv_label_set_text(selectorHelp, "Only alcohol amount changes");
  lv_obj_set_style_text_color(selectorHelp, lv_color_hex(0x95A2B3), 0);
  lv_obj_align_to(selectorHelp, selectorTitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

  for (std::uint8_t i = 0; i < 3; ++i) {
    lv_obj_t* button = lv_button_create(selectorCard);
    lv_obj_set_size(button, LV_PCT(100), 52);
    lv_obj_set_style_radius(button, 22, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_add_event_cb(button, handleStrengthSelect, LV_EVENT_CLICKED,
                        reinterpret_cast<void*>(static_cast<std::uintptr_t>(i)));
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 0, 54 + (i * 62));
    strengthButtons[i] = button;

    strengthPrimaryLabels[i] = lv_label_create(button);
    lv_obj_align(strengthPrimaryLabels[i], LV_ALIGN_LEFT_MID, 18, 0);

    strengthSecondaryLabels[i] = lv_label_create(button);
    lv_obj_align(strengthSecondaryLabels[i], LV_ALIGN_RIGHT_MID, -18, 0);
  }

  detailSummaryLabel = lv_label_create(detailScreen);
  lv_label_set_long_mode(detailSummaryLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(detailSummaryLabel, LV_PCT(100));
  lv_obj_set_style_text_color(detailSummaryLabel, lv_color_hex(0xAAB5C3), 0);
  lv_obj_align_to(detailSummaryLabel, selectorCard, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  detailEstimatedTimeLabel = lv_label_create(detailScreen);
  lv_obj_set_style_text_color(detailEstimatedTimeLabel, lv_color_hex(0x95A2B3), 0);
  lv_obj_align_to(detailEstimatedTimeLabel, detailSummaryLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  pourButton = lv_button_create(detailScreen);
  lv_obj_set_size(pourButton, LV_PCT(100), 72);
  lv_obj_set_style_radius(pourButton, 28, 0);
  lv_obj_set_style_bg_color(pourButton, lv_color_hex(0xF4B662), 0);
  lv_obj_set_style_border_width(pourButton, 0, 0);
  lv_obj_add_event_cb(pourButton, handlePourAction, LV_EVENT_CLICKED, nullptr);
  lv_obj_align(pourButton, LV_ALIGN_BOTTOM_MID, 0, 0);

  lv_obj_t* pourLabel = lv_label_create(pourButton);
  lv_label_set_text(pourLabel, "Pour");
  lv_obj_set_style_text_color(pourLabel, lv_color_hex(0x11151B), 0);
  lv_obj_center(pourLabel);
}

void buildSettingsScreen() {
  settingsScreen = lv_obj_create(rootScreen);
  lv_obj_set_size(settingsScreen, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_bg_color(settingsScreen, lv_color_hex(0x101418), 0);
  lv_obj_set_style_bg_opa(settingsScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(settingsScreen, 0, 0);
  lv_obj_set_style_pad_left(settingsScreen, 18, 0);
  lv_obj_set_style_pad_right(settingsScreen, 18, 0);
  lv_obj_set_style_pad_top(settingsScreen, 16, 0);
  lv_obj_set_style_pad_bottom(settingsScreen, 16, 0);

  settingsBackButton = createTopBackButton(settingsScreen, handleSettingsBack);
  lv_obj_align(settingsBackButton, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t* title = lv_label_create(settingsScreen);
  lv_label_set_text(title, "Settings");
  lv_obj_set_style_text_color(title, lv_color_hex(0xF4F6F8), 0);
  lv_obj_align_to(title, settingsBackButton, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  settingsSubtitleLabel = lv_label_create(settingsScreen);
  lv_label_set_text(settingsSubtitleLabel, "Machine configuration");
  lv_obj_set_style_text_color(settingsSubtitleLabel, lv_color_hex(0x95A2B3), 0);
  lv_obj_align_to(settingsSubtitleLabel, title, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

  settingsStatusLabel = lv_label_create(settingsScreen);
  lv_label_set_text(settingsStatusLabel, "Ready");
  lv_obj_set_style_text_color(settingsStatusLabel, lv_color_hex(0xA6B0BF), 0);
  lv_obj_align_to(settingsStatusLabel, settingsSubtitleLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

  settingsMainList = lv_obj_create(settingsScreen);
  lv_obj_set_size(settingsMainList, LV_PCT(100), 520);
  lv_obj_set_style_bg_opa(settingsMainList, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(settingsMainList, 0, 0);
  lv_obj_set_style_pad_all(settingsMainList, 0, 0);
  lv_obj_set_style_pad_row(settingsMainList, 10, 0);
  lv_obj_set_layout(settingsMainList, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(settingsMainList, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(settingsMainList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);
  lv_obj_align_to(settingsMainList, settingsStatusLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  createActionRow(settingsMainList, "Pump Mapping", "Assign ingredients to pump channels",
                  handleOpenPumpMapping);
  createActionRow(settingsMainList, "Calibration", "Flow tuning and pump timing",
                  handleOpenCalibration);
  createActionRow(settingsMainList, "Prime Pumps",
                  "Bring liquid to the start of the hose", handlePrimePumps);
  createActionRow(settingsMainList, "Flush / Cleaning",
                  "Run a safe cleaning cycle in mock mode", handleFlushCleaning);

  lv_obj_t* adminRow = lv_obj_create(settingsMainList);
  lv_obj_set_width(adminRow, LV_PCT(100));
  lv_obj_set_height(adminRow, 92);
  lv_obj_set_style_radius(adminRow, 24, 0);
  lv_obj_set_style_bg_color(adminRow, lv_color_hex(0x151B23), 0);
  lv_obj_set_style_border_color(adminRow, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_border_opa(adminRow, LV_OPA_10, 0);
  lv_obj_set_style_border_width(adminRow, 2, 0);
  lv_obj_set_style_pad_left(adminRow, 18, 0);
  lv_obj_set_style_pad_right(adminRow, 18, 0);
  lv_obj_set_style_pad_top(adminRow, 14, 0);
  lv_obj_set_style_pad_bottom(adminRow, 14, 0);

  lv_obj_t* adminTitle = lv_label_create(adminRow);
  lv_label_set_text(adminTitle, "Admin Lock");
  lv_obj_set_style_text_color(adminTitle, lv_color_hex(0xF4F6F8), 0);
  lv_obj_align(adminTitle, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t* adminSubtitle = lv_label_create(adminRow);
  lv_label_set_text(adminSubtitle, "Protect settings access");
  lv_obj_set_style_text_color(adminSubtitle, lv_color_hex(0x95A2B3), 0);
  lv_obj_align_to(adminSubtitle, adminTitle, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 6);

  adminLockSwitch = lv_switch_create(adminRow);
  lv_obj_align(adminLockSwitch, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_add_event_cb(adminLockSwitch, handleAdminLockChanged, LV_EVENT_VALUE_CHANGED, nullptr);

  createActionRow(settingsMainList, "Service Mode", "Diagnostics and maintenance",
                  handleOpenServiceMode);

  settingsDetailCard = lv_obj_create(settingsScreen);
  lv_obj_set_size(settingsDetailCard, LV_PCT(100), 360);
  lv_obj_set_style_bg_color(settingsDetailCard, lv_color_hex(0x141A22), 0);
  lv_obj_set_style_bg_opa(settingsDetailCard, LV_OPA_80, 0);
  lv_obj_set_style_radius(settingsDetailCard, 26, 0);
  lv_obj_set_style_border_width(settingsDetailCard, 0, 0);
  lv_obj_set_style_pad_all(settingsDetailCard, 18, 0);
  lv_obj_set_style_pad_row(settingsDetailCard, 12, 0);
  lv_obj_set_layout(settingsDetailCard, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(settingsDetailCard, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(settingsDetailCard, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);
  lv_obj_align_to(settingsDetailCard, settingsStatusLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

  settingsDetailTitleLabel = lv_label_create(settingsDetailCard);
  lv_obj_set_style_text_color(settingsDetailTitleLabel, lv_color_hex(0xF4F6F8), 0);

  settingsDetailBodyLabel = lv_label_create(settingsDetailCard);
  lv_label_set_long_mode(settingsDetailBodyLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(settingsDetailBodyLabel, LV_PCT(100));
  lv_obj_set_style_text_color(settingsDetailBodyLabel, lv_color_hex(0x95A2B3), 0);

  pumpMappingList = lv_obj_create(settingsDetailCard);
  lv_obj_set_width(pumpMappingList, LV_PCT(100));
  lv_obj_set_height(pumpMappingList, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(pumpMappingList, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(pumpMappingList, 0, 0);
  lv_obj_set_style_pad_all(pumpMappingList, 0, 0);
  lv_obj_set_style_pad_row(pumpMappingList, 10, 0);
  lv_obj_set_layout(pumpMappingList, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(pumpMappingList, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(pumpMappingList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_START);

  for (std::size_t i = 0; i < currentModel.pumpAssignments.size(); ++i) {
    lv_obj_t* row = lv_button_create(pumpMappingList);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, 58);
    lv_obj_set_style_radius(row, 20, 0);
    lv_obj_set_style_bg_color(row, lv_color_hex(0x1A212C), 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_add_event_cb(row, handleCyclePumpAssignment, LV_EVENT_CLICKED,
                        reinterpret_cast<void*>(static_cast<std::uintptr_t>(i)));

    pumpMappingValueLabels[i] = lv_label_create(row);
    lv_obj_set_style_text_color(pumpMappingValueLabels[i], lv_color_hex(0xF4F6F8), 0);
    lv_obj_align(pumpMappingValueLabels[i], LV_ALIGN_LEFT_MID, 16, 0);
  }

  settingsSaveButton = lv_button_create(settingsScreen);
  lv_obj_set_size(settingsSaveButton, LV_PCT(100), 68);
  lv_obj_set_style_radius(settingsSaveButton, 26, 0);
  lv_obj_set_style_bg_color(settingsSaveButton, lv_color_hex(0xF4B662), 0);
  lv_obj_set_style_border_width(settingsSaveButton, 0, 0);
  lv_obj_add_event_cb(settingsSaveButton, handleSaveSettings, LV_EVENT_CLICKED, nullptr);
  lv_obj_align(settingsSaveButton, LV_ALIGN_BOTTOM_MID, 0, 0);

  lv_obj_t* saveLabel = lv_label_create(settingsSaveButton);
  lv_label_set_text(saveLabel, "Save Settings");
  lv_obj_set_style_text_color(saveLabel, lv_color_hex(0x11151B), 0);
  lv_obj_center(saveLabel);

  refreshSettingsScreen();
}

void buildUiSkeleton() {
  rootScreen = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(rootScreen, lv_color_hex(0x101418), 0);
  lv_obj_set_style_bg_opa(rootScreen, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(rootScreen, 0, 0);
  lv_obj_set_style_pad_all(rootScreen, 0, 0);

  buildBootScreen();
  buildMenuScreen();
  buildDetailScreen();
  buildSettingsScreen();

  previewModeBadge = lv_label_create(rootScreen);
  lv_label_set_text(previewModeBadge, "Preview Mode");
  lv_obj_set_style_text_color(previewModeBadge, lv_color_hex(0x95A2B3), 0);
  lv_obj_set_style_bg_color(previewModeBadge, lv_color_hex(0x202630), 0);
  lv_obj_set_style_bg_opa(previewModeBadge, LV_OPA_80, 0);
  lv_obj_set_style_radius(previewModeBadge, 12, 0);
  lv_obj_set_style_pad_left(previewModeBadge, 10, 0);
  lv_obj_set_style_pad_right(previewModeBadge, 10, 0);
  lv_obj_set_style_pad_top(previewModeBadge, 4, 0);
  lv_obj_set_style_pad_bottom(previewModeBadge, 4, 0);
  lv_obj_align(previewModeBadge, LV_ALIGN_TOP_RIGHT, -14, 16);
  setObjectVisibility(previewModeBadge, previewModeEnabled);
}

void initializeUi(bool startInPreviewMenu) {
  currentModel = UiRenderModel {};
  currentModel.headerTitle = "Tipsy";
  currentModel.statusText = "Ready";
  currentModel.primaryActionLabel = "Pour";
  currentModel.headerSubtitle = startInPreviewMenu ? "Preview Mode" : "";

  activeCategory = DrinkCategory::All;
  selectedItem = nullptr;
  selectedStrengthIndex = 1;
  currentSettingsSubview = SettingsSubview::Main;
  settingsDirty = false;
  adminLockEnabled = false;
  settingsStatusMessage = "";
  mockPourStage = MockPourStage::Idle;
  transientStatusText = "";
  hasActivePourRequest = false;
  previewModeEnabled = startInPreviewMenu;
  clearBootTimer();
  clearMockPourTimer();

  buildUiSkeleton();
  refreshMenuScreen();
  updateDetailScreen();
  lv_screen_load(rootScreen);

  if (startInPreviewMenu) {
    showView(ScreenView::MainMenu);
  } else {
    showView(ScreenView::Boot);
    animateBootLogo();
    bootTimer = lv_timer_create(handleBootTimer, 1200, nullptr);
  }
}

}  // namespace

void ui_init() {
  initializeUi(false);
}

void ui_init_preview_main_menu() {
  initializeUi(true);
}

void ui_apply_model(const UiRenderModel& model) {
  currentModel = model;
  refreshMenuScreen();
  updateDetailScreen();
  refreshSettingsScreen();
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

void ui_bind_admin_opened(AdminOpenedCallback callback) {
  adminOpenedCallback = callback;
}

void ui_bind_prime_pumps(PrimePumpsCallback callback) {
  primePumpsCallback = callback;
}

void ui_bind_flush_cleaning(FlushCleaningCallback callback) {
  flushCleaningCallback = callback;
}

void ui_bind_pump_assignment_edited(PumpAssignmentEditedCallback callback) {
  pumpAssignmentEditedCallback = callback;
}

void ui_trigger_select_drink(const char* drinkId) {
  if (drinkSelectedCallback != nullptr && drinkId != nullptr) {
    drinkSelectedCallback(drinkId);
  }
}

void ui_trigger_start_selected_drink(std::uint16_t alcoholAmountMl) {
  if (startSelectedDrinkCallback != nullptr) {
    startSelectedDrinkCallback(alcoholAmountMl);
  }
}

void ui_trigger_admin_opened() {
  if (adminOpenedCallback != nullptr) {
    adminOpenedCallback();
  }
}

void ui_trigger_prime_pumps() {
  if (primePumpsCallback != nullptr) {
    primePumpsCallback();
  }
}

void ui_trigger_flush_cleaning() {
  if (flushCleaningCallback != nullptr) {
    flushCleaningCallback();
  }
}

void ui_trigger_pump_assignment_edited(std::uint8_t pumpIndex, const char* ingredientId,
                                       const char* ingredientDisplayName, bool enabled) {
  if (pumpAssignmentEditedCallback != nullptr && ingredientId != nullptr &&
      ingredientDisplayName != nullptr) {
    pumpAssignmentEditedCallback(pumpIndex, ingredientId, ingredientDisplayName, enabled);
  }
}

String ui_debug_screen_text() {
  String text;
  text.reserve(768);

  text += "View: ";
  switch (currentView) {
    case ScreenView::Boot:
      text += "Boot";
      break;
    case ScreenView::MainMenu:
      text += "MainMenu";
      break;
    case ScreenView::DrinkDetail:
      text += "DrinkDetail";
      break;
    case ScreenView::Settings:
      text += "Settings";
      break;
  }
  text += "\n";
  text += "Status: ";
  text += effectiveStatusText();
  text += "\n";

  if (selectedItem != nullptr) {
    text += "Selected item: ";
    text += selectedItem->displayName;
    text += "\n";
    text += "Alcohol strength: ";
    text += strengthClLabel(selectedStrengthIndex);
    text += " / ";
    text += strengthMlLabel(selectedStrengthIndex);
    text += "\n";
  }

  if (hasActivePourRequest) {
    text += "Pour request: ";
    text += activePourRequest.itemName;
    text += activePourRequest.isShot ? " [shot]" : " [drink]";
    text += "\n";
    text += "Alcohol: ";
    text += activePourRequest.alcoholAmountMl;
    text += " ml\n";
    text += "Estimated: ";
    text += activePourRequest.estimatedPourTimeSec;
    text += " sec\n";
  }

  text += "Category: ";
  switch (activeCategory) {
    case DrinkCategory::All:
      text += "All";
      break;
    case DrinkCategory::Drinks:
      text += "Drinks";
      break;
    case DrinkCategory::Shots:
      text += "Shots";
      break;
  }
  text += "\n";

  return text;
}

}  // namespace tipsy::ui::generated
