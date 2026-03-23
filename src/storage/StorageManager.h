#pragma once

#include <Arduino.h>

#include "storage/FileSystemManager.h"
#include "storage/JsonStorage.h"

namespace tipsy::storage {

// Coordinates storage startup and shared access to mounted filesystem services.
class StorageManager {
 public:
  StorageManager(FileSystemManager& fileSystemManager, JsonStorage& jsonStorage);

  bool begin();
  FileSystemManager& fileSystem();
  JsonStorage& json();
  const String& lastError() const;

 private:
  FileSystemManager& fileSystemManager_;
  JsonStorage& jsonStorage_;
  String lastError_;
};

}  // namespace tipsy::storage
