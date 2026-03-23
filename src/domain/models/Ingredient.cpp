#include "domain/models/Ingredient.h"

namespace tipsy::domain {

bool Ingredient::isValid() const {
  return !id.isEmpty() && !displayName.isEmpty();
}

}  // namespace tipsy::domain

