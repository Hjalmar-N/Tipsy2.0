#include "domain/models/PourRequest.h"

namespace tipsy::domain {

PourRequest PourRequest::manual(const String& ingredientId, std::uint8_t pumpIndex, float amountMl,
                                std::uint8_t speedPercent) {
  PourRequest request;
  request.mode = PourMode::ManualSingleIngredient;
  request.ingredientId = ingredientId;
  request.pumpIndex = pumpIndex;
  request.amountMl = amountMl;
  request.speedPercent = speedPercent;
  return request;
}

bool PourRequest::isValid() const {
  if (mode == PourMode::ManualSingleIngredient) {
    return !ingredientId.isEmpty() && amountMl > 0.0F && speedPercent > 0 &&
           speedPercent <= 100;
  }

  if (mode == PourMode::Recipe) {
    return !drinkRecipeId.isEmpty() && amountMl > 0.0F && speedPercent > 0 &&
           speedPercent <= 100;
  }

  return false;
}

}  // namespace tipsy::domain
