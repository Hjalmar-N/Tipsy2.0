#pragma once

#include <Arduino.h>

namespace tipsy::domain {

// A measured ingredient row within a drink recipe.
struct DrinkIngredient {
  String ingredientId;
  float amountMl = 0.0F;
  bool optional = false;

  bool isValid() const;
};

}  // namespace tipsy::domain

