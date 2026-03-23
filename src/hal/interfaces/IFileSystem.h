#pragma once

#include <Arduino.h>

namespace tipsy::hal {

// Abstracts persistent storage so repositories do not depend on LittleFS directly.
class IFileSystem {
 public:
  virtual ~IFileSystem() = default;

  virtual bool begin() = 0;
  virtual bool exists(const char* path) = 0;
  virtual String readText(const char* path) = 0;
  virtual bool writeText(const char* path, const String& content) = 0;
};

}  // namespace tipsy::hal

