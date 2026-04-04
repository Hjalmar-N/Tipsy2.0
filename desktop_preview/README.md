# Tipsy Desktop Preview

This project now includes a separate Windows desktop preview path that reuses the current LVGL-generated Tipsy UI flow in a native desktop window.

## What It Reuses

- The real generated UI implementation in [src/ui/generated/ui.cpp](/Users/Hjalmar/Tipsy2.0/src/ui/generated/ui.cpp)
- The approved boot animation and menu/detail/settings navigation flow
- The current mock-safe pour interactions
- Pump mapping driven menu availability logic in a small preview-side mock model

## What It Does Not Need

- No ESP32 board
- No TFT display
- No touch hardware
- No real pumps

## Repo-Side State

This desktop preview path is intentionally separated from the ESP32 firmware path:

- The ESP32 environment still builds from the normal `src/` firmware tree
- The desktop preview uses the separate `desktop-preview-win` PlatformIO environment
- The preview reuses the real generated UI implementation instead of a parallel mock screen system

## Local Windows Prerequisites

Codex can prepare the repo, but these machine prerequisites still need to exist locally on Windows:

1. Python launcher `py`
2. PlatformIO installed into that Python environment
3. A native C/C++ toolchain available on `PATH`

The simplest reliable option is:

1. Install PlatformIO:

```powershell
py -m pip install -U platformio
```

2. Install MSYS2 from [https://www.msys2.org/](https://www.msys2.org/)

3. In an MSYS2 UCRT64 shell, install the compiler:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

4. Add this folder to your Windows `PATH`:

```text
C:\msys64\ucrt64\bin
```

5. Verify the compiler is visible from a normal terminal:

```powershell
g++ --version
```

If you prefer Visual Studio Build Tools instead, that can also work, but the repo instructions below assume `g++` is available on `PATH`.

## Windows Build Recovery Steps

If you previously hit stale `.pio` cache problems, clean the preview environment first from a normal terminal in the repo root:

```powershell
cd C:\Users\Hjalmar\Tipsy2.0
if (Test-Path .pio\build\desktop-preview-win) { Remove-Item .pio\build\desktop-preview-win -Recurse -Force }
```

If LVGL still appears to cache an old config include path, also remove the preview library cache:

```powershell
cd C:\Users\Hjalmar\Tipsy2.0
if (Test-Path .pio\libdeps\desktop-preview-win) { Remove-Item .pio\libdeps\desktop-preview-win -Recurse -Force }
```

If the whole native cache looks broken, remove the full `.pio` folder:

```powershell
cd C:\Users\Hjalmar\Tipsy2.0
if (Test-Path .pio) { Remove-Item .pio -Recurse -Force }
```

## Windows Run Instructions

1. Open a normal `cmd.exe` or PowerShell window in the repo root:

```powershell
cd C:\Users\Hjalmar\Tipsy2.0
```

2. Build the preview:

```powershell
py -m platformio run -e desktop-preview-win
```

3. Run the built preview executable:

```powershell
.\.pio\build\desktop-preview-win\program.exe
```

If your PlatformIO installation supports the native exec target, this may also work:

```powershell
py -m platformio run -e desktop-preview-win -t exec
```

## Testable Flow On PC

- Boot animation
- Transition to main menu
- Category tabs: `All`, `Drinks`, `Shots`
- Drink detail screen
- Settings screen
- Back navigation
- Mock pour action
- Prime Pumps / Flush Cleaning mock backend status flow
- Pump Mapping changes affecting visible drinks

## Current Preview Limitations

- It reuses the generated UI directly, not the full embedded `UiManager -> UiBridge -> MachineController` stack
- The preview uses an in-memory mock model for recipe availability and pump mapping
- Real persistence, real pump timing, and real hardware drivers are still not part of the desktop path
- The preview still depends on a working native Windows compiler toolchain; the repo alone cannot provide that
