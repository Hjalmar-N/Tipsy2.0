#pragma once

#include <array>
#include <cstdint>

#include "config/AppConfig.h"
#include "domain/models/PourRequest.h"
#include "domain/models/SystemSettings.h"
#include "hal/interfaces/ITimeProvider.h"
#include "pumps/IPumpDriver.h"
#include "pumps/PumpTask.h"

namespace tipsy::pumps {

// Non-blocking pump orchestration for timed pours across all pump channels.
class PumpController {
 public:
  PumpController(IPumpDriver& pumpDriver, const tipsy::hal::ITimeProvider& timeProvider);

  bool begin();
  bool update();
  bool startPour(std::uint8_t pumpIndex, float volumeMl, std::uint8_t speedPercent,
                 const tipsy::domain::SystemSettings& settings);
  bool startPour(const tipsy::domain::PourRequest& request,
                 const tipsy::domain::SystemSettings& settings);
  bool isPumpActive(std::uint8_t pumpIndex) const;
  std::uint8_t activeTaskCount() const;
  void stopPump(std::uint8_t pumpIndex);
  void stopAll();

 private:
  bool validatePourRequest(const tipsy::domain::PourRequest& request,
                           const tipsy::domain::SystemSettings& settings) const;
  std::uint32_t calculateDurationMs(const tipsy::domain::PourRequest& request,
                                    const tipsy::domain::PumpCalibration& calibration) const;

  IPumpDriver& pumpDriver_;
  const tipsy::hal::ITimeProvider& timeProvider_;
  std::array<PumpTask, tipsy::config::kPumpCount> activeTasks_ {};
};

}  // namespace tipsy::pumps
