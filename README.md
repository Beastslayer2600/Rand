# RAND

Open-world crime RPG set in Johannesburg CBD and Pretoria. Protagonist: **Andre Venter**, a tenderspreneur who can stay legitimate or slide into state-capture-level crime.

**Studio:** Fortitudo Studio · **Engine:** Unreal Engine 5.8 · **Language:** C++ · **GDD:** `RAND_GDD_v1.docx`

There are no moral guardrails. Every decision is a business decision.

## Building

1. Right-click `RAND.uproject` → **Generate Visual Studio project files**.
2. Open `RAND.sln`, set **Development Editor / Win64**, build.
3. Or double-click `RAND.uproject` — the editor compiles the module on first open.

Needs a NavMesh in the test level for NPC patrol and SAPS pursuit.

## Controls

| Input | Action |
| --- | --- |
| WASD | Move (camera-relative) |
| Mouse | Look |
| Space | Jump |
| Left Shift | Sprint |
| E | Interact |
| Tab / Esc | Phone on / off |
| Left mouse | Fire (after Park Station arms you) |
| F5 / F9 | Save / load |
| F6 | Go dark (R25,000 — clears SAPS, pauses black businesses) |
| F7 | File a SARS return (15% of cash) |

Walk 400 cm/s, sprint 650 cm/s (GTA IV weight).

## Act 1 campaign (playable now)

The campaign director lives on the game mode. It seeds **Venter Consulting & Procurement** plus R125,000 operating capital, then chains:

1. **The Consultation** — Marshalltown tender. Thandi offers R85,000 to rig the committee.
2. **Spec Writing** or **Clean Work** — grey path if you took the bribe; 8% facilitation if you didn't.
3. **Park Station** — Bra Mike and the rank. Pay R40,000 for a corridor seat (grey logistics + sidearm) or walk.
4. **The Braai** — Sipho. Standing, not a shootout.
5. **Hawks at the Door** — Advocate Naidoo. Lawyer up (R120,000) or go dark.

Endings:

- **Stayed Clean** — you didn't take the short road.
- **The City Is Yours** — bribe + cash still standing.
- **Burned** — peak heat hit shoot-on-sight.

Briefings and branches arrive on the in-game WhatsApp thread.

## What this is not

A finished GTA-scale game. Content in-repo is still the test box (`L_TestBox`) and a Manny stand-in. Play Act 1 there. The GDD still needs World Partition cities, MetaHumans, traffic, radio, NPA/Interpol tracks, Act 2-3, audio, and QA.

## Notes

- GDD targets UE 5.4; `RAND.uproject` is associated with **5.8**.
- Git + Git LFS for `.uasset` / `.umap` and binary art/audio.
