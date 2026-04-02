#pragma once

#include <Arduino_GFX_Library.h>
#include <Wire.h>

// ==============================================================================
// Tipsy Waveshare ESP32-S3-Touch-LCD-3.5 Board Configuration
// ==============================================================================
// Waveshare's ST7796 display initialization requires pulling LCD_RST high 
// via a TCA9554 IO Expander on the I2C bus before SPI communication can begin.

// --- I2C Config (TCA9554 & Touch) ---
// Waveshare 3.5" TCA9554 version uses GPIO 8 for SDA and GPIO 9 for SCL.
constexpr std::int8_t kI2cSdaPin = 8; 
constexpr std::int8_t kI2cSclPin = 9;
constexpr std::uint8_t kTca9554Address = 0x38; // Default address for this board revision
constexpr std::uint8_t kTcaExioLcdRst = 1;     // EXIO1 is LCD_RST

// --- SPI Config (ST7796 Display) ---
// For this Waveshare board, the display shares the fast IO pins:
constexpr std::int8_t kSpiMosiPin = 11;
constexpr std::int8_t kSpiSckPin  = 12;
constexpr std::int8_t kSpiMisoPin = 13;
constexpr std::int8_t kTftCsPin   = 10;
// NOTE: Waveshare routes LCD_DC to GPIO 9, which is heavily overloaded and often overlaps
// with the I2C SCL line in default Arduino Wire states if not careful.
constexpr std::int8_t kTftDcPin   = 9;
constexpr std::int8_t kTftRstPin  = -1;  // Unused because TCA9554 handles it
constexpr std::int8_t kTftBlPin   = -1;  // Background hardware config handled by board logic

// --- Screen Properties ---
constexpr std::int32_t kTftWidth  = 320;
constexpr std::int32_t kTftHeight = 480;

// Start with one of these, usually 1 or 3 for landscape
constexpr std::uint8_t kDisplayRotation = 1;

#if !TIPSY_USE_MOCK_HAL
namespace tipsy::config {

// Instantiates the Arduino_GFX bus and display object using the config constants above.
// Only explicitly create the objects if we have configured the minimum GPIO required.
inline Arduino_GFX* createDisplayDriver() {
  if (kSpiMosiPin == -1 || kSpiSckPin == -1 || kI2cSdaPin == -1) {
    return nullptr; // Safety interlock: prevents crashing on unconfigured pins
  }

  Arduino_DataBus* bus = new Arduino_ESP32SPI(
      kTftDcPin, kTftCsPin, kSpiSckPin, kSpiMosiPin, kSpiMisoPin);
      
  Arduino_GFX* gfx = new Arduino_ST7796(
      bus, kTftRstPin, kDisplayRotation, false /* IPS */, 
      kTftWidth, kTftHeight, 0, 0, 0, 0);

  return gfx;
}

// Blocks until the TCA9554 is fully woken up over I2C and LCD_RST is pulled high
inline bool wakeupTca9554LcdReset() {
  if (kI2cSdaPin == -1 || kI2cSclPin == -1) return false;

  Wire.begin(kI2cSdaPin, kI2cSclPin);
  
  // Set EXIO1 (LCD_RST) to OUTPUT (clear bit 1 in config register 0x03)
  Wire.beginTransmission(kTca9554Address);
  Wire.write(0x03); 
  Wire.endTransmission();
  Wire.requestFrom(kTca9554Address, (uint8_t)1);
  if (Wire.available() == 0) return false;
  uint8_t conf = Wire.read();
  
  conf &= ~(1 << kTcaExioLcdRst);
  Wire.beginTransmission(kTca9554Address);
  Wire.write(0x03);
  Wire.write(conf);
  Wire.endTransmission();

  // Write HIGH to EXIO1 (set bit 1 in output port register 0x01)
  Wire.beginTransmission(kTca9554Address);
  Wire.write(0x01); 
  Wire.endTransmission();
  Wire.requestFrom(kTca9554Address, (uint8_t)1);
  if (Wire.available() == 0) return false;
  uint8_t output = Wire.read();
  
  output |= (1 << kTcaExioLcdRst);
  Wire.beginTransmission(kTca9554Address);
  Wire.write(0x01);
  Wire.write(output);
  Wire.endTransmission();

  delay(120); // ST7796 requires 120ms to stabilize after reset goes high
  return true;
}

} // namespace tipsy::config
#endif
