#pragma once

#include "ui/UiBridge.h"

namespace tipsy::ui {

// Keeps project-owned binding code separate from generated SquareLine exports.
class SquareLineAdapter {
 public:
  void bind(UiBridge& uiBridge);
  void begin();
  void update();

 private:
  UiBridge* uiBridge_ = nullptr;
};

}  // namespace tipsy::ui
