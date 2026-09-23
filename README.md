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
| WASD | Move |
| Mouse | Look |
| Space | Jump |
| Left Shift | Sprint |
| E | Interact |
| Tab / Esc | Phone on / off |
| 1 / 2 / 3 | Pick a phone reply |
| Left mouse | Fire (after Park Station or F10) |
| F5 / F9 | Save / load |
| F6 | Go dark (R25,000) |
| F7 | File a SARS return (15% of cash) |
| F8 | Playtest: +R50,000 |
| F10 | Playtest: arm |
| F11 | Playtest: complete current objective |
| F12 | Playtest: clear heat |

Walk 400 cm/s, sprint 650 cm/s (GTA IV weight).

## Campaign now

Director seeds **Venter Consulting & Procurement** plus R125,000 and a tender dossier.

**Act 1**
1. The Consultation — Thandi's R85k committee offer.
2. Spec Writing or Clean Work.
3. Park Station — R40k corridor seat, sidearm, burner.
4. The Braai.
5. Hawks at the Door — lawyer R120k or go dark.

**Act 2** (starts unless you are Burned)
6. City Deep — buy the warehouse for R200k or stay a passenger.
7. Municipal fleet tender.
8. NPA preservation order — bury it for R250k or fight.

**Act 3** (after the NPA order)
9. Ponte — buy the Berea floor for R350k or leave it.
10. Red notice — pay Naidoo R300k to lose Interpol's request, or let it land.
11. Cabinet — take the procurement-council seat or stay a contractor. The ending follows your heat and career stage.

Every four in-game hours a side hustle hits the phone (envelope, letter, taxi package, silence job).

**Tender board.** Each morning at 09:00 the eTender portal posts a municipal tender. Bid clean (needs a CIDB grade that covers the value; win chance comes from B-BBEE preference points on the 80/20 system), pay Thandi's SCM cut to rig the award, or pass. Every two wins lifts the CIDB grade (starts at 2, limits R200k to unlimited at grade 9). Rigging stacks whistle-blower risk, rolled daily: exposure means +30 Hawks heat and a 14-day National Treasury blacklist. From day 3 Cousin Lerato offers a 51% BEE front for R150k: level 1 on paper, and you can keep bidding while blacklisted, but every win through it adds risk. The HUD status line shows CIDB grade, BEE level and any blacklist.

**Factions.** Nigerian networks, Cape Flats Reach, Zim/Moz networks, Eastern European money, the taxi industry and the Zama Zamas each have a standing from -100 to 100 (War, Hostile, Neutral, Partner, Allied). Every six in-game hours one of them calls with a deal: cash for heat, a laundering buy-in that comes back clean, or taxi-rank protection that cools SAPS heat. Refusing costs standing, and the Nigerian networks swing hardest either way. Factions at War add Rivals heat every hour; Allied factions pay a tribute share. Unanswered offers lapse after 12 hours.

New offers wait while an older phone reply is still pending, so side hustles, tenders and faction deals no longer bury campaign choices.

## What this is not

A finished GTA-scale game. Maps are still `L_TestBox` and a Manny stand-in. World Partition cities, MetaHumans, traffic and audio assets are still outstanding. The radio and ambient zones are data-only until sound assets exist.
