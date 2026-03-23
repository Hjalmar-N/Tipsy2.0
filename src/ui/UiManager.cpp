#include "ui/UiManager.h"

namespace tipsy::ui {

UiManager::UiManager(UiBridge& uiBridge) : display_(), uiBridge_(uiBridge) {}

bool UiManager::begin() {
  lv_init();
  display_.begin();
  uiBridge_.begin();
  squareLineAdapter_.bind(uiBridge_);
  squareLineAdapter_.begin();
  return true;
}

void UiManager::update() {
  // Future work: add LVGL tick source plus display/input driver registration here.
  lv_timer_handler();
  uiBridge_.syncFromMachine();
}

}  // namespace tipsy::ui
