#pragma once

#include <array>
#include <cstddef>

#include <ArduinoJson.h>

#include "config/AppConfig.h"
#include "domain/models/Ingredient.h"
#include "storage/JsonStorage.h"

namespace tipsy::app {

// Storage-backed ingredient catalog service.
class IngredientService {
 public:
  explicit IngredientService(tipsy::storage::JsonStorage& jsonStorage);

  bool load();
  bool save();
  const std::array<tipsy::domain::Ingredient, tipsy::config::kIngredientCatalogCount>& all() const;
  std::size_t count() const;
  const tipsy::domain::Ingredient* findById(const String& ingredientId) const;
  const String& lastError() const;

 private:
  void loadDefaults();
  bool ensureDefaultFile();
  bool parseIngredients(const JsonArrayConst& ingredientsArray);
  void serializeIngredients(JsonArray ingredientsArray) const;

  tipsy::storage::JsonStorage& jsonStorage_;
  std::array<tipsy::domain::Ingredient, tipsy::config::kIngredientCatalogCount> ingredients_ {};
  std::size_t ingredientCount_ = 0;
  String lastError_;
};

}  // namespace tipsy::app
