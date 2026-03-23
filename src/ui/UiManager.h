#pragma once

#include <cstdint>

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
  void tick(std::uint32_t elapsedMs);
  bool registerDisplayDriver();
  bool registerInputDriver();
  bool isReady() const;

 private:
  TFT_eSPI display_;
  UiBridge& uiBridge_;
  SquareLineAdapter squareLineAdapter_;
  bool ready_ = false;
  bool displayDriverRegistered_ = false;
  bool inputDriverRegistered_ = false;
};

}  // namespace tipsy::ui
