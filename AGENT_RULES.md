# Tipsy Agent Rules

You are working on **Tipsy**, an ESP32-S3 automated drink-mixing machine with a small touchscreen and LVGL-based UI.

Your job is to help implement and improve the real product safely and incrementally.

## Product-first rule

Treat Tipsy as a **product first**, not just as a codebase.

Before making changes, preserve the approved user experience and machine behavior.

## Approved product behavior

### Boot flow
- On startup, show a Tipsy logo / boot animation
- Then transition automatically to the main menu

### Main menu
- POS / restaurant style drink grid
- Category filters:
  - All
  - Drinks
  - Shots
- Drink cards must be easy to tap on a small touchscreen
- Settings must be reachable from the main UI

### Drink detail flow
- User selects a drink from the grid
- Then enters a detail screen for that drink
- Only these strength options are valid:
  - 4 cl / 40 ml
  - 6 cl / 60 ml
  - 8 cl / 80 ml
- There must be **no S / M / L size concept anywhere**

### Drink logic
- For mixed drinks:
  - only the alcohol amount changes when switching between 4 cl, 6 cl, and 8 cl
  - mixers stay fixed
- For shots:
  - the selected amount is the full pour

### Settings behavior
Settings should include at least:
- Pump Mapping
- Calibration
- Prime Pumps
- Flush / Cleaning
- Admin Lock
- Service Mode

### Critical product rule
Pump mapping must affect menu availability.

If a drink requires an ingredient that is not mapped to an enabled pump, that drink must be unavailable or blocked from pouring.

---

## Architecture rules

### Backend is the source of truth
The backend must own:
- recipe resolution
- alcohol override logic
- shot logic
- ingredient availability
- pump mapping
- pour planning
- service actions
- persistence of machine-relevant state

### UI responsibilities only
The UI should only own:
- rendering
- navigation
- user input
- displaying backend state
- forwarding user actions into backend/application logic

### Do not duplicate business logic in the UI
Do not implement or keep local UI-only logic for:
- drink availability
- recipe calculation
- alcohol override behavior
- shot volume logic
- pump mapping decisions
- pour blocking rules

If such logic exists in UI/mock/generated code, prefer removing or redirecting it to backend-owned paths.

### Respect the existing architecture
Prefer working within the current flow:
- UiManager
- SquareLineAdapter
- generated UI layer
- UiBridge
- MachineController
- services / storage / pump logic

Do **not** introduce a parallel UI architecture unless absolutely necessary.

---

## Development direction

### Preferred approach
- Reuse the existing app/backend logic as much as possible
- Keep UI rendering focused on UI concerns
- Keep recipe resolution, availability, mapping, and pour planning in backend paths
- Make the system easy to test before and during hardware bring-up
- Prefer small, safe, reviewable changes

### Not a priority
Do **not** restart or prioritize the Windows native LVGL preview effort unless there is a very strong reason.

The simulation path is acceptable for logic validation.
The firmware path is the priority for real product progress.

---

## Hardware bring-up rules

When working on ESP32-S3 bring-up:
- preserve firmware buildability
- prefer incremental hardware bring-up
- use safe stubs instead of fake success
- avoid enabling real pumps too early
- call out board-specific assumptions clearly
- identify display, touch, filesystem, timing, and driver gaps explicitly

If pump hardware is not yet safe to run:
- keep a compile-time or config-level safe mode
- avoid accidental activation
- keep simulation unaffected

---

## Change management rules

When modifying code:
- make the **smallest safe change**
- avoid broad refactors unless explicitly requested
- do not redesign working architecture unnecessarily
- do not mix unrelated changes in one patch
- preserve mock-safe behavior where useful for testing
- remove duplicate/mock-only logic when real backend ownership already exists

If you are uncertain:
- inspect first
- report uncertainty explicitly
- do not pretend placeholder code is production-ready

---

## Required reporting format

For every substantial task, report clearly:

1. What is fully implemented
2. What is partial
3. What is placeholder
4. What exact files/functions were inspected or changed
5. What risks remain for real ESP32-S3 hardware
6. What the next highest-value step is

If code was changed, also report:
- why the change was made
- how behavior changed before vs after
- what remains unresolved
- whether the change affects simulation, firmware, or both

---

## Guardrails

Do not:
- reintroduce S / M / L sizing
- move business rules into the UI
- create a disconnected mock state system
- revive desktop preview work as a main track
- hide uncertainty
- claim hardware readiness without identifying placeholders

Do:
- preserve approved product behavior
- keep backend authoritative
- make progress toward real hardware testing
- improve clarity between implemented vs partial vs placeholder
- optimize for real product readiness