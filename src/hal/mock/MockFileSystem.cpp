#include "hal/mock/MockFileSystem.h"

namespace tipsy::hal {

bool MockFileSystem::begin() {
  return true;
}

bool MockFileSystem::exists(const char* path) {
  (void)path;
  return false;
}

String MockFileSystem::readText(const char* path) {
  (void)path;
  return String();
}

bool MockFileSystem::writeText(const char* path, const String& content) {
  (void)path;
  (void)content;
  return true;
}

}  // namespace tipsy::hal

