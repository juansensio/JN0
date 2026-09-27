# MVP roadmap

## Current baseline

The source roadmap baseline was **ROADMAP DEFINED**, implementation not started. M0 is now **DONE**: 8/8 tasks complete and 5/5 gates passed, including user-confirmed hosted CI success. M1 is **DONE** with all four exit gates demonstrated locally. M2 is now **DONE** with all five exit gates demonstrated locally. M3 is now **DONE** with all four exit gates demonstrated locally. M4 is active and **NOT STARTED**; its next action is to define the authoritative protocol and match-server boundary.

Only mark a milestone done when every exit gate is demonstrated. M1–M6 follow sequential dependencies. M7 can begin after M2 and run alongside M3–M6. M8 can begin after M3. M9 requires M3–M8 to be complete. M0–M3 are closed; M4 may begin. M7 is now dependency-eligible. Later milestones retain their documented dependencies.

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

**Status: DONE.**

**Depends on M1.** Create the simulator CLI, RandomBot, HeuristicBot, batch execution, and reproducible benchmarks. Add workers/threads where they improve performance.

**Exit gates:** no Godot linkage or window; at least thousands of consecutive games without errors or evident memory leaks; benchmarks report games/s and worker scaling through the practical limit; RandomBot vs RandomBot completes automatically; HeuristicBot beats RandomBot under a defined statistical evaluation.

**Evidence:** [M2 protocol and validation](log/m2-validation.md). Release and sanitized Debug checks 8/8 PASS; one-million-game runs through all 12 hardware threads with identical results/checksums; approximately flat peak RSS from 10,000 to one million games; 100,000-game sanitized stress with no diagnostics. Fixed 10,000 seat-swapped seed pairs: 90.43% heuristic wins / 90.44% score / 89.2161% one-sided 95% lower score bound. Terminal human-versus-bot play and replay verification also pass.

## M3 Godot offline PvE

**Status: DONE.**

**Depends on M2.** Integrate Godot 4 through GDExtension/godot-cpp. Render cards as rectangles/text. Show hand, board, lives, turn, action selection, match result, and restart. Support human play against both baseline bots.

**Exit gates:** a complete match works offline; Godot contains no game rules; legal actions and game state come exclusively from the core; the same replay gives the same result in headless execution and the client.

**Evidence:** [M3 validation](log/m3-validation.md). Required headless checks 8/8 PASS; optional Godot Debug checks 9/9 PASS; 32 complete real-scene games across both seats/bots, 210 human defenses, and 32/32 exported replays verified headlessly. Exact 62-action fixture parity through Godot import/submission; native input/visibility/atomic-rejection checks PASS. Actual turn/defense/result viewports visually inspected. Rules duplicated outside core 0.

**Pre-M3 prerequisite test:** the optional macOS GDExtension loading/setup smoke
test passes; see [validation](log/pre-m3-godot-smoke.md). This does not start the
offline PvE implementation or pass any M3 exit gate.

## M4 Authoritative online PvP

**Active milestone. Status: NOT STARTED.**

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

## M2 closure — 2026-09-27 / working tree

Active milestone: M2. Status: DONE. Next active milestone: M3 NOT STARTED.

Completed work: seeded observation/legal-action-only RandomBot and deterministic HeuristicBot; reusable simulator library; complete-match replays; deterministic indexed batch work with worker threads; paired statistical evaluation; reproducible throughput/scaling benchmark; CLI human-versus-bot play, bot match, batch, evaluate, benchmark, and replay verification; root README with first-game/manual-test instructions. Core rules and original Word sources unchanged; no Godot or backend implementation.

Evidence: [M2 validation](log/m2-validation.md). Required configure/build/test PASS; Release and warnings-as-errors ASan/UBSan Debug CTest 8/8 PASS. Four terminal input games completed across both seats/bots and their saved replays verified. Fixed evaluation: 20,000 games, 18,086 heuristic wins, 1,910 losses, 4 draws; 90.44% score and 89.2161% lower score bound. One-million-game RandomBot batches run at 1/2/4/8/12 workers with exact action digest/count/result parity. Separate 100,000-game sanitized four-worker stress passed with no diagnostics.

Gates met: headless/no Godot linkage; thousands of consecutive games without errors or evident memory leaks; throughput and worker scaling through hardware limit; automatic RandomBot-vs-RandomBot games; HeuristicBot beats RandomBot under the fixed paired statistical protocol. Gates pending: none for M2.

Problems/debt: local macOS/Apple Clang validation only; hosted Ubuntu CI is configured but has not run on this working tree. macOS LeakSanitizer unavailable; no exhaustive leak-detector claim. Peak RSS from per-child resource accounting is approximately flat (2,129,920 bytes at 10,000 games; 2,146,304 bytes at one million). Timing depends on load and hardware; full final scaling measurements are in the validation log. Inference assumes independent pseudorandom seed-pair trials; fixed empirical superiority does not prove strength against all opponents/seeds.

Current metrics: M2 gates 5/5 PASS; Release/Debug checks 8/8 PASS; worker determinism regressions 0; 62,300,744 actions per one-million-game benchmark run; 48,338.6 games/s with one worker and 159,217 games/s with eight workers (best tested, 3.294×); heuristic win rate 90.43%; rules duplicated outside core 0.

Next concrete action: begin M3 by integrating the shared C++ core through GDExtension/godot-cpp, then build a minimal offline Godot view and legal-action input against both bots. M7 may run alongside later milestones, but is not included in M2 closure.

## Pre-M3 smoke test — 2026-09-27 / working tree

Active milestone: M3. Status: NOT STARTED (offline PvE); prerequisite smoke test complete.

Completed work: optional `JANUS_BUILD_GODOT` target (default OFF); native `JanusGame`
Node owning the existing core; macOS extension descriptor; dummy Godot scene with
no gameplay UI or rules. Used the user's existing godot-cpp submodule checkout.

Evidence: [pre-M3 validation](log/pre-m3-godot-smoke.md). Required configure/build/test
PASS, 8/8 existing checks PASS; Debug native extension builds; two headless scene
runs exit 0 with identical seed-42 diagnostics (player 1, 3 lives / 4 hand / 8 deck /
0 board per seat, 4 legal actions). A second editor import exits 0.

Gates met: none of M3's full-game gates. Gates pending: complete offline PvE,
observation/legal-action UI against both bots, zero duplicated rules audit for that
implementation, and complete-match replay parity.

Problems/debt: first editor import created the cache but crashed with signal 11 on
exit; the subsequent import and both game runs succeeded. Cause unresolved; no
interactive editor run verified. The descriptor is macOS-only, as requested.

Current metrics: smoke scene 2/2 runs PASS; existing checks 8/8 PASS; gameplay rules
added outside core 0. Setup output equality is not full-match replay parity.

Next concrete action: when starting M3, expose player Observation and legal actions
through the adapter, then implement the minimal offline view/input against both bots.

Documentation follow-up: added [the C++ ↔ Godot integration guide](godot-integration.md)
covering current loading/ownership/binding behavior and the future feature workflow.
Planned interfaces are explicitly distinguished from implemented smoke methods.
No rules, interfaces, or milestone gates changed; M3 remains NOT STARTED.

## M3 closure — 2026-09-27 / working tree

Active milestone: M3. Status: DONE. Next active milestone: M4 NOT STARTED.

Completed work: optional macOS Godot 4.7 adapter for owned observations, legal
actions, validated inputs, existing baseline bots, full-range seeded reset, and
atomic core-codec replay import/export; minimal offline card UI with hand/boards,
lives/counts, turn/defense, legal-action buttons, result, restart, both seats/bots,
and save/load. Core rules and original source documents unchanged.

Evidence: [M3 validation](log/m3-validation.md). Required root configure/build/test
PASS, Release 8/8. Extension Debug configure/build PASS and CTest 9/9. Real-scene
button automation completes 32 offline matches (seeds 0–7, both bots/seats),
including 210 human defense selections. All 32 saved replays pass CLI execution
and expected-result checks; both-seat observation roundtrip PASS. Specification
fixture matches all final visible fields and exact encoded replay through Godot
import and 62 submitted actions. Native malformed/overflow/illegal/terminal input,
hidden-information/owned-value, and atomic replay checks PASS. Actual game turn,
defense, and result viewports rendered and visually inspected.

Gates met: complete offline match; no Godot rules; exclusively core-provided
observation/legal choices and validation; headless/client replay result parity.
Gates pending: none for M3.

Problems/debt: local macOS validation only; other platforms await M8. Hosted CI
not executed for this tree; no hosted Godot CI. Restricted sandbox emits Godot
platform certificate/settings errors; final Godot checks ran with approved normal
macOS access and clean logs. Historical first-import crash did not recur; cause
remains unknown. Imported replay is a static verified view, single save slot,
no native hot reload, and partial import does not restore bot RNG continuation.
No manual human mouse-play session claimed; real scene/button paths are automated.

Current metrics: M3 gates 4/4 PASS; headless Release checks 8/8 PASS; Godot Debug
checks 9/9 PASS; full UI games 32/32; generated client replay parity 32/32;
fixture parity PASS; integration assertion failures 0; duplicated rules 0.

Next concrete action: begin M4 with versioned action/observation/result messages
and the authoritative match-server integration. M7 and M8 are dependency-eligible;
no later milestone is implemented or closed here.
