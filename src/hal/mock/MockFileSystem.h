#pragma once

#include <array>

#include "config/AppConfig.h"
#include "hal/interfaces/IFileSystem.h"

namespace tipsy::hal {

// Mock filesystem for early testing before persistent storage is finalized.
class MockFileSystem final : public IFileSystem {
 public:
  bool begin() override;
  bool exists(const char* path) override;
  String readText(const char* path) override;
  bool writeText(const char* path, const String& content) override;

 private:
  struct Entry {
    bool used = false;
    String path;
    String content;
  };

  Entry* findEntry(const char* path);
  const Entry* findEntry(const char* path) const;

  std::array<Entry, tipsy::config::kMaxDrinkCount> entries_ {};
};

}  // namespace tipsy::hal
