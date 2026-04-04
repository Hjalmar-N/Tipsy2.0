#pragma once

#include <cstdint>

#include "ui/UiBridge.h"

namespace tipsy::ui::events {

// Placeholder callback entry points for future SquareLine-generated event handlers.
void handleDrinkSelected(UiBridge& uiBridge, const char* drinkId);
void handleStartSelectedDrink(UiBridge& uiBridge, std::uint16_t alcoholOverrideMl = 0,
                              std::uint8_t speedPercent = 100);
void handleManualPourRequested(UiBridge& uiBridge, const char* ingredientId, float volumeMl,
                               std::uint8_t speedPercent);
void handleAdminOpened(UiBridge& uiBridge);
void handlePrimePumpsRequested(UiBridge& uiBridge);
void handleFlushCleaningRequested(UiBridge& uiBridge);
void handlePumpAssignmentEdited(UiBridge& uiBridge, std::uint8_t pumpIndex,
                                const char* ingredientId, const char* ingredientDisplayName,
                                bool enabled);

}  // namespace tipsy::ui::events
