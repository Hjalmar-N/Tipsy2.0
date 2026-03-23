#pragma once

#include <Arduino.h>
#include <array>
#include <cstddef>
#include <cstdint>

#include "config/AppConfig.h"
#include "domain/models/DrinkIngredient.h"

namespace tipsy::domain {

// Fixed-size recipe model to avoid unnecessary heap pressure later.
struct DrinkRecipe {
  String id;
  String displayName;
  String categoryId;
  String description;
  bool enabled = true;
  bool visibleInMenu = true;
  std::uint16_t sortOrder = 0;
  std::array<DrinkIngredient, config::kMaxRecipeIngredients> ingredients {};
  std::size_t ingredientCount = 0;

  bool addIngredient(const DrinkIngredient& ingredient);
  bool isValid() const;
  void clearIngredients();
};

}  // namespace tipsy::domain
