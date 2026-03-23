#pragma once

#include <Arduino.h>

namespace tipsy::ui {

// Small read-model used by the future UI to render drink choices without touching app internals.
struct UiDrinkItem {
  String id;
  String displayName;
  bool available = false;
  bool selected = false;
};

}  // namespace tipsy::ui

