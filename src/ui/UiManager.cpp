#include "ui/UiManager.h"

namespace tipsy::ui {

UiManager::UiManager(UiBridge& uiBridge) : display_(), uiBridge_(uiBridge) {}

bool UiManager::begin() {
  lv_init();
  display_.begin();
  uiBridge_.begin();
  squareLineAdapter_.bind(uiBridge_);
  squareLineAdapter_.begin();
  ready_ = true;
  return true;
}

void UiManager::update() {
  if (!ready_) {
    return;
  }

  // Keep the UI-facing snapshot fresh before LVGL processes timers and callbacks.
  uiBridge_.syncFromMachine();
  squareLineAdapter_.update();
  lv_timer_handler();
}

void UiManager::tick(std::uint32_t elapsedMs) {
  lv_tick_inc(elapsedMs);
}

bool UiManager::registerDisplayDriver() {
  // Future work: bind LVGL draw buffers and TFT flush callbacks here.
  displayDriverRegistered_ = false;
  return displayDriverRegistered_;
}

bool UiManager::registerInputDriver() {
  // Future work: bind touch, encoder, or button input drivers here.
  inputDriverRegistered_ = false;
  return inputDriverRegistered_;
}

bool UiManager::isReady() const {
  return ready_;
}

}  // namespace tipsy::ui
