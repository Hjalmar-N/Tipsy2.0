#pragma once

#include <Arduino.h>
#include <array>
#include <cstddef>
#include <cstdint>

#include "app/models/MachineStatus.h"
#include "config/AppConfig.h"

namespace tipsy::app {

struct MachinePourIngredient {
  String ingredientId;
  String ingredientDisplayName;
  float amountMl = 0.0F;
  bool isAlcohol = false;
  bool mapped = false;
  std::uint8_t pumpIndex = 255;
};

struct MachinePourPlan {
  bool available = false;
  bool wouldRoute = false;
  bool isShot = false;
  String drinkId;
  String drinkName;
  String categoryId;
  std::uint16_t alcoholAmountMl = 0;
  std::array<MachinePourIngredient, tipsy::config::kMaxRecipeIngredients> ingredients {};
  std::size_t ingredientCount = 0;
  std::uint16_t estimatedPourTimeSec = 0;
  MachineStatus status;
};

}  // namespace tipsy::app
