#pragma once

#include <TFT_eSPI.h>
#include <lvgl.h>

#include "ui/UiBridge.h"
#include "ui/SquareLineAdapter.h"

namespace tipsy::ui {

// Owns LVGL startup and periodic UI processing while keeping app logic behind UiBridge.
class UiManager {
 public:
  explicit UiManager(UiBridge& uiBridge);

  bool begin();
  void update();

 private:
  TFT_eSPI display_;
  UiBridge& uiBridge_;
  SquareLineAdapter squareLineAdapter_;
};

}  // namespace tipsy::ui
