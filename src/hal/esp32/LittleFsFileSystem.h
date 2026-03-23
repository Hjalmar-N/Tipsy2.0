#pragma once

#include "hal/interfaces/IFileSystem.h"

namespace tipsy::hal {

// Real LittleFS adapter used by repositories for JSON persistence.
class LittleFsFileSystem final : public IFileSystem {
 public:
  bool begin() override;
  bool exists(const char* path) override;
  String readText(const char* path) override;
  bool writeText(const char* path, const String& content) override;
};

}  // namespace tipsy::hal

