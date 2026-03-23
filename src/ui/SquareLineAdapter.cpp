#include "ui/SquareLineAdapter.h"

#include "ui/generated/ui.h"

namespace tipsy::ui {

void SquareLineAdapter::bind(UiBridge& uiBridge) {
  uiBridge_ = &uiBridge;
}

void SquareLineAdapter::begin() {
  ui_init();
  // Future work: register SquareLine widget callbacks here using uiBridge_ and UiEvents.
  (void)uiBridge_;
}

void SquareLineAdapter::update() {
  if (uiBridge_ == nullptr) {
    return;
  }

  const UiState state = uiBridge_->currentState();
  ui_show_status(state.statusMessage.c_str());
  ui_show_drinks();
}

}  // namespace tipsy::ui
