#pragma once

#include <cstddef>

namespace tipsy::config {

// Global compile-time limits for the first machine generation.
inline constexpr std::size_t kPumpCount = TIPSY_PUMP_COUNT;
inline constexpr std::size_t kIngredientCatalogCount = 9;
inline constexpr std::size_t kMaxRecipeIngredients = 8;
inline constexpr std::size_t kDefaultRecipeCount = 5;
inline constexpr std::size_t kMaxDrinkCount = 16;
inline constexpr const char* kDrinksPath = "/drinks.json";
inline constexpr const char* kIngredientsPath = "/ingredients.json";
inline constexpr const char* kPumpMapPath = "/pump_map.json";
inline constexpr const char* kSettingsPath = "/settings.json";

}  // namespace tipsy::config
