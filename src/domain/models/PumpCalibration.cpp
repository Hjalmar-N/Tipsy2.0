#include "domain/models/PumpCalibration.h"

namespace tipsy::domain {

std::uint32_t PumpCalibration::estimatePourTimeMs(float amountMl) const {
  if (mlPerSecond <= 0.0F || amountMl <= 0.0F) {
    return 0;
  }

  return static_cast<std::uint32_t>((amountMl / mlPerSecond) * 1000.0F);
}

bool PumpCalibration::isValid() const {
  return mlPerSecond > 0.0F;
}

}  // namespace tipsy::domain

