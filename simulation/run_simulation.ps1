Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

Set-Location "C:\Users\Hjalmar\Tipsy2.0"

py -m platformio run -e tipsy-sim-win
& ".\.pio\build\tipsy-sim-win\program.exe"
