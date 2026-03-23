#pragma once

#include <Arduino.h>
#include <cstdint>

namespace tipsy::domain {

// Describes a liquid or mixer available to recipes and pump mapping.
struct Ingredient {
  String id;
  String displayName;
  String categoryId;
  bool enabled = true;
  bool alcoholic = false;
  float abvPercent = 0.0F;
  std::uint16_t sortOrder = 0;

  bool isValid() const;
};

}  // namespace tipsy::domain
