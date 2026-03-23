#pragma once

#include <array>

#include "config/AppConfig.h"
#include "domain/models/DrinkRecipe.h"
#include "storage/JsonStorage.h"

namespace tipsy::app {

// Storage-backed drink catalog service with first-boot default generation.
class RecipeService {
 public:
  explicit RecipeService(tipsy::storage::JsonStorage& jsonStorage);

  bool load();
  bool save();
  bool upsertRecipe(const tipsy::domain::DrinkRecipe& recipe);
  const std::array<tipsy::domain::DrinkRecipe, tipsy::config::kMaxDrinkCount>& all() const;
  std::size_t count() const;
  const tipsy::domain::DrinkRecipe* findById(const String& drinkId) const;
  const String& lastError() const;

 private:
  void loadDefaults();
  bool ensureDefaultFile();
  bool parseRecipes(const JsonArrayConst& drinksArray);
  void serializeRecipes(JsonArray drinksArray) const;

  tipsy::storage::JsonStorage& jsonStorage_;
  std::array<tipsy::domain::DrinkRecipe, tipsy::config::kMaxDrinkCount> recipes_ {};
  std::size_t recipeCount_ = 0;
  String lastError_;
};

}  // namespace tipsy::app

