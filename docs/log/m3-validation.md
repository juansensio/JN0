# M3 Godot offline PvE validation — 2026-09-27

Status: **DONE**, all four M3 exit gates demonstrated locally. Rules version 1 and
original Word sources are unchanged. Environment: macOS / Apple Clang 21 / Godot
`4.7.2.stable.official.ed1daf0bf`; existing godot-cpp revision
`507ed9d840c01a3c5b2a39af8bb4000bfac30bf5`, API 4.7.

## Implemented scope

- Optional GDExtension `JanusGame` owns the same `janus::Game`, links the existing
  baseline bots, and exposes owned player observations, legal actions, validated
  submissions, bot steps, seeded restart, and core replay import/export.
- Godot rectangles/text show stable card IDs/values, hand, both boards, lives,
  deck/hand/discard counts, turn, pending defense, result, and restart. Both seats
  and baseline bots are selectable. Legal buttons come from core actions.
- Save replaces `user://last-match.json`; load accepts verified partial/complete
  JSON as a static view. Invalid seed/action/replay leaves the match unchanged.
- Seed conversion accepts the full unsigned 64-bit range through decimal strings.
  No rule, native engine dependency, or presentation code was added to core.

## Checks and evidence

Required root `make configure`, `make build`, `make test`: PASS, Release CTest
**8/8**. The headless build still defaults to `JANUS_BUILD_GODOT=OFF`.

Separate Debug `JANUS_BUILD_GODOT=ON` configure/build: PASS. Final CTest **9/9**,
including `godot_offline`, 12.12 seconds locally. The integration runner imports
Godot and uses its actual native binding and actual scene/button callbacks:

```text
M3 Godot checks: 32 complete UI games, 210 human defenses, failures=0
```

The 32 matches use seeds 0–7 × both human seats × both baseline opponents. Human
input is automated through rendered legal-action buttons; opponent actions use the
same method called by the live scene. Every match reaches terminal in fewer than
200 test iterations (a test watchdog, not a game rule), renders a result, disables
terminal actions, exports JSON, reloads through native and UI paths, preserves
both seats' exact observations, and leaves loaded views static. Repeated restart
constructs a new match for each combination.

Each of the 32 exported complete client replays is executed by `janus_cli replay`
with its expected result: **32/32 PASS**. The specification fixture is also loaded
through Godot, then independently submitted action by action through the binding.
Both routes produce the same encoded replay. Golden checks cover all observable
final fields: 62 actions, terminal, player 0 winner, 2/0 lives, empty decks,
ordered hands/boards/discards, and no next actor. The existing direct-core fixture
checks also assert every full-state field.

Boundary checks cover full-range seed/overflow/malformed input, actor/viewer/card
ranges, unsupported action/bot strings, forced-pass/wrong-zone/terminal rejection,
empty terminal legal actions, no opponent hand/deck order/seed in observations,
detached returned arrays, malformed/illegal/result-mismatched replay atomicity,
and partial replay roundtrip. Assertions remain active independently of C++
Release/NDEBUG.

Actual OpenGL/Metal game viewports were rendered and visually inspected at human
turn, pending defense, and win. Artifacts (ignored build output):

- `build/godot/m3-client-turn.png`
- `build/godot/m3-client-defense.png`
- `build/godot/m3-client-result.png`

Readable legal buttons, cards, status, counts, and restart fit the default
1120×820 viewport; smaller windows can scroll. This is actual client rendering,
not a mockup. The graphical rendering helper uses the same scene/actions.
No manual human mouse-play session is claimed.

## Exit gate audit

| M3 gate | Evidence | Status |
| --- | --- | --- |
| Complete match works offline | 32 complete real-scene human-button/bot matches; no backend/account/network operations in the client flow | PASS |
| Godot contains no game rules | Source audit: `main.gd` renders observations/actions; adapter converts/delegates; core owns transitions/combat/draws/legality/results; bots reuse M2 policies | PASS |
| Legal actions and state come exclusively from core | No `snapshot` binding; UI calls `observe(human)` and `legal_actions(human)`, clicks call checked `step`, bots receive their own observations/legal actions | PASS |
| Same replay gives same headless/client result | Exact 62-action fixture through Godot import and submit, plus 32/32 client logs verified by CLI with expected results | PASS |

Rules duplicated outside core: **0**. Client/server/Python cross-platform parity
is not claimed; those integrations belong to later milestones.

## Limitations and debt

Validation is local macOS only; descriptor remains macOS-only as established by
pre-M3. Hosted headless CI exists but has not run on this working tree; hosted
Godot CI is not configured. M8 covers additional platform builds.

Restricted sandbox Godot runs emitted macOS system-certificate/editor-settings
errors despite successful gameplay assertions. Final import, integration suite,
and graphical renders ran with approved normal macOS access and clean logs. No
recurrence of the historical first-import crash was observed in these runs; its
root cause is not claimed resolved.

Replay loading is a verified static view, not an animated playback editor. Native
partial import resets bot policy RNG rather than restoring its historical stream;
the client prevents continuation, so recorded-action replay remains exact.
Save uses a single local slot and overwrites it. Native hot reload is not configured.
These are MVP scope choices, not failed M3 gates.

Next action: M4 authoritative online PvP, beginning with the shared versioned
observation/action/result protocol and match-server boundary. No M4 work is part
of this closure.
