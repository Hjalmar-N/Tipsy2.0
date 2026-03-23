#include "storage/StorageManager.h"

namespace tipsy::storage {

StorageManager::StorageManager(FileSystemManager& fileSystemManager, JsonStorage& jsonStorage)
    : fileSystemManager_(fileSystemManager), jsonStorage_(jsonStorage) {}

bool StorageManager::begin() {
  const bool ok = fileSystemManager_.begin();
  lastError_ = ok ? String() : fileSystemManager_.lastError();
  return ok;
}

FileSystemManager& StorageManager::fileSystem() {
  return fileSystemManager_;
}

JsonStorage& StorageManager::json() {
  return jsonStorage_;
}

const String& StorageManager::lastError() const {
  return lastError_;
}

}  // namespace tipsy::storage
