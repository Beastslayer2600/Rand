# RAND

Open-World Crime RPG set in Johannesburg CBD & Pretoria. Protagonist: André Venter, a tenderspreneur descending into state-capture-level crime.

**Studio:** Fortitudo Studio · **Engine:** Unreal Engine 5.8 (GDD targets 5.4 — see note) · **Language:** C++

## Building

1. Right-click `RAND.uproject` → **Generate Visual Studio project files** (or run UnrealBuildTool with `-projectfiles`).
2. Open `RAND.sln`, set configuration to **Development Editor / Win64**, build.
3. Or just double-click `RAND.uproject` — the editor offers to compile the module on first open.

## Current state (Milestone 1 — Foundation)

- C++ game module `RAND`.
- `ARANDCharacter` — third-person controller for André. Enhanced Input, authored in C++ (no editor setup needed):
  - **WASD** move (camera-relative), **Mouse** look, **Space** jump, **Left Shift** sprint.
  - Deliberately weighty movement (walk 400 / sprint 650 cm/s) per the GTA IV reference.
- `ARANDGameMode` — spawns André as the default pawn.

## Notes

- **Engine version:** the GDD specifies UE 5.4; the installed engine is **5.8**. `RAND.uproject` is set to `5.8`. To retarget, change `EngineAssociation` and regenerate project files.
- Version control uses Git + Git LFS (`.gitattributes` tracks `.uasset`/`.umap` and binary art/audio).
