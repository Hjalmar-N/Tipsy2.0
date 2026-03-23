#include "hal/mock/MockFileSystem.h"

namespace tipsy::hal {

bool MockFileSystem::begin() {
  return true;
}

bool MockFileSystem::exists(const char* path) {
  return findEntry(path) != nullptr;
}

String MockFileSystem::readText(const char* path) {
  const Entry* entry = findEntry(path);
  return entry == nullptr ? String() : entry->content;
}

bool MockFileSystem::writeText(const char* path, const String& content) {
  if (path == nullptr || *path == '\0') {
    return false;
  }

  Entry* entry = findEntry(path);
  if (entry == nullptr) {
    for (auto& candidate : entries_) {
      if (!candidate.used) {
        candidate.used = true;
        candidate.path = path;
        candidate.content = content;
        return true;
      }
    }

    return false;
  }

  entry->content = content;
  return true;
}

MockFileSystem::Entry* MockFileSystem::findEntry(const char* path) {
  if (path == nullptr || *path == '\0') {
    return nullptr;
  }

  for (auto& entry : entries_) {
    if (entry.used && entry.path == path) {
      return &entry;
    }
  }

  return nullptr;
}

const MockFileSystem::Entry* MockFileSystem::findEntry(const char* path) const {
  if (path == nullptr || *path == '\0') {
    return nullptr;
  }

  for (const auto& entry : entries_) {
    if (entry.used && entry.path == path) {
      return &entry;
    }
  }

  return nullptr;
}

}  // namespace tipsy::hal
