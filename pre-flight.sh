#!/usr/bin/env bash

set -euo pipefail

ENVIRONMENT="${1:-esp32-s3-devkitc-1}"

if [[ ! -f "platformio.ini" ]]; then
  echo "pre-flight.sh must be run from the repository root."
  echo "Expected to find: platformio.ini"
  exit 1
fi

echo "Building PlatformIO environment: ${ENVIRONMENT}"
pio run -e "${ENVIRONMENT}"

echo
case "${ENVIRONMENT}" in
  esp32-s3-devkitc-1)
    echo "Build complete for real firmware."
    echo "Next steps:"
    echo "  pio run -e esp32-s3-devkitc-1 -t upload"
    echo "  pio device monitor -b 115200"
    ;;
  desktop-preview-win)
    echo "Build complete for the desktop preview."
    echo "Next steps:"
    echo "  ./.pio/build/desktop-preview-win/program.exe"
    echo "  pio run -e desktop-preview-win -t exec"
    ;;
  tipsy-sim-win)
    echo "Build complete for the simulation."
    echo "Next steps:"
    echo "  ./.pio/build/tipsy-sim-win/program.exe"
    echo "  powershell -ExecutionPolicy Bypass -File ./simulation/run_simulation.ps1"
    ;;
  *)
    echo "Build complete for '${ENVIRONMENT}'."
    echo "Next step:"
    echo "  Inspect ./.pio/build/${ENVIRONMENT}/ for target-specific output."
    ;;
esac
