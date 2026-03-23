#pragma once

#include <Arduino.h>
#include <cstdint>

namespace tipsy::domain {

enum class PourMode : std::uint8_t {
  None = 0,
  ManualSingleIngredient = 1,
  Recipe = 2,
};

// Command object for both manual pours and future queued recipe pours.
struct PourRequest {
  String requestId;
  PourMode mode = PourMode::None;
  std::uint8_t pumpIndex = 0;
  String ingredientId;
  String drinkRecipeId;
  float amountMl = 0.0F;
  std::uint8_t speedPercent = 100;
  std::uint32_t createdAtMs = 0;

  static PourRequest manual(const String& ingredientId, std::uint8_t pumpIndex, float amountMl,
                            std::uint8_t speedPercent = 100);
  bool isValid() const;
};

}  // namespace tipsy::domain
