param(
  [switch]$Clean
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

if ($Clean -and (Test-Path ".pio\build\desktop-preview-win")) {
  Remove-Item ".pio\build\desktop-preview-win" -Recurse -Force
}

if (-not (Get-Command py -ErrorAction SilentlyContinue)) {
  throw "Python launcher 'py' was not found. Install Python for Windows first."
}

if (-not (Get-Command g++ -ErrorAction SilentlyContinue)) {
  throw "Native compiler 'g++' was not found on PATH. Install MSYS2 UCRT64 GCC and add C:\msys64\ucrt64\bin to PATH."
}

py -m platformio run -e desktop-preview-win

$exePath = ".\.pio\build\desktop-preview-win\program.exe"
if (-not (Test-Path $exePath)) {
  throw "Build finished but $exePath was not created."
}

& $exePath
