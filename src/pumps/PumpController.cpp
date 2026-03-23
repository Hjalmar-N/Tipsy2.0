#include "pumps/PumpController.h"

namespace tipsy::pumps {

PumpController::PumpController(IPumpDriver& pumpDriver,
                               const tipsy::hal::ITimeProvider& timeProvider)
    : pumpDriver_(pumpDriver), timeProvider_(timeProvider) {}

bool PumpController::begin() {
  const bool ok = pumpDriver_.begin();
  stopAll();
  return ok;
}

bool PumpController::update() {
  const std::uint32_t nowMs = timeProvider_.millis32();
  bool allHealthy = true;

  for (std::uint8_t pumpIndex = 0; pumpIndex < activeTasks_.size(); ++pumpIndex) {
    PumpTask& task = activeTasks_[pumpIndex];
    if (!task.active) {
      continue;
    }

    if (nowMs >= task.endAtMs()) {
      stopPump(pumpIndex);
    }
  }

  return allHealthy;
}

bool PumpController::startPour(std::uint8_t pumpIndex, float volumeMl, std::uint8_t speedPercent,
                               const tipsy::domain::SystemSettings& settings) {
  const auto* assignment = settings.findAssignment(pumpIndex);
  if (assignment == nullptr) {
    return false;
  }

  const tipsy::domain::PourRequest request =
      tipsy::domain::PourRequest::manual(assignment->ingredientId, pumpIndex, volumeMl,
                                         speedPercent);
  return startPour(request, settings);
}

bool PumpController::startPour(const tipsy::domain::PourRequest& request,
                               const tipsy::domain::SystemSettings& settings) {
  const std::uint8_t pumpIndex = request.pumpIndex;
  if (!validatePourRequest(request, settings)) {
    return false;
  }

  const auto* assignment = settings.findAssignment(pumpIndex);
  const auto* calibration = settings.findCalibration(pumpIndex);
  if (assignment == nullptr || calibration == nullptr) {
    return false;
  }

  if (!pumpDriver_.setPumpSpeed(pumpIndex, request.speedPercent)) {
    return false;
  }

  PumpTask& task = activeTasks_[pumpIndex];
  task.active = true;
  task.pumpIndex = pumpIndex;
  task.targetVolumeMl = request.amountMl;
  task.speedPercent = request.speedPercent;
  task.flowMlPerSecond = calibration->mlPerSecond;
  task.startedAtMs = timeProvider_.millis32();
  task.durationMs = calculateDurationMs(request, *calibration);
  return true;
}

bool PumpController::isPumpActive(std::uint8_t pumpIndex) const {
  if (pumpIndex >= activeTasks_.size()) {
    return false;
  }

  return activeTasks_[pumpIndex].active;
}

std::uint8_t PumpController::activeTaskCount() const {
  std::uint8_t count = 0;
  for (const auto& task : activeTasks_) {
    if (task.active) {
      ++count;
    }
  }
  return count;
}

void PumpController::stopPump(std::uint8_t pumpIndex) {
  if (pumpIndex >= activeTasks_.size()) {
    return;
  }

  pumpDriver_.stopPump(pumpIndex);
  activeTasks_[pumpIndex] = {};
}

void PumpController::stopAll() {
  pumpDriver_.stopAll();
  for (std::uint8_t pumpIndex = 0; pumpIndex < activeTasks_.size(); ++pumpIndex) {
    activeTasks_[pumpIndex] = {};
  }
}

bool PumpController::validatePourRequest(const tipsy::domain::PourRequest& request,
                                         const tipsy::domain::SystemSettings& settings) const {
  const std::uint8_t pumpIndex = request.pumpIndex;
  if (!request.isValid() || pumpIndex >= activeTasks_.size()) {
    return false;
  }

  const auto* assignment = settings.findAssignment(pumpIndex);
  const auto* calibration = settings.findCalibration(pumpIndex);
  if (assignment == nullptr || calibration == nullptr) {
    return false;
  }

  if (!assignment->enabled || !assignment->isAssigned() || !calibration->enabled ||
      !calibration->isValid()) {
    return false;
  }

  if (!request.ingredientId.isEmpty() && request.ingredientId != assignment->ingredientId) {
    return false;
  }

  if (activeTasks_[pumpIndex].active) {
    return false;
  }

  return true;
}

std::uint32_t PumpController::calculateDurationMs(
    const tipsy::domain::PourRequest& request,
    const tipsy::domain::PumpCalibration& calibration) const {
  const float speedScale = static_cast<float>(request.speedPercent) / 100.0F;
  const float effectiveFlowMlPerSec = calibration.mlPerSecond * speedScale;
  if (effectiveFlowMlPerSec <= 0.0F) {
    return 0;
  }

  const float durationMs = (request.amountMl / effectiveFlowMlPerSec) * 1000.0F;
  if (durationMs < 1.0F) {
    return 1;
  }

  return static_cast<std::uint32_t>(durationMs);
}

}  // namespace tipsy::pumps
