# MVP roadmap

## Current baseline

The source roadmap baseline was **ROADMAP DEFINED**, implementation not started. M0 is now **DONE**: 8/8 tasks complete and 5/5 gates passed, including user-confirmed hosted CI success. M1 is **DONE** with all four exit gates demonstrated locally. M2 is active and **NOT STARTED**; its next action is to specify the baseline-bot evaluation protocol and implement the headless simulator.

Only mark a milestone done when every exit gate is demonstrated. M1–M6 follow sequential dependencies. M7 can begin after M2 and run alongside M3–M6. M8 can begin after M3. M9 requires M3–M8 to be complete. M0 and M1 are closed; M2 may begin. Later milestones retain their documented dependencies.

## M0 Spec and monorepo

**Task tracker:** [M0 implementation tasks](m0-tasks.md). **DONE**, with 8/8 tasks complete and 5/5 exit gates passed. See [closure evidence](m0-validation.md).

**Work:** create the target directories in [architecture](architecture.md), formalize rules and edge cases, define the core types and lifecycle, and specify replay as config + seed + actions. Add a minimal C++ project and basic CI.

**Exit gates:** no rule depends on Godot; every game transition is representable by `Action`; `GameState` and `Observation` are clearly separated; replay represents a complete game; the minimal C++ project builds with basic CI.

**Evidence:** clean local build PASS; initial tests 1/1 PASS; hosted CI PASS (user-confirmed); unresolved gameplay/contract questions 0. See [M0 validation](m0-validation.md).

## M1 Deterministic C++ core

**Status: DONE.**

**Depends on M0.** Implement setup, seeded shuffle, draws, hands, boards, lives, legal actions, play, attack/defend, combat, direct damage, and terminal results. Add serialization or an equivalent debugging representation and unit/full-game tests.

**Exit gates:** all MVP rules have relevant tests; repeated config + seed + actions gives exactly the same result; a full match runs from tests without Godot; the core rejects illegal actions.

**Evidence:** [M1 validation](log/m1-validation.md). Release and sanitized Debug CTest 2/2 PASS; all rule/illegal-action cases covered; exact 62-action fixture parity; 1,024 complete seeded games / 63,801 actions / zero determinism regressions. The owned full-state snapshot provides the required debugging representation. Hosted CI for these changes has not been executed locally.

## M2 Headless simulator and baseline bots

**Active milestone. Status: NOT STARTED.**

**Depends on M1.** Create the simulator CLI, RandomBot, HeuristicBot, batch execution, and reproducible benchmarks. Add workers/threads where they improve performance.

**Exit gates:** no Godot linkage or window; at least thousands of consecutive games without errors or evident memory leaks; benchmarks report games/s and worker scaling through the practical limit; RandomBot vs RandomBot completes automatically; HeuristicBot beats RandomBot under a defined statistical evaluation.

**Evidence:** single/multi-worker games/s, benchmark game count, heuristic win rate, zero crashes. The exact statistical protocol must be specified.

## M3 Godot offline PvE

**Depends on M2.** Integrate Godot 4 through GDExtension/godot-cpp. Render cards as rectangles/text. Show hand, board, lives, turn, action selection, match result, and restart. Support human play against both baseline bots.

**Exit gates:** a complete match works offline; Godot contains no game rules; legal actions and game state come exclusively from the core; the same replay gives the same result in headless execution and the client.

**Evidence:** full offline game, zero duplicated rules, replay parity pass/fail.

## M4 Authoritative online PvP

**Depends on M3.** Link the core into the match server. Define connection, action, state/observation, and result messages. Connect two clients and support minimal reconnection or clean failure. P2P stays outside this milestone unless a concrete need emerges.

**Exit gates:** two devices/processes complete a server-mediated match; illegal actions are rejected; a modified client cannot decide results or arbitrarily alter state; server results can be reproduced from replay.

**Evidence:** completed online games, zero synchronization errors, zero illegal actions accepted.

## M5 Backend matchmaking and ranked

**Depends on M4.** Use TypeScript/Node and Postgres for guest/minimal identity, profile, matchmaking queue, server creation/assignment, simple Elo or equivalent rating, global leaderboard, and basic history.

**Exit gates:** Play Ranked queues and produces a match without manual intervention; the result updates both players' rating exactly once; the leaderboard reflects it; history identifies the match and result.

**Evidence:** queue success, match completion, test matchmaking time, zero rating-consistency errors.

## M6 Store and inventory without real money

**Depends on M5.** Add backend catalog and account inventory/entitlements, a free claimable SKU such as Test Expansion, and client entitlement synchronization. Prepare payment adapters conceptually for later work.

**Exit gates:** a user claims a free SKU through UI; backend grants exactly one entitlement; it survives closing/reopening the client; it unlocks demonstrable content.

**Evidence:** 100% entitlement consistency, zero duplicate grants, persistence test pass.

## M7 Reinforcement learning and self-play

**Depends on M2; may run alongside M3–M6.** Add thin Python bindings, observation encoding, action masks, vectorized environments, baseline evaluation, initial training/self-play, checkpoints, and reproducible evaluation.

**Exit gates:** Python runs multiple environments without Godot; there are no FPS/real-time waits; Python results match direct C++; a trained agent beats RandomBot under a fixed protocol; the model can later load as a game bot.

**Evidence:** environment steps/s, parallel environments, training steps, win rates against both baselines, reproducibility by seed.

## M8 Multiplatform builds

**Depends on M3.** Produce Windows/macOS/Linux desktop builds where the environment permits, a directly installable Android build, and development iOS on an owned device with an Apple environment. Automate scripts/CI reasonably. Public store fees and Nintendo/Switch are outside this validation.

**Exit gates:** the same commit produces functional desktop, Android, and development iOS builds; all use the same core; offline behavior is equivalent across platforms. Missing platform environments do not count as passing the gate.

**Evidence:** platforms actually executed, build success, platform-specific bugs, zero core forks.

## M9 Integrated MVP

**Depends on M3–M8.** Combine offline human-vs-bot, online ranked, rating/leaderboard, free-entitlement store, a trained AI bot, installable builds, and minimal failure/match/technical-funnel telemetry.

**Exit gates:** a new user completes offline → ranked → ranking → store without manual intervention; server and client use the same rules core; the RL agent plays in-game; the architecture can support Game 2 without a required core rewrite.

**Evidence:** end-to-end completion, blocking errors, crashes, completed matches, zero client/server divergences.

## Tracking development

Before starting work, inspect the active milestone and pending gates. Record evidence for each advance rather than inferring completion from apparent progress. Document new architectural decisions in the roadmap or architecture reference before implementing them. Record nonblocking issues as debt; issues invalidating a gate block closure.

Use this template for progress entries here:

```text
Date / commit:
Active milestone:
Status: NOT STARTED / IN PROGRESS / BLOCKED / DONE
Completed work:
Evidence: tests / benchmark / build / demo / link
Gates met:
Gates pending:
Problems and debt:
Current metrics:
Next concrete action:
```

Track correctness/determinism, simulation throughput, platform parity, networking errors, rating/inventory consistency, AI baseline performance, and duplicated rules outside the core. Do not invent benchmark targets or mark unsupported metrics as passing.

## Progress — 2026-09-27 / working tree

Active milestone: M0. Status: IN PROGRESS.

Completed: explicit rules D01–D07 and edge cases; C++20 monorepo skeleton; separate state/observation/config/result declarations; action/lifecycle contract and transition table; SplitMix64/shuffle specification; JSON replay schema and reviewed complete seed-0 fixture; reusable core target/config validator; headless contract consumer; GitHub CI configuration; developer commands.

Evidence: [M0 validation](m0-validation.md), [contract](core-contract.md), [replay](replay.md), [complete trace](replay-example.md), [task tracker](m0-tasks.md). Fresh Release configure/build succeeded with CMake 4.0.3 and Apple Clang 21; CTest 1/1 passed. No external core dependencies.

Gates met: G0-1 pure-core boundary; G0-2 all transitions represented by Action; G0-3 separate owned observation/state; G0-4 config + seed + actions represents a terminal match (specification review, not M1 execution).

Pending: G0-5 hosted CI execution on the changed revision. Workflow configuration and local checks are complete, but no successful hosted run is available. No milestone closure or M1 activation yet.

Problems/debt: local JSON Schema validator unavailable; schema/fixture JSON parsed and shapes reviewed. Replay loading/execution, deterministic C++ parity, rules, and observation projection are intentionally M1 work, not claimed by contract checks. Original source documents unchanged; additions to unspecified source details are recorded in D01–D07, with no changed original combat/life rules.

Metrics: 6/8 tasks DONE; 4/5 gates PASS; clean build PASS; initial tests 1/1 PASS; unresolved gameplay/contract questions 0; replay fixture 62 accepted actions, seed 0, player 0 wins. No gameplay throughput/platform/network metrics exist yet.

Next action: execute hosted CI for this revision, record run URL and commit, complete M0-07/M0-08 and activate M1. First M1 action: implement reset/setup and specified RNG with seed-0 golden ordering tests before implementing transitions.

## M0 closure — 2026-09-27

Baseline revision: `c0c770ff0e324989be55a46a171797981f1b015c`; closure docs and Makefile fix are working-tree changes. Status: M0 DONE; active milestone M1 NOT STARTED.

Completed: M0-07 hosted CI success confirmed by the user; M0-08 final gate audit and handoff. Added Makefile guidance and declared command targets phony so the build directory cannot suppress compilation.

Evidence: user message “ci / cd passes” on 2026-09-27; [M0 validation](m0-validation.md); local `make configure`, `make build`, and `make test` pass with 1/1 contract checks. Hosted result is user-reported, not independently retrieved; no run URL supplied. All five exit gates PASS; all eight tasks DONE; no unresolved rule/contract questions.

Nonblocking debt: machine JSON Schema validation unavailable locally. Full gameplay, observation projection, replay execution, and deterministic parity are M1 gates and are not claimed complete.

Next action: implement M1 reset/setup and SplitMix64/bounded shuffle, with seed-0 golden deck/starter tests before gameplay transitions.

## M1 closure — 2026-09-27 / working tree

Active milestone: M1. Status: DONE. Next active milestone: M2 NOT STARTED.

Completed work: pure C++20 Game lifecycle; SplitMix64 and unbiased Fisher–Yates setup; draw/play/attack/explicit defense/pass; ordered zones and public discards; lives/win/draw/terminal handling; legal actions and specified error precedence; owned observations and complete debugging snapshots; strict replay JSON parser/encoder and fresh-game executor with indexed legality failures. No rule changes or new rules version; original source documents unchanged.

Evidence: [M1 validation](log/m1-validation.md). Required `make configure`, `make build`, `make test` PASS; CTest 2/2 PASS. Fresh Debug build with warnings as errors and AddressSanitizer/UndefinedBehaviorSanitizer PASS, CTest 2/2 with no diagnostics. The seed-0 fixture executes all 62 accepted actions and matches every final field. Repeated states and JSON-reconstructed states match across 1,024 complete seeded games (822 wins, 202 draws, 63,801 actions).

Gates met: all MVP rules have relevant tests; config + seed + actions reproduces exact state/result; complete matches run headlessly without Godot; illegal actions are rejected without state mutation. Gates pending: none for M1.

Problems/debt: validation is local on Apple Clang 21/macOS; hosted Ubuntu CI is configured to run both tests but has not yet run on these working-tree changes. Godot/server/Python parity belongs to their integration milestones. Standalone JSON Schema tooling remains unavailable; the implemented strict codec validates replay fields/config/ranges and is covered by malformed-input tests. No M2 throughput, bot strength, mass-simulation memory, or platform-parity claims.

Current metrics: M1 gates 4/4 PASS; local Release/Debug test suites 2/2 PASS; determinism regressions 0; exact fixture parity PASS; rules duplicated outside core 0.

Next concrete action: begin M2 by specifying the heuristic evaluation protocol, then implement simulator CLI and observation/legal-action-only RandomBot and HeuristicBot. Do not begin Godot or backend work before dependency gates.
