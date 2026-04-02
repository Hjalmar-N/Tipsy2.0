#include "app/services/RecipeService.h"

#include <ArduinoJson.h>

#include "storage/StoragePaths.h"

namespace tipsy::app {

namespace {

constexpr size_t kRecipeDocCapacity = 8192;

}  // namespace

RecipeService::RecipeService(tipsy::storage::JsonStorage& jsonStorage) : jsonStorage_(jsonStorage) {}

bool RecipeService::load() {
  if (!ensureDefaultFile()) {
    return false;
  }

  DynamicJsonDocument doc(kRecipeDocCapacity);
  if (!jsonStorage_.readJson(tipsy::storage::paths::kDrinks, doc)) {
    lastError_ = jsonStorage_.lastError();
    return false;
  }

  return parseRecipes(doc["drinks"].as<JsonArrayConst>());
}

bool RecipeService::save() {
  DynamicJsonDocument doc(kRecipeDocCapacity);
  doc["schemaVersion"] = 1;
  JsonArray drinks = doc.createNestedArray("drinks");
  serializeRecipes(drinks);

  const bool ok = jsonStorage_.writeJson(tipsy::storage::paths::kDrinks, doc);
  lastError_ = ok ? String() : jsonStorage_.lastError();
  return ok;
}

bool RecipeService::upsertRecipe(const tipsy::domain::DrinkRecipe& recipe) {
  if (!recipe.isValid()) {
    lastError_ = "Recipe is invalid.";
    return false;
  }

  for (std::size_t i = 0; i < recipeCount_; ++i) {
    if (recipes_[i].id == recipe.id) {
      recipes_[i] = recipe;
      return save();
    }
  }

  if (recipeCount_ >= recipes_.size()) {
    lastError_ = "Recipe storage is full.";
    return false;
  }

  recipes_[recipeCount_] = recipe;
  ++recipeCount_;
  return save();
}

const std::array<tipsy::domain::DrinkRecipe, tipsy::config::kMaxDrinkCount>& RecipeService::all()
    const {
  return recipes_;
}

std::size_t RecipeService::count() const {
  return recipeCount_;
}

const tipsy::domain::DrinkRecipe* RecipeService::findById(const String& drinkId) const {
  for (std::size_t i = 0; i < recipeCount_; ++i) {
    if (recipes_[i].id == drinkId) {
      return &recipes_[i];
    }
  }

  return nullptr;
}

const String& RecipeService::lastError() const {
  return lastError_;
}

void RecipeService::loadDefaults() {
  recipeCount_ = 0;

  recipes_[0].id = "gin_tonic";
  recipes_[0].displayName = "Gin & Tonic";
  recipes_[0].categoryId = "highball";
  recipes_[0].description = "Classic gin and tonic.";
  recipes_[0].enabled = true;
  recipes_[0].visibleInMenu = true;
  recipes_[0].sortOrder = 10;
  recipes_[0].addIngredient({"gin", 60.0F, false, true});
  recipes_[0].addIngredient({"tonic", 150.0F});

  recipes_[1].id = "vodka_soda";
  recipes_[1].displayName = "Vodka Soda";
  recipes_[1].categoryId = "highball";
  recipes_[1].description = "Clean and simple vodka soda.";
  recipes_[1].enabled = true;
  recipes_[1].visibleInMenu = true;
  recipes_[1].sortOrder = 20;
  recipes_[1].addIngredient({"vodka", 60.0F, false, true});
  recipes_[1].addIngredient({"soda", 150.0F});

  recipes_[2].id = "rum_cola";
  recipes_[2].displayName = "Rum & Cola";
  recipes_[2].categoryId = "highball";
  recipes_[2].description = "Rum served with cola over ice.";
  recipes_[2].enabled = true;
  recipes_[2].visibleInMenu = true;
  recipes_[2].sortOrder = 30;
  recipes_[2].addIngredient({"rum", 60.0F, false, true});
  recipes_[2].addIngredient({"cola", 150.0F});

  recipes_[3].id = "tequila_sunrise";
  recipes_[3].displayName = "Tequila Sunrise";
  recipes_[3].categoryId = "signature";
  recipes_[3].description = "Tequila, orange juice, and grenadine.";
  recipes_[3].enabled = true;
  recipes_[3].visibleInMenu = true;
  recipes_[3].sortOrder = 40;
  recipes_[3].addIngredient({"tequila", 60.0F, false, true});
  recipes_[3].addIngredient({"orange_juice", 120.0F});
  recipes_[3].addIngredient({"grenadine", 15.0F});

  recipes_[4].id = "screwdriver";
  recipes_[4].displayName = "Screwdriver";
  recipes_[4].categoryId = "highball";
  recipes_[4].description = "Vodka topped with orange juice.";
  recipes_[4].enabled = true;
  recipes_[4].visibleInMenu = true;
  recipes_[4].sortOrder = 50;
  recipes_[4].addIngredient({"vodka", 60.0F, false, true});
  recipes_[4].addIngredient({"orange_juice", 150.0F});

  recipes_[5].id = "mojito";
  recipes_[5].displayName = "Mojito";
  recipes_[5].categoryId = "signature";
  recipes_[5].description = "Rum with mint, soda, and lime.";
  recipes_[5].enabled = true;
  recipes_[5].visibleInMenu = true;
  recipes_[5].sortOrder = 60;
  recipes_[5].addIngredient({"rum", 60.0F, false, true});
  recipes_[5].addIngredient({"mint_mix", 120.0F});
  recipes_[5].addIngredient({"soda", 80.0F});
  recipes_[5].addIngredient({"lime", 20.0F});

  recipes_[6].id = "whiskey_sour";
  recipes_[6].displayName = "Whiskey Sour";
  recipes_[6].categoryId = "signature";
  recipes_[6].description = "Whiskey with sour mix and lime.";
  recipes_[6].enabled = true;
  recipes_[6].visibleInMenu = true;
  recipes_[6].sortOrder = 70;
  recipes_[6].addIngredient({"whiskey", 60.0F, false, true});
  recipes_[6].addIngredient({"sour_mix", 90.0F});
  recipes_[6].addIngredient({"lime", 20.0F});

  recipes_[7].id = "margarita";
  recipes_[7].displayName = "Margarita";
  recipes_[7].categoryId = "signature";
  recipes_[7].description = "Tequila with citrus mix and lime.";
  recipes_[7].enabled = true;
  recipes_[7].visibleInMenu = true;
  recipes_[7].sortOrder = 80;
  recipes_[7].addIngredient({"tequila", 60.0F, false, true});
  recipes_[7].addIngredient({"citrus_mix", 80.0F});
  recipes_[7].addIngredient({"lime", 20.0F});

  recipes_[8].id = "tequila_shot";
  recipes_[8].displayName = "Tequila Shot";
  recipes_[8].categoryId = "shot";
  recipes_[8].description = "Straight tequila shot.";
  recipes_[8].enabled = true;
  recipes_[8].visibleInMenu = true;
  recipes_[8].sortOrder = 90;
  recipes_[8].addIngredient({"tequila", 60.0F, false, true});

  recipes_[9].id = "vodka_shot";
  recipes_[9].displayName = "Vodka Shot";
  recipes_[9].categoryId = "shot";
  recipes_[9].description = "Straight vodka shot.";
  recipes_[9].enabled = true;
  recipes_[9].visibleInMenu = true;
  recipes_[9].sortOrder = 100;
  recipes_[9].addIngredient({"vodka", 60.0F, false, true});

  recipes_[10].id = "whiskey_shot";
  recipes_[10].displayName = "Whiskey Shot";
  recipes_[10].categoryId = "shot";
  recipes_[10].description = "Straight whiskey shot.";
  recipes_[10].enabled = true;
  recipes_[10].visibleInMenu = true;
  recipes_[10].sortOrder = 110;
  recipes_[10].addIngredient({"whiskey", 60.0F, false, true});

  recipeCount_ = 11;
}

bool RecipeService::ensureDefaultFile() {
  DynamicJsonDocument doc(kRecipeDocCapacity);
  doc["schemaVersion"] = 1;
  JsonArray drinks = doc.createNestedArray("drinks");
  loadDefaults();
  serializeRecipes(drinks);

  const bool ok = jsonStorage_.ensureFile(tipsy::storage::paths::kDrinks, doc);
  lastError_ = ok ? String() : jsonStorage_.lastError();
  return ok;
}

bool RecipeService::parseRecipes(const JsonArrayConst& drinksArray) {
  recipeCount_ = 0;

  for (JsonObjectConst drink : drinksArray) {
    if (recipeCount_ >= recipes_.size()) {
      lastError_ = "Too many drinks in storage.";
      return false;
    }

    tipsy::domain::DrinkRecipe recipe;
    recipe.id = drink["id"] | "";
    recipe.displayName = drink["displayName"] | "";
    recipe.categoryId = drink["categoryId"] | "";
    recipe.description = drink["description"] | "";
    recipe.enabled = drink["enabled"] | true;
    recipe.visibleInMenu = drink["visibleInMenu"] | true;
    recipe.sortOrder = drink["sortOrder"] | 0;

    JsonArrayConst ingredients = drink["ingredients"].as<JsonArrayConst>();
    for (JsonObjectConst ingredient : ingredients) {
      recipe.addIngredient({ingredient["ingredientId"] | "",
                            ingredient["amountMl"] | 0.0F,
                            ingredient["optional"] | false,
                            ingredient["isAlcohol"] | false});
    }

    if (!recipe.isValid()) {
      lastError_ = String("Invalid recipe found: ") + recipe.id;
      return false;
    }

    recipes_[recipeCount_] = recipe;
    ++recipeCount_;
  }

  lastError_ = String();
  return true;
}

void RecipeService::serializeRecipes(JsonArray drinksArray) const {
  for (std::size_t i = 0; i < recipeCount_; ++i) {
    const auto& recipe = recipes_[i];
    JsonObject drink = drinksArray.createNestedObject();
    drink["id"] = recipe.id;
    drink["displayName"] = recipe.displayName;
    drink["categoryId"] = recipe.categoryId;
    drink["description"] = recipe.description;
    drink["enabled"] = recipe.enabled;
    drink["visibleInMenu"] = recipe.visibleInMenu;
    drink["sortOrder"] = recipe.sortOrder;

    JsonArray ingredients = drink.createNestedArray("ingredients");
    for (std::size_t j = 0; j < recipe.ingredientCount; ++j) {
      JsonObject ingredient = ingredients.createNestedObject();
      ingredient["ingredientId"] = recipe.ingredients[j].ingredientId;
      ingredient["amountMl"] = recipe.ingredients[j].amountMl;
      ingredient["optional"] = recipe.ingredients[j].optional;
      ingredient["isAlcohol"] = recipe.ingredients[j].isAlcohol;
    }
  }
}

}  // namespace tipsy::app
