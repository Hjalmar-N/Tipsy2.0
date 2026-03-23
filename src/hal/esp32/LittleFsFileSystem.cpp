#include "hal/esp32/LittleFsFileSystem.h"

#include <LittleFS.h>

namespace tipsy::hal {

bool LittleFsFileSystem::begin() {
  return LittleFS.begin(true);
}

bool LittleFsFileSystem::exists(const char* path) {
  return LittleFS.exists(path);
}

String LittleFsFileSystem::readText(const char* path) {
  File file = LittleFS.open(path, "r");
  if (!file) {
    return String();
  }

  String content = file.readString();
  file.close();
  return content;
}

bool LittleFsFileSystem::writeText(const char* path, const String& content) {
  File file = LittleFS.open(path, "w");
  if (!file) {
    return false;
  }

  const std::size_t written = file.print(content);
  file.close();
  return written == content.length();
}

}  // namespace tipsy::hal

