#include "domain/models/DrinkRecipe.h"

namespace tipsy::domain {

bool DrinkRecipe::addIngredient(const DrinkIngredient& ingredient) {
  if (!ingredient.isValid() || ingredientCount >= ingredients.size()) {
    return false;
  }

  ingredients[ingredientCount] = ingredient;
  ++ingredientCount;
  return true;
}

bool DrinkRecipe::isValid() const {
  return !id.isEmpty() && !displayName.isEmpty() && ingredientCount > 0;
}

void DrinkRecipe::clearIngredients() {
  ingredientCount = 0;
}

}  // namespace tipsy::domain

