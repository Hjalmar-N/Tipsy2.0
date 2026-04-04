#include "storage/JsonStorage.h"

#include <memory>

namespace tipsy::storage {

JsonStorage::JsonStorage(FileSystemManager& fileSystemManager)
    : fileSystemManager_(fileSystemManager) {}

bool JsonStorage::exists(const char* path) const {
  return fileSystemManager_.exists(path);
}

bool JsonStorage::readJson(const char* path, DynamicJsonDocument& doc) {
  const String content = fileSystemManager_.readText(path);
  if (content.isEmpty()) {
    lastError_ = String("File is empty or unreadable: ") + path;
    return false;
  }

  const DeserializationError error = deserializeJson(doc, content.c_str());
  if (error) {
    lastError_ = String("JSON parse failed for ") + path + ": " + error.c_str();
    return false;
  }

  lastError_ = String();
  return true;
}

bool JsonStorage::writeJson(const char* path, const DynamicJsonDocument& doc) {
  const size_t contentLength = measureJson(doc);
  auto buffer = std::make_unique<char[]>(contentLength + 1U);
  const size_t written = serializeJson(doc, buffer.get(), contentLength + 1U);
  buffer[written] = '\0';
  const String content(buffer.get());
  if (!fileSystemManager_.writeText(path, content)) {
    lastError_ = String("JSON write failed for ") + path + ": " +
                 fileSystemManager_.lastError();
    return false;
  }

  lastError_ = String();
  return true;
}

bool JsonStorage::ensureFile(const char* path, const DynamicJsonDocument& defaultDoc) {
  if (fileSystemManager_.exists(path)) {
    lastError_ = String();
    return true;
  }

  return writeJson(path, defaultDoc);
}

const String& JsonStorage::lastError() const {
  return lastError_;
}

}  // namespace tipsy::storage
