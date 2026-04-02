#include "ui/UiManager.h"

namespace tipsy::ui {

UiManager::UiManager(UiBridge& uiBridge) : display_(nullptr), uiBridge_(uiBridge) {}

bool UiManager::begin() {
  lv_init();
  
#if TIPSY_USE_MOCK_HAL
  display_ = new MockDisplayDriver();
#else
  // Must unlock the display controller out of hardware reset first
  tipsy::config::wakeupTca9554LcdReset();
  display_ = tipsy::config::createDisplayDriver();
#endif

  if (display_ != nullptr) {
    display_->begin();
  }
  
  registerDisplayDriver();
  registerInputDriver();
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

#if !TIPSY_USE_MOCK_HAL
namespace {
// Sizing now driven entirely by src/config/DisplaySetup.h
constexpr std::size_t kDrawBufferSize = (kTftWidth * kTftHeight) / 10; 
std::uint8_t drawBuffer[kDrawBufferSize * 2]; // LVGL9 uses bytes. *2 for RGB565

void displayFlushCb(lv_display_t * disp, const lv_area_t * area, std::uint8_t * px_map) {
  Arduino_GFX *gfx = static_cast<Arduino_GFX*>(lv_display_get_user_data(disp));
  if (gfx != nullptr) {
    std::uint32_t w = lv_area_get_width(area);
    std::uint32_t h = lv_area_get_height(area);
    gfx->draw16bitRGBBitmap(area->x1, area->y1, reinterpret_cast<std::uint16_t*>(px_map), w, h);
  }
  lv_display_flush_ready(disp);
}

void touchReadCb(lv_indev_t * indev, lv_indev_data_t * data) {
  // TODO: Read from physical I2C touch IC (e.g. GT911 or CST816)
  // bool isPressed = physicalTouchRead(&data->point.x, &data->point.y);
  // data->state = isPressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
}  // namespace
#endif

bool UiManager::registerDisplayDriver() {
#if !TIPSY_USE_MOCK_HAL
  if (display_ == nullptr) return false;

  lv_display_t * disp = lv_display_create(display_->width(), display_->height());
  // Ensure LVGL outputs 16-bit color
  lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
  lv_display_set_user_data(disp, display_);
  lv_display_set_buffers(disp, drawBuffer, nullptr, sizeof(drawBuffer), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(disp, displayFlushCb);
  
  displayDriverRegistered_ = true;
#else
  // Future work: bind LVGL draw buffers and TFT flush callbacks here.
  displayDriverRegistered_ = false;
#endif
  return displayDriverRegistered_;
}

bool UiManager::registerInputDriver() {
#if !TIPSY_USE_MOCK_HAL
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchReadCb);
  inputDriverRegistered_ = true;
#else
  // Future work: bind touch, encoder, or button input drivers here.
  inputDriverRegistered_ = false;
#endif
  return inputDriverRegistered_;
}

bool UiManager::isReady() const {
  return ready_;
}

}  // namespace tipsy::ui
