#include "ui/UiEvents.h"

namespace tipsy::ui::events {

void handleDrinkSelected(UiBridge& uiBridge, const char* drinkId) {
  uiBridge.onSelectDrink(String(drinkId));
}

void handleManualPourRequested(UiBridge& uiBridge, const char* ingredientId, float volumeMl,
                               std::uint8_t speedPercent) {
  uiBridge.onStartManualPour(String(ingredientId), volumeMl, speedPercent);
}

void handleAdminOpened(UiBridge& uiBridge) {
  uiBridge.onOpenAdmin();
}

void handlePumpAssignmentEdited(UiBridge& uiBridge, std::uint8_t pumpIndex,
                                const char* ingredientId, const char* ingredientDisplayName,
                                bool enabled) {
  uiBridge.onEditPumpAssignment(pumpIndex, String(ingredientId),
                                String(ingredientDisplayName), enabled);
}

}  // namespace tipsy::ui::events

