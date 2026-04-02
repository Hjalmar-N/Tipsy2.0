#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include <array>
#include <cstddef>
#include <cstdint>

#include <lvgl.h>
#include <src/drivers/windows/lv_windows_display.h>
#include <src/drivers/windows/lv_windows_input.h>

#include "config/AppConfig.h"
#include "ui/generated/ui.h"

namespace tipsy::desktop_preview {

namespace {

enum class PreviewCategory : std::uint8_t {
  Drink = 0,
  Shot,
};

struct PreviewRecipeIngredient {
  const char* ingredientId;
  bool isAlcohol;
};

struct PreviewRecipe {
  const char* id;
  const char* displayName;
  const char* subtitle;
  const char* categoryId;
  PreviewCategory category;
  std::array<PreviewRecipeIngredient, 4> ingredients {};
  std::size_t ingredientCount = 0;
};

struct PendingStatus {
  String completeMessage;
  std::uint32_t readyDelayMs = 1200;
};

constexpr std::array<const char*, 13> kIngredientOptions = {
    "",           "gin",         "vodka",       "rum",      "tequila",
    "whiskey",    "tonic",       "cola",        "soda",     "mint_mix",
    "lime",       "sour_mix",    "citrus_mix",
};

constexpr std::array<const char*, tipsy::config::kPumpCount> kDefaultPumpAssignments = {
    "gin", "tonic", "vodka", "whiskey", "soda", "tequila"};

constexpr std::array<PreviewRecipe, 9> kRecipes = {{
    {"mojito", "Mojito", "Fresh mint rum", "signature", PreviewCategory::Drink,
     {{{"rum", true}, {"mint_mix", false}, {"soda", false}, {"lime", false}}}, 4},
    {"gin_tonic", "Gin Tonic", "Classic crisp serve", "highball", PreviewCategory::Drink,
     {{{"gin", true}, {"tonic", false}, {"", false}, {"", false}}}, 2},
    {"whiskey_sour", "Whiskey Sour", "Citrus and smooth", "signature",
     PreviewCategory::Drink,
     {{{"whiskey", true}, {"sour_mix", false}, {"lime", false}, {"", false}}}, 3},
    {"vodka_soda", "Vodka Soda", "Clean and light", "highball", PreviewCategory::Drink,
     {{{"vodka", true}, {"soda", false}, {"", false}, {"", false}}}, 2},
    {"margarita", "Margarita", "Bright tequila mix", "signature", PreviewCategory::Drink,
     {{{"tequila", true}, {"citrus_mix", false}, {"lime", false}, {"", false}}}, 3},
    {"rum_cola", "Rum Cola", "Easy sweet serve", "highball", PreviewCategory::Drink,
     {{{"rum", true}, {"cola", false}, {"", false}, {"", false}}}, 2},
    {"tequila_shot", "Tequila Shot", "Straight tequila", "shot", PreviewCategory::Shot,
     {{{"tequila", true}, {"", false}, {"", false}, {"", false}}}, 1},
    {"vodka_shot", "Vodka Shot", "Straight vodka", "shot", PreviewCategory::Shot,
     {{{"vodka", true}, {"", false}, {"", false}, {"", false}}}, 1},
    {"whiskey_shot", "Whiskey Shot", "Straight whiskey", "shot", PreviewCategory::Shot,
     {{{"whiskey", true}, {"", false}, {"", false}, {"", false}}}, 1},
}};

std::array<String, tipsy::config::kPumpCount> pumpAssignments {};
String selectedDrinkId;
String statusText = "Ready";
lv_timer_t* pendingStatusTimer = nullptr;
PendingStatus pendingStatus {};

const char* ingredientDisplayName(const String& ingredientId) {
  if (ingredientId == "gin") return "Gin";
  if (ingredientId == "vodka") return "Vodka";
  if (ingredientId == "rum") return "Rum";
  if (ingredientId == "tequila") return "Tequila";
  if (ingredientId == "whiskey") return "Whiskey";
  if (ingredientId == "tonic") return "Tonic";
  if (ingredientId == "cola") return "Cola";
  if (ingredientId == "soda") return "Soda";
  if (ingredientId == "mint_mix") return "Mint Mix";
  if (ingredientId == "lime") return "Lime";
  if (ingredientId == "sour_mix") return "Sour Mix";
  if (ingredientId == "citrus_mix") return "Citrus Mix";
  return "Unassigned";
}

const PreviewRecipe* findRecipe(const String& recipeId) {
  for (const auto& recipe : kRecipes) {
    if (recipeId == recipe.id) {
      return &recipe;
    }
  }

  return nullptr;
}

bool isIngredientMapped(const char* ingredientId) {
  if (ingredientId == nullptr || *ingredientId == '\0') {
    return true;
  }

  for (const auto& assignment : pumpAssignments) {
    if (assignment == ingredientId) {
      return true;
    }
  }

  return false;
}

bool isRecipeAvailable(const PreviewRecipe& recipe) {
  for (std::size_t i = 0; i < recipe.ingredientCount; ++i) {
    if (!isIngredientMapped(recipe.ingredients[i].ingredientId)) {
      return false;
    }
  }

  return true;
}

void clearPendingStatusTimer() {
  if (pendingStatusTimer != nullptr) {
    lv_timer_delete(pendingStatusTimer);
    pendingStatusTimer = nullptr;
  }
}

void applyPreviewModel();

void handlePendingStatusTimer(lv_timer_t* timer) {
  (void)timer;

  if (!pendingStatus.completeMessage.isEmpty()) {
    statusText = pendingStatus.completeMessage;
    pendingStatus.completeMessage = "";
    lv_timer_set_period(pendingStatusTimer, pendingStatus.readyDelayMs);
    applyPreviewModel();
    return;
  }

  statusText = "Ready";
  clearPendingStatusTimer();
  applyPreviewModel();
}

void startTwoStageStatus(const String& startingMessage, const String& completeMessage,
                         std::uint32_t startDelayMs = 1400, std::uint32_t readyDelayMs = 1200) {
  statusText = startingMessage;
  pendingStatus.completeMessage = completeMessage;
  pendingStatus.readyDelayMs = readyDelayMs;
  clearPendingStatusTimer();
  pendingStatusTimer = lv_timer_create(handlePendingStatusTimer, startDelayMs, nullptr);
  applyPreviewModel();
}

void ensureSelectionIsValid() {
  if (selectedDrinkId.isEmpty()) {
    return;
  }

  const PreviewRecipe* recipe = findRecipe(selectedDrinkId);
  if (recipe == nullptr || !isRecipeAvailable(*recipe)) {
    selectedDrinkId = "";
  }
}

void applyPreviewModel() {
  tipsy::ui::generated::UiRenderModel model {};
  ensureSelectionIsValid();

  const PreviewRecipe* selectedRecipe = findRecipe(selectedDrinkId);
  model.headerTitle = "Tipsy";
  model.statusText = statusText;
  model.primaryActionLabel = "Pour Drink";
  model.selectedDrinkText =
      selectedRecipe == nullptr ? String("None") : String(selectedRecipe->displayName);

  for (std::size_t i = 0; i < kRecipes.size() && i < model.drinks.size(); ++i) {
    model.drinks[i].id = kRecipes[i].id;
    model.drinks[i].displayName = kRecipes[i].displayName;
    model.drinks[i].subtitle = kRecipes[i].subtitle;
    model.drinks[i].categoryId = kRecipes[i].categoryId;
    model.drinks[i].available = isRecipeAvailable(kRecipes[i]);
    model.drinks[i].availabilityText = model.drinks[i].available ? "Available" : "Unavailable";
    model.drinks[i].selected = selectedDrinkId == kRecipes[i].id;
    model.drinks[i].disabled = !model.drinks[i].available;
    ++model.drinkCount;
  }

  for (std::size_t i = 0; i < pumpAssignments.size() && i < model.pumpAssignments.size(); ++i) {
    model.pumpAssignments[i].pumpIndex = static_cast<std::uint8_t>(i);
    model.pumpAssignments[i].ingredientId = pumpAssignments[i];
    model.pumpAssignments[i].ingredientDisplayName =
        pumpAssignments[i].isEmpty() ? "Unassigned" : ingredientDisplayName(pumpAssignments[i]);
    model.pumpAssignments[i].enabled = !pumpAssignments[i].isEmpty();
  }

  tipsy::ui::generated::ui_apply_model(model);
}

void onDrinkSelected(const char* drinkId) {
  selectedDrinkId = drinkId == nullptr ? "" : drinkId;
  statusText = selectedDrinkId.isEmpty() ? "Ready" : "Ready to pour";
  clearPendingStatusTimer();
  applyPreviewModel();
}

void onStartSelectedDrink(std::uint16_t alcoholAmountMl) {
  const PreviewRecipe* recipe = findRecipe(selectedDrinkId);
  if (recipe == nullptr || !isRecipeAvailable(*recipe)) {
    statusText = "Selected drink is unavailable.";
    applyPreviewModel();
    return;
  }

  String startMessage = "Pouring ";
  startMessage += recipe->displayName;
  startMessage += " ";
  startMessage += alcoholAmountMl;
  startMessage += " ml...";

  String completeMessage = recipe->displayName;
  completeMessage += " complete.";
  startTwoStageStatus(startMessage, completeMessage);
}

void onAdminOpened() {
  statusText = "Admin settings open.";
  clearPendingStatusTimer();
  applyPreviewModel();
}

void onPrimePumps() {
  startTwoStageStatus("Priming pumps...", "Pump priming complete.", 1200, 1000);
}

void onFlushCleaning() {
  startTwoStageStatus("Flush / Cleaning started.", "Flush / Cleaning complete.", 1500, 1000);
}

void onPumpAssignmentEdited(std::uint8_t pumpIndex, const char* ingredientId,
                            const char* ingredientDisplayName, bool enabled) {
  (void)ingredientDisplayName;
  if (pumpIndex >= pumpAssignments.size()) {
    return;
  }

  pumpAssignments[pumpIndex] = enabled && ingredientId != nullptr ? ingredientId : "";
  statusText = "Pump mapping updated.";
  clearPendingStatusTimer();
  ensureSelectionIsValid();
  applyPreviewModel();
}

}  // namespace

}  // namespace tipsy::desktop_preview

int main() {
  using namespace tipsy::desktop_preview;

  for (std::size_t i = 0; i < kDefaultPumpAssignments.size(); ++i) {
    pumpAssignments[i] = kDefaultPumpAssignments[i];
  }

  lv_init();

  lv_display_t* display =
      lv_windows_create_display(L"Tipsy Desktop Preview", 320, 480, 100, false, true);
  if (display == nullptr) {
    return 1;
  }

  lv_windows_acquire_pointer_indev(display);
  lv_windows_acquire_keypad_indev(display);
  lv_windows_acquire_encoder_indev(display);

  lv_lock();
  tipsy::ui::generated::ui_init_preview_main_menu();
  tipsy::ui::generated::ui_bind_drink_selected(&onDrinkSelected);
  tipsy::ui::generated::ui_bind_start_selected_drink(&onStartSelectedDrink);
  tipsy::ui::generated::ui_bind_admin_opened(&onAdminOpened);
  tipsy::ui::generated::ui_bind_prime_pumps(&onPrimePumps);
  tipsy::ui::generated::ui_bind_flush_cleaning(&onFlushCleaning);
  tipsy::ui::generated::ui_bind_pump_assignment_edited(&onPumpAssignmentEdited);
  applyPreviewModel();
  lv_unlock();

  HWND windowHandle = lv_windows_get_display_window_handle(display);
  while (windowHandle != nullptr && IsWindow(windowHandle)) {
    std::uint32_t idleTimeMs = lv_timer_handler();
    if (idleTimeMs == LV_NO_TIMER_READY) {
      idleTimeMs = LV_DEF_REFR_PERIOD;
    }

    lv_delay_ms(idleTimeMs);
  }

  clearPendingStatusTimer();
  return 0;
}
