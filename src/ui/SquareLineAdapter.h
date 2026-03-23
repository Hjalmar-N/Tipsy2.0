#pragma once

#include "ui/UiBridge.h"
#include "ui/generated/ui.h"

namespace tipsy::ui {

// Keeps project-owned binding code separate from generated SquareLine exports.
class SquareLineAdapter {
 public:
  void bind(UiBridge& uiBridge);
  void begin();
  void update();

 private:
  String machineStateText(tipsy::app::MachineState state) const;
  void applyStateFeedback(const UiState& state, generated::UiRenderModel& model) const;
  generated::UiRenderModel buildRenderModel(const UiState& state) const;

  UiBridge* uiBridge_ = nullptr;
};

}  // namespace tipsy::ui
