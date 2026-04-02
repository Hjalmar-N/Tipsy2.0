#pragma once

#include <Arduino.h>

namespace tipsy::ui {

// Small read-model used by the future UI to render drink choices without touching app internals.
struct UiDrinkItem {
  String id;
  String displayName;
  String subtitle;
  String categoryId;
  bool available = false;
  bool selected = false;
};

}  // namespace tipsy::ui
