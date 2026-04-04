# Tipsy Simulation

This is a console-first simulation path for validating Tipsy product logic before ESP32 hardware arrives.

It does not require:
- LVGL window runtime
- TFT display hardware
- pump hardware

It does reuse the backend logic for:
- recipe availability
- alcohol override behavior
- pump mapping
- prime pumps
- flush / cleaning
- mock pump timing

## Build

```powershell
cd C:\Users\Hjalmar\Tipsy2.0
py -m platformio run -e tipsy-sim-win
```

## Run

```powershell
.\.pio\build\tipsy-sim-win\program.exe
```

Or run both build and execution:

```powershell
powershell -ExecutionPolicy Bypass -File .\simulation\run_simulation.ps1
```

## Included scenarios

- available mixed drink at 4 cl
- available mixed drink at 8 cl
- shot at 6 cl
- pump mapping affecting visible availability
- unavailable drink because of mapping
- prime pumps
- flush / cleaning
