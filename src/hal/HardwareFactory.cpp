#include "hal/HardwareFactory.h"

#include "hal/esp32/LittleFsFileSystem.h"
#include "hal/esp32/SystemTimeProvider.h"
#include "hal/mock/MockFileSystem.h"
#include "pumps/Esp32PumpDriver.h"
#include "pumps/MockPumpDriver.h"

namespace tipsy::hal {

HardwareBundle HardwareFactory::create() {
  HardwareBundle bundle;

#if TIPSY_USE_MOCK_HAL
  bundle.pumpDriver.reset(new tipsy::pumps::MockPumpDriver());
  bundle.fileSystem.reset(new MockFileSystem());
#else
  bundle.pumpDriver.reset(new tipsy::pumps::Esp32PumpDriver());
  bundle.fileSystem.reset(new LittleFsFileSystem());
#endif

  bundle.timeProvider.reset(new SystemTimeProvider());
  return bundle;
}

}  // namespace tipsy::hal
