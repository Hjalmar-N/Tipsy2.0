# Agent Notes

Read [README.md](/C:/Users/Hjalmar/Tipsy2.0/README.md) and [AGENT_RULES.md](/C:/Users/Hjalmar/Tipsy2.0/AGENT_RULES.md) before making changes.

This repo has three distinct paths:

- Real firmware: `esp32-s3-devkitc-1`
- Desktop preview: `desktop-preview-win`
- Simulation: `tipsy-sim-win`

Treat them as separate targets with different purposes. Do not assume the desktop preview or simulation proves real hardware readiness.

Some hardware-facing pieces are still placeholder or unfinished, especially around real pump control, finalized board mappings, and LVGL display/input bring-up. Keep that distinction explicit in code and docs.

Prefer small, safe, repo-aligned changes. Avoid broad refactors, avoid duplicating existing rules, and preserve any working separation between firmware, preview, and simulation unless the task clearly requires otherwise.
