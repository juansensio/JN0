# M1 deterministic core validation

Date: 2026-09-27. Revision: working tree. Status: DONE; all four M1 exit gates PASS. M2 is active, NOT STARTED.

## Implementation

The dependency-free C++20 `janus_core` target implements all `Game` methods and the existing rules-v1 contract. Setup uses explicit SplitMix64, unbiased rejection sampling, and canonical Fisher–Yates ordering. No randomness occurs after setup. Every transition uses an actor-tagged Action; illegal actions return the specified error/current result without mutation. Observation contains only owned permitted data. `snapshot()` provides the complete owned debugging representation with fieldwise equality, including config/seed/private zones; no external state mutation/restore is accepted.

`janus/replay.hpp` supplies strict JSON loading/encoding and fresh-game execution, preserving ordered actions, full uint64 decimal-string seeds, optional result assertions, and zero-based illegal-action indices. The core has no engine, network, OS, timing, or file I/O dependency. Rules and source documents are unchanged.

## Gate evidence

| Exit gate | Evidence | Result |
| --- | --- | --- |
| All MVP rules have relevant tests | Setup/RNG goldens; play/draw/empty deck/full board; all combat comparisons; blocked/direct life damage; defense/turn/phase transitions; forced pass/reset/two-pass draw; win/terminal/reset; visibility and ordering; card conservation | PASS |
| Same config + seed + actions gives exactly the same result | 1,024 seeded matches with paired full-state comparison after each action, plus JSON replay reconstruction and exact terminal state comparison | PASS |
| Full match runs without Godot | Actual JSON seed-0 fixture executes 62 actions, matching every final zone/life/counter/result; 1,024 additional complete headless games | PASS |
| Core rejects illegal actions | Explicit error-precedence cases plus every actor (0/1/invalid), payload, and card (0–24) at reachable states in 16 seeded games; rejection compares complete state unchanged | PASS |

Tests: `tests/contract_checks.cpp`, `tests/game_checks.cpp`, `tests/fixtures/replay-v1.json`. Checks throw/report failures in Release/NDEBUG; they do not rely on disabled assertions. Full-game resource bounds are test guards only, not additional game rules or timers. The test-local action picker is correctness tooling, not an M2 bot implementation.

Replay rejection coverage includes malformed/missing/unknown/duplicate fields (including escaped key equivalents), unsupported versions/config, seed syntax/overflow, invalid actor/card/action shape, malformed result shape, incomplete log, result mismatch, and first illegal action. The encoder is tested under a grouping locale to ensure stable decimal JSON output.

## Executed checks

- `make configure`, `make build`, `make test`: Release PASS, CTest 2/2.
- Fresh `build/m1-sanitized` Debug configure/build: PASS with `-Wall -Wextra -Wpedantic -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer. CTest 2/2 PASS; no sanitizer diagnostics.
- Environment: CMake 4.0.3, Apple Clang 21, macOS. No external core libraries, Godot, backend, or credentials.
- Gameplay output: 1,024 complete games; 822 wins, 202 draws; 63,801 accepted actions; zero determinism regressions.
- Exact fixture: terminal win for player 0, lives [2,0], 62 actions, passes 0, active player 0, no pending attack; all ordered zones match `docs/replay-example.md`.

## Limits and handoff

Hosted CI is configured to run both suites; no hosted run on these changes is claimed. Cross-compiler/platform and Godot/server/Python parity remain future evidence. These correctness checks do not establish M2 throughput, thousands-of-games stress/memory gates, or heuristic strength. Standalone JSON Schema validation tooling remains unavailable; replay semantics are enforced by the core codec/executor and regression checks.

Next action: M2 headless simulator and baseline bots, beginning with a defined statistical evaluation protocol. Original Word sources remain preserved.
