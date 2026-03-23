#pragma once

#include <Arduino.h>

#include "hal/interfaces/IFileSystem.h"

namespace tipsy::storage {

// Mounts and guards filesystem access so higher layers can fail cleanly.
class FileSystemManager {
 public:
  explicit FileSystemManager(tipsy::hal::IFileSystem& fileSystem);

  bool begin();
  bool isMounted() const;
  bool exists(const char* path) const;
  String readText(const char* path) const;
  bool writeText(const char* path, const String& content) const;
  const String& lastError() const;

 private:
  tipsy::hal::IFileSystem& fileSystem_;
  bool mounted_ = false;
  mutable String lastError_;
};

}  // namespace tipsy::storage

