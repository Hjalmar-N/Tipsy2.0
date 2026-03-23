#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "storage/FileSystemManager.h"

namespace tipsy::storage {

// Shared JSON helper used by services to keep raw file I/O separated from domain logic.
class JsonStorage {
 public:
  explicit JsonStorage(FileSystemManager& fileSystemManager);

  bool exists(const char* path) const;
  bool readJson(const char* path, DynamicJsonDocument& doc);
  bool writeJson(const char* path, const DynamicJsonDocument& doc);
  bool ensureFile(const char* path, const DynamicJsonDocument& defaultDoc);
  const String& lastError() const;

 private:
  FileSystemManager& fileSystemManager_;
  mutable String lastError_;
};

}  // namespace tipsy::storage
