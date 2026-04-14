#pragma once

#include <Arduino_GFX_Library.h>
#include <Arduino.h>
#include <Wire.h>
#include <cstdint>

// ==============================================================================
// Tipsy Waveshare ESP32-S3-Touch-LCD-3.5 Board Configuration
// ==============================================================================
// Waveshare's Arduino and schematic docs indicate a TCA9554 IO expander is part of
// the display bring-up path. The vendor pinout and camera example both align the
// board's exported ESP I2C bus to GPIO8/GPIO7; the remaining uncertainty from the
// schematic is the TCA9554 address strapping around U5.

// --- I2C Config (display control expander) ---
constexpr std::int8_t kDisplayCtrlI2cSdaPin = 8;
constexpr std::int8_t kDisplayCtrlI2cSclPin = 7;
constexpr std::uint8_t kTca9554AddressBase = 0x20;
constexpr std::uint8_t kTca9554AddressA2High = 0x24;
constexpr std::uint8_t kTca9554AddressAllHigh = 0x27;
constexpr std::uint8_t kTcaExioLcdAux = 0;
constexpr std::uint8_t kTcaExioLcdRst = 1;     // EXIO1 is LCD_RST

// --- Probe flag: set to 1 to activate AXS15231B/QSPI diagnostic path ---
// Set back to 0 to restore normal ST7796/SPI operation.
#define TIPSY_PROBE_AXS15231B_QSPI 1

// --- QSPI pin constants for AXS15231B probe ---
// Confirmed pin mapping for Waveshare ESP32-S3-Touch-LCD-3.5B.
// RST is -1 because the TCA9554 path handles the hardware reset pulse.
constexpr std::int8_t kQspiCsPin  = 45;
constexpr std::int8_t kQspiSckPin = 47;
constexpr std::int8_t kQspiD0Pin  = 21;
constexpr std::int8_t kQspiD1Pin  = 48;
constexpr std::int8_t kQspiD2Pin  = 40;
constexpr std::int8_t kQspiD3Pin  = 39;

// --- SPI Config (ST7796 Display) ---
// Waveshare's official Arduino_GFX example for this exact board uses:
// MOSI=1, SCLK=5, DC=3, CS=-1, RST=-1, BL=6. The same demo keeps a MISO pin
// defined as GPIO2, while the ESP-IDF factory port marks display MISO as NC.
// Keep GPIO2 here to match the vendor Arduino_GFX path we are using.
constexpr std::int8_t kSpiMosiPin = 1;
constexpr std::int8_t kSpiSckPin  = 5;
constexpr std::int8_t kSpiMisoPin = 2;
constexpr std::int8_t kTftCsPin   = -1;
constexpr std::int8_t kTftDcPin   = 3;
constexpr std::int8_t kTftRstPin  = -1;  // Unused because TCA9554 handles it
constexpr std::int8_t kTftBlPin   = 6;   // Vendor schematic maps LCD_BL to ESP IO6

// --- Screen Properties ---
constexpr std::int32_t kTftWidth  = 320;
constexpr std::int32_t kTftHeight = 480;

// Portrait-first migration target for the Waveshare 3.5" panel.
constexpr std::uint8_t kDisplayRotation = 0;

#if !TIPSY_USE_MOCK_HAL
namespace tipsy::config {

enum class DisplayColorTestMode : std::uint8_t {
  ModeA_InversionOffRgb = 0,
  ModeB_InversionOnRgb,
  ModeC_InversionOffBgr,
  ModeD_InversionOnBgr,
};

constexpr DisplayColorTestMode kDisplayColorTestMode =
    DisplayColorTestMode::ModeD_InversionOnBgr;

inline bool displayColorModeUsesInversion() {
  switch (kDisplayColorTestMode) {
    case DisplayColorTestMode::ModeA_InversionOffRgb:
    case DisplayColorTestMode::ModeC_InversionOffBgr:
      return false;
    case DisplayColorTestMode::ModeB_InversionOnRgb:
    case DisplayColorTestMode::ModeD_InversionOnBgr:
      return true;
  }

  return false;
}

inline bool displayColorModeUsesBgr() {
  switch (kDisplayColorTestMode) {
    case DisplayColorTestMode::ModeA_InversionOffRgb:
    case DisplayColorTestMode::ModeB_InversionOnRgb:
      return false;
    case DisplayColorTestMode::ModeC_InversionOffBgr:
    case DisplayColorTestMode::ModeD_InversionOnBgr:
      return true;
  }

  return false;
}

inline const char* displayColorModeName() {
  switch (kDisplayColorTestMode) {
    case DisplayColorTestMode::ModeA_InversionOffRgb:
      return "Mode A: inversion OFF, RGB";
    case DisplayColorTestMode::ModeB_InversionOnRgb:
      return "Mode B: inversion ON, RGB";
    case DisplayColorTestMode::ModeC_InversionOffBgr:
      return "Mode C: inversion OFF, BGR";
    case DisplayColorTestMode::ModeD_InversionOnBgr:
      return "Mode D: inversion ON, BGR";
  }

  return "Unknown";
}

class TipsyDebugArduino_ST7796 : public Arduino_ST7796 {
public:
  using Arduino_ST7796::Arduino_ST7796;

  void setRotation(uint8_t r) override {
    Arduino_TFT::setRotation(r);

    std::uint8_t madctl = displayColorModeUsesBgr() ? ST7796_MADCTL_BGR : ST7796_MADCTL_RGB;
    switch (getRotation()) {
      case 1:
        madctl |= ST7796_MADCTL_MX | ST7796_MADCTL_MV;
        break;
      case 2:
        madctl |= ST7796_MADCTL_MX | ST7796_MADCTL_MY;
        break;
      case 3:
        madctl |= ST7796_MADCTL_MY | ST7796_MADCTL_MV;
        break;
      default:
        break;
    }

    _bus->beginWrite();
    _bus->writeCommand(ST7796_MADCTL);
    _bus->write(madctl);
    _bus->endWrite();
  }
};

struct DisplayControlBusConfig {
  std::int8_t sdaPin;
  std::int8_t sclPin;
  std::uint8_t address;
};

inline bool probeI2cAddress(std::int8_t sdaPin, std::int8_t sclPin, std::uint8_t address) {
  Wire.begin(sdaPin, sclPin);
  Wire.setClock(400000);
  delay(5);
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

inline bool readTcaRegister(std::uint8_t address, std::uint8_t reg, std::uint8_t& value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const std::uint8_t bytesRead = Wire.requestFrom(address, static_cast<std::uint8_t>(1));
  if (bytesRead != 1 || Wire.available() == 0) {
    return false;
  }

  value = Wire.read();
  return true;
}

inline bool writeTcaRegister(std::uint8_t address, std::uint8_t reg, std::uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

inline bool resolveDisplayControlBus(DisplayControlBusConfig& resolved) {
  static constexpr DisplayControlBusConfig kCandidates[] = {
      {kDisplayCtrlI2cSdaPin, kDisplayCtrlI2cSclPin, kTca9554AddressBase},
      {kDisplayCtrlI2cSdaPin, kDisplayCtrlI2cSclPin, kTca9554AddressA2High},
      {kDisplayCtrlI2cSdaPin, kDisplayCtrlI2cSclPin, kTca9554AddressAllHigh},
  };

  for (const auto& candidate : kCandidates) {
    log_printf("Display control probe: SDA=%d SCL=%d addr=0x%02X\n",
               candidate.sdaPin, candidate.sclPin, candidate.address);
    if (probeI2cAddress(candidate.sdaPin, candidate.sclPin, candidate.address)) {
      resolved = candidate;
      log_printf("Display control detected on SDA=%d SCL=%d addr=0x%02X\n",
                 resolved.sdaPin, resolved.sclPin, resolved.address);
      return true;
    }
  }

  log_printf("Display control expander not detected on known candidate buses.\n");
  return false;
}

// Instantiates the Arduino_GFX bus and display object using the config constants above.
// Only explicitly create the objects if we have configured the minimum GPIO required.
inline Arduino_GFX* createDisplayDriver() {
  if (kSpiMosiPin == -1 || kSpiSckPin == -1 || kDisplayCtrlI2cSdaPin == -1) {
    return nullptr; // Safety interlock: prevents crashing on unconfigured pins
  }

  Arduino_DataBus* bus = new Arduino_ESP32SPI(
      kTftDcPin, kTftCsPin, kSpiSckPin, kSpiMosiPin, kSpiMisoPin);
      
  Arduino_GFX* gfx = new TipsyDebugArduino_ST7796(
      bus, kTftRstPin, kDisplayRotation, false /* IPS */, 
      kTftWidth, kTftHeight, 0, 0, 0, 0);

  return gfx;
}

// Pull LCD_RST high through the IO expander before SPI display init begins.
inline bool wakeupTca9554LcdReset() {
  DisplayControlBusConfig busConfig{};
  if (!resolveDisplayControlBus(busConfig)) {
    return false;
  }

  std::uint8_t conf = 0;
  if (!readTcaRegister(busConfig.address, 0x03, conf)) {
    log_printf("Display control read failed: config register.\n");
    return false;
  }

  conf &= ~(1 << kTcaExioLcdRst);  // EXIO1 only: confirmed LCD reset line
  if (!writeTcaRegister(busConfig.address, 0x03, conf)) {
    log_printf("Display control write failed: config register.\n");
    return false;
  }

  std::uint8_t output = 0;
  if (!readTcaRegister(busConfig.address, 0x01, output)) {
    log_printf("Display control read failed: output register.\n");
    return false;
  }

  // Pulse only EXIO1 (confirmed LCD_RST line). EXIO0 is left undisturbed:
  // its function on this board is unknown and toggling it alongside reset
  // risks corrupting the panel boot sequence.
  output |= (1 << kTcaExioLcdRst);
  if (!writeTcaRegister(busConfig.address, 0x01, output)) {
    log_printf("Display control write failed: output register high phase.\n");
    return false;
  }

  delay(10);

  output &= ~(1 << kTcaExioLcdRst);
  if (!writeTcaRegister(busConfig.address, 0x01, output)) {
    log_printf("Display control write failed: output register low phase.\n");
    return false;
  }

  delay(10);

  output |= (1 << kTcaExioLcdRst);
  if (!writeTcaRegister(busConfig.address, 0x01, output)) {
    log_printf("Display control write failed: output register final phase.\n");
    return false;
  }

  log_printf("LCD control bit pulsed via expander bit %u.\n", kTcaExioLcdRst);
  log_printf("LCD reset released through expander bit %u.\n", kTcaExioLcdRst);
  delay(120);
  return true;
}

// Waveshare's schematic maps LCD_BL to MCU IO6 through a transistor stage, and
// the vendor demos use a separate brightness init path. Start with a simple
// full-on enable before any future PWM brightness work.
inline void enableDisplayBacklight() {
  if (kTftBlPin < 0) {
    return;
  }

  pinMode(kTftBlPin, OUTPUT);
  digitalWrite(kTftBlPin, HIGH);
  log_printf("LCD backlight enabled on GPIO %d.\n", kTftBlPin);
}

// Instantiates the QSPI bus and AXS15231B display object for the diagnostic probe path.
// Only compiled and callable when TIPSY_PROBE_AXS15231B_QSPI is 1.
inline Arduino_GFX* createAXS15231BQspiDriver() {
  log_printf("[diag][display] AXS15231B/QSPI probe path active\n");
  log_printf("[diag][display] Initializing Arduino_ESP32QSPI"
             " CS=%d SCK=%d D0=%d D1=%d D2=%d D3=%d\n",
             kQspiCsPin, kQspiSckPin,
             kQspiD0Pin, kQspiD1Pin, kQspiD2Pin, kQspiD3Pin);
  Arduino_DataBus* bus = new Arduino_ESP32QSPI(
      kQspiCsPin, kQspiSckPin,
      kQspiD0Pin, kQspiD1Pin, kQspiD2Pin, kQspiD3Pin);

  log_printf("[diag][display] Initializing Arduino_AXS15231B %dx%d RST=-1\n",
             AXS15231B_TFTWIDTH, AXS15231B_TFTHEIGHT);
  return new Arduino_AXS15231B(
      bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */,
      false /* IPS */, AXS15231B_TFTWIDTH, AXS15231B_TFTHEIGHT);
}

} // namespace tipsy::config
#endif
