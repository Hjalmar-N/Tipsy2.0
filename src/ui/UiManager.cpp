#include "ui/UiManager.h"

namespace tipsy::ui {

UiManager::UiManager(UiBridge& uiBridge) : display_(nullptr), uiBridge_(uiBridge) {}

bool UiManager::begin() {
  lv_init();
  
#if TIPSY_USE_MOCK_HAL
  display_ = new MockDisplayDriver();
#else
  // Must unlock the display controller out of hardware reset first
  if (!tipsy::config::wakeupTca9554LcdReset()) {
    return false;
  }
  tipsy::config::enableDisplayBacklight();
  display_ = tipsy::config::createDisplayDriver();
#endif

  if (display_ == nullptr) {
    return false;
  }

#if !TIPSY_USE_MOCK_HAL
  if (!display_->begin()) {
    log_printf("Display driver begin() failed.\n");
    return false;
  }
  display_->invertDisplay(tipsy::config::displayColorModeUsesInversion());
  log_printf("Display color test mode active: %s\n",
             tipsy::config::displayColorModeName());
#else
  display_->begin();
#endif

  if (!registerDisplayDriver()) {
    return false;
  }

  if (!registerInputDriver()) {
    return false;
  }

  uiBridge_.begin();
  squareLineAdapter_.bind(uiBridge_);
  squareLineAdapter_.begin();
  ready_ = true;
  return true;
}

void UiManager::tick(std::uint32_t elapsedMs) {
  lv_tick_inc(elapsedMs);
}

#if !TIPSY_USE_MOCK_HAL
namespace {
// Sizing now driven entirely by src/config/DisplaySetup.h
constexpr std::size_t kDrawBufferSize = (kTftWidth * kTftHeight) / 4;
std::uint8_t drawBuffer[kDrawBufferSize * 2]; // LVGL9 uses bytes. *2 for RGB565
constexpr std::uint8_t kFt6336Address = 0x38;
constexpr std::uint8_t kFt6x36DataStartRegister = 0x00;
constexpr std::uint8_t kFt6x36VendorRegister = 0xA8;
constexpr std::uint8_t kFt6x36ChipIdRegister = 0xA3;
constexpr std::uint8_t kFt6x36FirmwareRegister = 0xA6;
constexpr lv_coord_t kTouchPortraitWidth = kTftWidth;
constexpr lv_coord_t kTouchPortraitHeight = kTftHeight;

bool readTouchRegister(std::uint8_t reg, std::uint8_t& value) {
  Wire.beginTransmission(kFt6336Address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const std::uint8_t bytesRead = Wire.requestFrom(kFt6336Address, static_cast<std::uint8_t>(1));
  if (bytesRead != 1 || Wire.available() == 0) {
    return false;
  }

  value = Wire.read();
  return true;
}

bool readTouchDataBlock(std::uint8_t reg, std::uint8_t* buffer, std::uint8_t length) {
  if (buffer == nullptr || length == 0) {
    return false;
  }

  Wire.beginTransmission(kFt6336Address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const std::uint8_t bytesRead = Wire.requestFrom(kFt6336Address, length);
  if (bytesRead != length) {
    return false;
  }

  for (std::uint8_t i = 0; i < length; ++i) {
    if (Wire.available() == 0) {
      return false;
    }
    buffer[i] = Wire.read();
  }

  return true;
}

bool readTouchPoint(lv_point_t& point) {
  std::uint8_t buffer[16] = {0};
  if (!readTouchDataBlock(kFt6x36DataStartRegister, buffer, sizeof(buffer))) {
    return false;
  }

  const std::uint8_t touchCount = buffer[2] & 0x0F;
  if (touchCount == 0 || touchCount == 0x0F) {
    return false;
  }

  const lv_coord_t rawX = static_cast<lv_coord_t>(((buffer[3] & 0x0F) << 8) | buffer[4]);
  const lv_coord_t rawY = static_cast<lv_coord_t>(((buffer[5] & 0x0F) << 8) | buffer[6]);

  // With portrait rotation the FT6336 data aligns to the screen axes directly.
  point.x = static_cast<lv_coord_t>(LV_CLAMP(0, rawX, kTouchPortraitWidth - 1));
  point.y = static_cast<lv_coord_t>(LV_CLAMP(0, rawY, kTouchPortraitHeight - 1));
  return true;
}

void logFt6336Probe() {
  log_printf("Touch probe: SDA=%d SCL=%d addr=0x%02X\n",
             kDisplayCtrlI2cSdaPin, kDisplayCtrlI2cSclPin, kFt6336Address);

  Wire.begin(kDisplayCtrlI2cSdaPin, kDisplayCtrlI2cSclPin);
  Wire.setClock(400000);
  delay(5);
  Wire.beginTransmission(kFt6336Address);
  if (Wire.endTransmission() != 0) {
    log_printf("FT6336 not detected on SDA=%d SCL=%d addr=0x%02X\n",
               kDisplayCtrlI2cSdaPin, kDisplayCtrlI2cSclPin, kFt6336Address);
    return;
  }

  log_printf("FT6336 responded on SDA=%d SCL=%d addr=0x%02X\n",
             kDisplayCtrlI2cSdaPin, kDisplayCtrlI2cSclPin, kFt6336Address);

  std::uint8_t vendorId = 0;
  std::uint8_t chipId = 0;
  std::uint8_t firmwareVersion = 0;
  const bool vendorOk = readTouchRegister(kFt6x36VendorRegister, vendorId);
  const bool chipOk = readTouchRegister(kFt6x36ChipIdRegister, chipId);
  const bool firmwareOk = readTouchRegister(kFt6x36FirmwareRegister, firmwareVersion);

  if (vendorOk || chipOk || firmwareOk) {
    log_printf("FT6336 identity: vendor=%s0x%02X chip=%s0x%02X fw=%s0x%02X\n",
               vendorOk ? "" : "ERR:",
               vendorId,
               chipOk ? "" : "ERR:",
               chipId,
               firmwareOk ? "" : "ERR:",
               firmwareVersion);
  } else {
    log_printf("FT6336 responded but identity registers could not be read.\n");
  }
}

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
  static_cast<void>(indev);
  lv_point_t point = {0, 0};
  if (readTouchPoint(point)) {
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = point.x;
    data->point.y = point.y;
    return;
  }

  data->state = LV_INDEV_STATE_RELEASED;
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
  logFt6336Probe();
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

void UiManager::update() {
  if (!ready_) {
    return;
  }

  // Keep the UI-facing snapshot fresh before LVGL processes timers and callbacks.
  uiBridge_.syncFromMachine();
  squareLineAdapter_.update();
  lv_timer_handler();
}

}  // namespace tipsy::ui
