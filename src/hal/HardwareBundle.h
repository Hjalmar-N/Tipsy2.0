#pragma once

#include <memory>

#include "hal/interfaces/IFileSystem.h"
#include "hal/interfaces/ITimeProvider.h"
#include "pumps/IPumpDriver.h"

namespace tipsy::hal {

// Groups the active hardware adapters selected at startup.
struct HardwareBundle {
  std::unique_ptr<tipsy::pumps::IPumpDriver> pumpDriver;
  std::unique_ptr<IFileSystem> fileSystem;
  std::unique_ptr<ITimeProvider> timeProvider;
};

}  // namespace tipsy::hal
