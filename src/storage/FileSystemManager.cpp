#include "storage/FileSystemManager.h"

namespace tipsy::storage {

FileSystemManager::FileSystemManager(tipsy::hal::IFileSystem& fileSystem)
    : fileSystem_(fileSystem) {}

bool FileSystemManager::begin() {
  mounted_ = fileSystem_.begin();
  lastError_ = mounted_ ? String() : String("Failed to mount LittleFS.");
  return mounted_;
}

bool FileSystemManager::isMounted() const {
  return mounted_;
}

bool FileSystemManager::exists(const char* path) const {
  if (!mounted_) {
    lastError_ = "Filesystem is not mounted.";
    return false;
  }

  return fileSystem_.exists(path);
}

String FileSystemManager::readText(const char* path) const {
  if (!mounted_) {
    lastError_ = "Filesystem is not mounted.";
    return String();
  }

  return fileSystem_.readText(path);
}

bool FileSystemManager::writeText(const char* path, const String& content) const {
  if (!mounted_) {
    lastError_ = "Filesystem is not mounted.";
    return false;
  }

  const bool ok = fileSystem_.writeText(path, content);
  lastError_ = ok ? String() : String("Failed to write file.");
  return ok;
}

const String& FileSystemManager::lastError() const {
  return lastError_;
}

}  // namespace tipsy::storage

