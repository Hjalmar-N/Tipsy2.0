# Tipsy2.0

Tipsy2.0 is an ESP32-S3 based cocktail machine project built with PlatformIO and the Arduino framework.
The current codebase focuses on clean architecture, mockable hardware seams, JSON-backed storage, and a future-ready UI integration path for LVGL and SquareLine Studio.

## Current Architecture

### `src/app`
- Application/business logic and state machine orchestration.
- `MachineController` is the central workflow controller for drink selection, manual pour mode, admin mode, maintenance mode, completion, and error handling.
- Services provide storage-backed access to drinks, ingredients, and system settings.

### `src/domain/models`
- Embedded-friendly domain models for drinks, ingredients, pump assignments, calibrations, pour requests, and system settings.
- Models use IDs instead of UI labels as the primary linkage between data sets.

### `src/pumps`
- Non-blocking pump control architecture.
- `IPumpDriver` abstracts real hardware vs mock behavior.
- `PumpController` tracks active timed pours and stops pumps automatically when durations expire.
- `Esp32PumpDriver` is currently a skeleton for future LEDC PWM integration.

### `src/storage`
- LittleFS and JSON storage abstractions.
- `FileSystemManager` mounts and guards the filesystem.
- `JsonStorage` handles raw JSON file reads/writes.
- `StorageManager` coordinates storage startup.

### `src/ui`
- Lightweight UI integration seam for future LVGL + SquareLine work.
- `UiBridge` translates UI events into `MachineController` calls.
- `UiEvents` provides placeholder callback entry points for future generated screens.
- `UiManager` owns LVGL startup and periodic UI processing.

### `src/hal`
- Hardware abstraction support for filesystem, time, and hardware bundle construction.
- Supports mock mode and future real ESP32 hardware mode selection.

### `data`
- Example JSON payloads for drinks, ingredients, pump assignments, and system settings.
- These are examples/reference assets; runtime defaults are currently created in code on first boot.

## Storage Model

The project stores JSON in LittleFS using these paths:

- `/drinks.json`
- `/ingredients.json`
- `/pump_map.json`
- `/settings.json`

On first boot:
- LittleFS is mounted.
- Missing files are created from built-in defaults.
- The application then loads those persisted files into memory.

On later boots:
- Existing JSON files are loaded instead of regenerated.

## Current Placeholder Values

The project intentionally uses temporary placeholder data until hardware is finalized:

- Pump count: 6
- Pump calibration flow rates: placeholder `mlPerSecond` values
- PWM implementation: mock driver by default
- ESP32 LEDC output: skeleton only, pin/channel mapping not finalized
- LVGL display/input integration: not fully implemented yet

## Current Module Flow

Startup currently follows this order:

1. Create hardware bundle
2. Create filesystem and JSON storage managers
3. Create storage-backed services
4. Create `PumpController`
5. Create `MachineController`
6. Create `UiBridge` and `UiManager`
7. Initialize pumps and application

Main loop responsibilities:

1. Run LVGL timer handling
2. Synchronize UI state from application state
3. Update `MachineController`
4. Update `PumpController`

## Known TODOs

- Implement real ESP32-S3 LEDC pin/channel setup in `Esp32PumpDriver`
- Add LVGL display flush, tick source, and input driver integration
- Bind SquareLine-generated callbacks through `UiEvents`
- Decide on simultaneous vs sequenced recipe pours for real hardware
- Add persistent recovery strategy for corrupt JSON files
- Add hardware safety rules, fault handling, and watchdog behavior
- Move to a single authoritative source for default data
- Add validation and tests around first boot, storage errors, and state transitions

## Design Goals

- Clean separation between UI, business logic, pumps, storage, and hardware access
- Non-blocking control flow suitable for embedded runtime
- Compatibility with both mock mode and future real hardware mode
- Future-proof structure for restaurant/admin features and UI growth

