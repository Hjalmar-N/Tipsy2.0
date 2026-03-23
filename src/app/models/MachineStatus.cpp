#include "app/models/MachineStatus.h"

namespace tipsy::app {

MachineStatus MachineStatus::ok(MachineState state, const String& message) {
  MachineStatus status;
  status.success = true;
  status.code = MachineStatusCode::Ok;
  status.state = state;
  status.message = message;
  return status;
}

MachineStatus MachineStatus::error(MachineStatusCode code, MachineState state,
                                   const String& message) {
  MachineStatus status;
  status.success = false;
  status.code = code;
  status.state = state;
  status.message = message;
  return status;
}

}  // namespace tipsy::app

