# M3 tabletop presentation follow-up — 2026-09-27

Status: requested presentation work complete; M3 remains DONE, M4 NOT STARTED.
No core/native rules or replay format changes. Original Word sources unchanged.
The stale M3 status in `AGENTS.md` was aligned with the already-closed M3 in the
roadmap and user instruction; this follow-up does not implement M4.

## Delivered

- Player-relative tabletop with three board slots per seat, bottom fanned hand,
  anonymous opposing backs, mirrored decks/discards and live counts.
- Numeric card faces, hover lift/enlargement, click selection, contextual legal
  confirmation, incoming attacker highlight, forced pass, lives/hand/actor HUD.
- Play/draw, opponent reveal, attack/defense clash, procedural impact, surviving
  card return, destruction to the owner's discard, direct-life and pass feedback.
- Gameplay input and bot pacing gated during animation; session cancellation on
  restart/import. Saving still exports accepted core actions deterministically.
- Small card, hand, board, pile, HUD, settings, table, impact and animation scripts;
  reusable view scenes and [component/debugging documentation](../client-presentation.md).
- 1280 × 900 reference canvas, aspect-preserving window scaling, compact settings.

## Evidence

Executed on local macOS, Godot 4.7.2, existing Debug native extension.

| Check | Result |
| --- | --- |
| Root `make configure`, `make build`, `make test` | PASS; Release 8/8 |
| Full `ctest --test-dir build/godot --output-on-failure` | PASS; Debug/Godot 9/9 |
| Final focused `godot_offline` after combat readability adjustment | PASS; 1/1 |
| Real scene selection/confirmation bulk games | 32/32, both seats and bots; 306 human defenses |
| Generated replays checked by CLI | 32/32 expected-result parity |
| Native boundary, atomic rejection, fixture parity | PASS, existing suite retained |
| Animated scene games | 4/4, both seats and bots; 242 accepted actions |
| Animated combat outcomes | Both survivor directions and ties; all 3 observed |
| Animated action coverage | Play, attack, defend, direct damage, forced pass observed |
| Hover/click/confirmation, input locking | PASS |
| Public zone/count synchronization and anonymous backs | PASS |
| Restart and replay import during movement | PASS; no stale state/action or leftover transient cards |
| Final renderer | Exit 0, clean log; OpenGL compatibility renderer |
| Whitespace check | `git diff --check` PASS |

Rendered and visually inspected `build/godot/tabletop-{turn,selection,small,defense,
defense-selection,combat,result}.png`. Opening/selection and small-window layouts
fit; combat status stays above the clash; discard top cards/counts and empty piles
are visible at result. Screenshots and logs are ignored build artifacts. Core and
native adapter source changes: zero; client combat value comparisons: zero.

## Limitations and next action

Procedural prototype art; no sound or final production assets. Local macOS checks
only; no hosted CI, other platform checks, or manual human mouse-play session is
claimed. Hover and click handlers, selection, and confirmation are automated.
Restricted first runs emitted macOS certificate/settings/log access errors; final
integration/import/render checks used approved normal macOS access and clean logs.
Existing replay-view/single-save-slot limitations remain as documented in M3.

Next: user playtest and visual feedback. The next implementation milestone remains
M4's authoritative protocol/match-server boundary; its gates are unchanged.
