#include "app/services/IngredientService.h"

#include <ArduinoJson.h>

#include "storage/StoragePaths.h"

namespace tipsy::app {

IngredientService::IngredientService(tipsy::storage::JsonStorage& jsonStorage)
    : jsonStorage_(jsonStorage) {}

bool IngredientService::load() {
  if (!ensureDefaultFile()) {
    return false;
  }

  DynamicJsonDocument doc(3072);
  if (!jsonStorage_.readJson(tipsy::storage::paths::kIngredients, doc)) {
    lastError_ = jsonStorage_.lastError();
    return false;
  }

  return parseIngredients(doc["ingredients"].as<JsonArrayConst>());
}

bool IngredientService::save() {
  DynamicJsonDocument doc(3072);
  doc["schemaVersion"] = 1;
  JsonArray ingredients = doc.createNestedArray("ingredients");
  serializeIngredients(ingredients);

  const bool ok = jsonStorage_.writeJson(tipsy::storage::paths::kIngredients, doc);
  lastError_ = ok ? String() : jsonStorage_.lastError();
  return ok;
}

const std::array<tipsy::domain::Ingredient, tipsy::config::kIngredientCatalogCount>&
IngredientService::all() const {
  return ingredients_;
}

std::size_t IngredientService::count() const {
  return ingredientCount_;
}

const tipsy::domain::Ingredient* IngredientService::findById(const String& ingredientId) const {
  for (std::size_t i = 0; i < ingredientCount_; ++i) {
    if (ingredients_[i].id == ingredientId) {
      return &ingredients_[i];
    }
  }

  return nullptr;
}

const String& IngredientService::lastError() const {
  return lastError_;
}

void IngredientService::loadDefaults() {
  ingredients_[0] = {"gin", "Gin", "spirit", true, true, 37.5F, 10};
  ingredients_[1] = {"vodka", "Vodka", "spirit", true, true, 40.0F, 20};
  ingredients_[2] = {"rum", "Rum", "spirit", true, true, 37.5F, 30};
  ingredients_[3] = {"tequila", "Tequila", "spirit", true, true, 38.0F, 40};
  ingredients_[4] = {"tonic", "Tonic Water", "mixer", true, false, 0.0F, 50};
  ingredients_[5] = {"soda", "Soda Water", "mixer", true, false, 0.0F, 60};
  ingredients_[6] = {"cola", "Cola", "mixer", true, false, 0.0F, 70};
  ingredients_[7] = {"orange_juice", "Orange Juice", "juice", true, false, 0.0F, 80};
  ingredients_[8] = {"grenadine", "Grenadine", "syrup", true, false, 0.0F, 90};
  ingredientCount_ = 9;
}

bool IngredientService::ensureDefaultFile() {
  DynamicJsonDocument doc(3072);
  doc["schemaVersion"] = 1;
  JsonArray ingredients = doc.createNestedArray("ingredients");
  loadDefaults();
  serializeIngredients(ingredients);

  const bool ok = jsonStorage_.ensureFile(tipsy::storage::paths::kIngredients, doc);
  lastError_ = ok ? String() : jsonStorage_.lastError();
  return ok;
}

bool IngredientService::parseIngredients(const JsonArrayConst& ingredientsArray) {
  ingredientCount_ = 0;

  for (JsonObjectConst ingredientJson : ingredientsArray) {
    if (ingredientCount_ >= ingredients_.size()) {
      lastError_ = "Too many ingredients in storage.";
      return false;
    }

    tipsy::domain::Ingredient ingredient;
    ingredient.id = ingredientJson["id"] | "";
    ingredient.displayName = ingredientJson["displayName"] | "";
    ingredient.categoryId = ingredientJson["categoryId"] | "";
    ingredient.enabled = ingredientJson["enabled"] | true;
    ingredient.alcoholic = ingredientJson["alcoholic"] | false;
    ingredient.abvPercent = ingredientJson["abvPercent"] | 0.0F;
    ingredient.sortOrder = ingredientJson["sortOrder"] | 0;

    if (!ingredient.isValid()) {
      lastError_ = String("Invalid ingredient found: ") + ingredient.id;
      return false;
    }

    ingredients_[ingredientCount_] = ingredient;
    ++ingredientCount_;
  }

  lastError_ = String();
  return true;
}

void IngredientService::serializeIngredients(JsonArray ingredientsArray) const {
  for (std::size_t i = 0; i < ingredientCount_; ++i) {
    const auto& ingredient = ingredients_[i];
    JsonObject entry = ingredientsArray.createNestedObject();
    entry["id"] = ingredient.id;
    entry["displayName"] = ingredient.displayName;
    entry["categoryId"] = ingredient.categoryId;
    entry["enabled"] = ingredient.enabled;
    entry["alcoholic"] = ingredient.alcoholic;
    entry["abvPercent"] = ingredient.abvPercent;
    entry["sortOrder"] = ingredient.sortOrder;
  }
}

}  // namespace tipsy::app
