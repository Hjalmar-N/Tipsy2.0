#pragma once

#include "hal/interfaces/IFileSystem.h"

namespace tipsy::hal {

// Mock filesystem for early testing before persistent storage is finalized.
class MockFileSystem final : public IFileSystem {
 public:
  bool begin() override;
  bool exists(const char* path) override;
  String readText(const char* path) override;
  bool writeText(const char* path, const String& content) override;
};

}  // namespace tipsy::hal

