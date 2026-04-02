#include "ui/UiEvents.h"

namespace tipsy::ui::events {

void handleDrinkSelected(UiBridge& uiBridge, const char* drinkId) {
  uiBridge.onSelectDrink(String(drinkId));
}

void handleStartSelectedDrink(UiBridge& uiBridge, std::uint16_t alcoholOverrideMl,
                              std::uint8_t speedPercent) {
  uiBridge.onStartSelectedDrink(alcoholOverrideMl, speedPercent);
}

void handleManualPourRequested(UiBridge& uiBridge, const char* ingredientId, float volumeMl,
                               std::uint8_t speedPercent) {
  uiBridge.onStartManualPour(String(ingredientId), volumeMl, speedPercent);
}

void handleAdminOpened(UiBridge& uiBridge) {
  uiBridge.onOpenAdmin();
}

void handlePrimePumpsRequested(UiBridge& uiBridge) {
  uiBridge.onPrimePumps();
}

void handleFlushCleaningRequested(UiBridge& uiBridge) {
  uiBridge.onFlushCleaning();
}

void handlePumpAssignmentEdited(UiBridge& uiBridge, std::uint8_t pumpIndex,
                                const char* ingredientId, const char* ingredientDisplayName,
                                bool enabled) {
  uiBridge.onEditPumpAssignment(pumpIndex, String(ingredientId),
                                String(ingredientDisplayName), enabled);
}

}  // namespace tipsy::ui::events
