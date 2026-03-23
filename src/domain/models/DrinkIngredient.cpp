#include "domain/models/DrinkIngredient.h"

namespace tipsy::domain {

bool DrinkIngredient::isValid() const {
  return !ingredientId.isEmpty() && amountMl > 0.0F;
}

}  // namespace tipsy::domain

