#include "domain/models/PumpAssignment.h"

namespace tipsy::domain {

bool PumpAssignment::isAssigned() const {
  return enabled && !ingredientId.isEmpty();
}

}  // namespace tipsy::domain

