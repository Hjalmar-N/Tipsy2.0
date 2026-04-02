#pragma once

#include <cstdint>

#include <lvgl.h>

#include "ui/UiBridge.h"
#include "ui/SquareLineAdapter.h"

#if !TIPSY_USE_MOCK_HAL
#include "config/DisplaySetup.h"
#endif

namespace tipsy::ui {

#if TIPSY_USE_MOCK_HAL
// Keeps mock builds independent from final board setup.
class MockDisplayDriver {
 public:
  void begin() {}
  std::int32_t width() const { return 320; }
  std::int32_t height() const { return 240; }
};
using UiDisplayDriver = MockDisplayDriver*;
#else
using UiDisplayDriver = Arduino_GFX*;
#endif

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
  UiDisplayDriver display_;
  UiBridge& uiBridge_;
  SquareLineAdapter squareLineAdapter_;
  bool ready_ = false;
  bool displayDriverRegistered_ = false;
  bool inputDriverRegistered_ = false;
};

}  // namespace tipsy::ui
