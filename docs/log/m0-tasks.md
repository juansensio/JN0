# M0 implementation tasks

M0 turns the MVP rules into an implementable specification and prepares a minimal C++ monorepo. This task list expands the [roadmap's M0](roadmap.md#m0-spec-and-monorepo); it does not change its exit gates or implement M1 gameplay.

## Progress status

Last updated: 2026-09-27. **Milestone status: DONE. Tasks complete: 8/8. Exit gates passed: 5/5.** Local contracts/build pass; successful hosted CI is confirmed by the user on 2026-09-27. See [validation evidence](m0-validation.md).

| ID | Task | Depends on | Status | Evidence |
| --- | --- | --- | --- | --- |
| M0-01 | Resolve and formalize game rules | None | DONE | game-rules.md D01–D07; worked examples |
| M0-02 | Create monorepo skeleton | None | DONE | Tracked responsibility READMEs; CMakeLists.txt; .gitignore |
| M0-03 | Define state, configuration, and observations | M0-01, M0-02 | DONE | types.hpp; core-contract.md; clean compile |
| M0-04 | Define actions and game lifecycle | M0-03 | DONE | game.hpp; phase table; complete trace |
| M0-05 | Specify deterministic RNG and replay | M0-04 | DONE | replay.md; schema; 62-action seed-0 fixture/review |
| M0-06 | Add minimal C++ build and contract checks | M0-04 | DONE | Clean Release build; core_contracts 1/1 pass |
| M0-07 | Add basic CI and developer commands | M0-05, M0-06 | DONE | Workflow; verified Makefile commands; user-confirmed CI pass |
| M0-08 | Audit M0 exit gates and hand off to M1 | M0-01 through M0-07 | DONE | Closure record in roadmap.md and m0-validation.md |

M0-01 and M0-02 can proceed independently. After M0-04, replay specification and build work can proceed independently. BLOCKED here means a prerequisite is incomplete, not that user intervention is necessarily required.

## M0-01 Resolve and formalize game rules

**Deliverable:** update [game-rules.md](game-rules.md) with a complete rule specification and decision records for additions or interpretations of the source. Keep the original rules intact unless a change is explicitly agreed and documented.

**Completion gates:**

- [x] Confirm the proposed deck and define setup order, seeded shuffle, initial draw, and first-player selection.
- [x] Define turn handoff and attack/defense phases, including which player acts while defense is pending.
- [x] Define surviving-attacker reuse/exhaustion behavior and behavior when no main action is legal.
- [x] Define terminal behavior and any draw/stalemate handling needed by the agreed rules.
- [x] Specify hand/deck/board visibility and pending-combat information for each player.
- [x] Write worked examples for play/draw, all three combat comparisons, direct damage, full board, empty deck, no legal action, and game end.
- [x] Every gameplay question in the current rules document has a recorded answer; none is silently defaulted.

**Evidence to record:** specification sections and decision references; remaining open gameplay questions = 0. Contract questions move into M0-03 through M0-05 and must also be resolved before milestone closure.

## M0-02 Create monorepo skeleton

**Deliverable:** tracked directories for `core/`, `simulator/`, `bots/`, `bindings/godot/`, `bindings/python/`, `client/`, `ai/`, `match-server/`, `backend/`, `protocol/`, `tests/`, and `infra/`.

**Completion gates:**

- [x] Each required directory contains a short responsibility README or another meaningful tracked file; empty local directories alone do not count.
- [x] Choose and record C++20 or C++23 and a minimal build system consistent with a reusable core library.
- [x] Define public-header, implementation, and test locations for the core.
- [x] Ignore generated build output and local tooling artifacts without excluding source or tests.
- [x] Structure and dependency direction match [architecture.md](architecture.md); no engine/backend dependencies are introduced into the core.

**Evidence to record:** repository tree, build-tool decision, and boundary review. Later systems need placeholders only at M0.

## M0-03 Define state configuration and observations

**Deliverable:** compilable C++ declarations and accompanying field semantics for `PlayerId`, `CardId`, `GameConfig`, `GameState`, `Observation`, and `GameResult`.

**Completion gates:**

- [x] Define player/card identity, card values, container ordering, and relevant numeric constraints.
- [x] `GameConfig` captures replay-relevant setup/rule choices with explicit defaults and validation semantics.
- [x] `GameState` represents hands, decks, boards, lives, active player, phase, pending combat, and result as required by the agreed specification.
- [x] `Observation` is a separate type with a field-by-field visibility policy; it does not expose opponent private data or hidden deck order through state references.
- [x] `GameResult` represents ongoing/terminal outcomes, including draws if the agreed rules permit them.
- [x] All declarations are independent of Godot, networking, OS APIs, and platform services.

**Evidence to record:** header paths, field/visibility documentation, and compile results from M0-06.

## M0-04 Define actions and game lifecycle

**Deliverable:** C++ declarations for `Action` and the game API, plus a transition table specifying phase, acting player, allowed action, validation conditions, resulting phase, and turn handoff.

**Completion gates:**

- [x] Define concrete payloads for play, attack, defense, and any additional action required by M0-01.
- [x] Specify `reset(seed)`, player observation, legal-action enumeration, `step(action)`, and result access with concrete inputs/outputs.
- [x] Specify caller identity, invalid-action errors, and whether rejection leaves state unchanged.
- [x] Specify action ordering and whether `step` resolves combat/draw/turn changes atomically or requires subsequent actions.
- [x] Every post-reset state transition is driven by a defined action, including defense and any no-legal-action handling. Setup is explicitly covered by reset/config/seed.
- [x] Describe an entire match trace from reset to terminal result without engine callbacks, UI timing, or backend decisions.

**Evidence to record:** API headers, complete transition table, and a reviewed full-match specification trace. Gameplay implementation remains M1.

## M0-05 Specify deterministic RNG and replay

**Deliverable:** a replay schema and example fixture, with documented RNG and compatibility semantics.

**Completion gates:**

- [x] Specify RNG algorithm, seed type, shuffle algorithm, and random-consumption ordering so reproduction does not depend on unspecified library behavior.
- [x] Replay includes configuration, explicit seed, ordered actions and actor identity where needed, plus rules/format version information.
- [x] Define stable encoding for card/player identifiers and action payloads.
- [x] Define malformed-input, illegal-sequence, and incompatible-version handling.
- [x] Supply a complete example trace ending in a result, including a defender choice, with expected outcome documented.
- [x] Cross-check the fixture against the rules and transition table: no hidden manual choice or omitted input is needed to reconstruct the match.

**Evidence to record:** schema/specification and fixture paths, version policy, and trace review. Executing replay through the completed game core and proving repeatability belong to M1; cross-runtime parity belongs to later milestones.

## M0-06 Add minimal C++ build and contract checks

**Deliverable:** a minimal reusable core library target, a small headless consumer/check target, and initial automated checks.

**Completion gates:**

- [x] A clean configure/build compiles core declarations under the selected C++ standard.
- [x] A headless consumer includes the public API and links the library without Godot or other engine dependencies.
- [x] Initial checks exercise actual M0 artifacts, such as identifier/configuration contracts and separate observation/state types, without pretending unimplemented gameplay passes.
- [x] No placeholder gameplay implementation is treated as completed rules, determinism, or replay validation.
- [x] Build artifacts stay outside tracked source; the core target introduces no rendering, audio, networking, OS-service, or platform dependencies.

**Evidence to record:** exact commands, toolchain/version, clean-build result, test output, and dependency review. Full rule coverage and complete-game tests are M1 gates.

## M0-07 Add basic CI and developer commands

**Deliverable:** basic CI configuration plus documented local setup/build/test commands, using the same checks as M0-06.

**Completion gates:**

- [x] CI starts from a clean checkout and configures, builds, and runs initial checks without Godot, backend services, or private credentials.
- [x] Build or check failures make CI fail; no required check is silently skipped.
- [x] Required toolchain/build-tool versions and working-directory assumptions are documented.
- [x] Local commands have been run successfully from a clean build directory.
- [x] A successful CI execution is recorded. If hosted CI is unavailable, keep this item pending and record the limitation; configuration alone is not a passing run.
- [x] Update `AGENTS.md` with real build/test guidance once commands exist.

**Evidence to record:** CI configuration path, successful run/log reference, and local verification output. M8 owns the later multiplatform build matrix.

## M0-08 Audit exit gates and hand off to M1

**Deliverable:** a closure record in [roadmap.md](roadmap.md) linking the evidence for each gate and naming the next M1 action.

**Completion gates:**

- [x] M0-01 through M0-07 are DONE with evidence; unresolved gameplay/contract questions = 0.
- [x] All five milestone exit gates below pass on the recorded repository revision.
- [x] Record remaining nonblocking debt separately; no debt invalidates a gate.
- [x] Update this task list and the roadmap together. Mark M0 DONE and activate M1 only after the evidence supports closure.

## Milestone exit gate dashboard

PENDING means the gate has not yet been demonstrated. Use PASS only with evidence; use FAIL when a check actually fails.

| Gate | Required evidence | Contributing tasks | Status |
| --- | --- | --- | --- |
| G0-1 No rule depends on Godot | Rule/API boundary review and core-only build | M0-01–M0-04, M0-06 | PASS |
| G0-2 All game transitions expressible through Action | Complete transition table and full-match action trace | M0-01, M0-04 | PASS |
| G0-3 GameState and Observation clearly separated | Separate C++ types and documented visibility policy | M0-03, M0-06 | PASS |
| G0-4 Replay can represent a complete game | Versioned config + seed + action schema and complete fixture review | M0-05 | PASS |
| G0-5 Minimal C++ project builds with basic CI | Clean local build, passing initial checks, successful CI run | M0-02, M0-06, M0-07 | PASS |

Current metrics: clean build = **PASS**; initial tests = **1/1 PASS**; unresolved rule/contract questions = **0**; complete replay fixture = **62 actions, seed 0, player 0 wins** (specification review only); hosted CI = **PASS (user-confirmed)**. All M0 gates pass; M1 is active and NOT STARTED.

## How to update progress

Use NOT STARTED for ready tasks with no implementation, IN PROGRESS for active work, BLOCKED for an unmet dependency or named obstacle, and DONE only when every task checkbox has evidence. When prerequisites close, move dependent tasks from BLOCKED to NOT STARTED. Change M0 to IN PROGRESS when implementation begins.

Update the summary table, task checkboxes, gate dashboard, and roadmap progress record together. Add an entry below for each meaningful advance:

```text
Date / revision:
Task ID / status:
Completed work:
Evidence paths / commands / result:
Task gates met and pending:
Milestone gates affected:
Open questions / blockers / debt:
Next action:
```

## Implementation evidence — 2026-09-27 / working tree

M0-01 through M0-06 DONE; M0-07 IN PROGRESS; M0-08 BLOCKED by hosted CI execution. [m0-validation.md](m0-validation.md) records commands/toolchain/results and the four passing gate reviews. No M1 gameplay has been implemented. Rule questions = 0; contract questions = 0. JSON files parse, but a JSON Schema validator is unavailable locally; full replay loading/execution remains M1.

The above implementation entry records the state before closure; the following entry supersedes its pending statuses.

## Closure — 2026-09-27

M0-07 and M0-08 DONE. The user confirmed CI/CD passes and authorized M0 closure. Baseline revision: `c0c770ff0e324989be55a46a171797981f1b015c`; closure documentation and Makefile phony-target fix are subsequent working-tree changes. Hosted success is user-reported; no run URL was supplied, and independent retrieval was unavailable because the GitHub CLI is not installed. Evidence attribution is explicit in [m0-validation.md](m0-validation.md).

Makefile configure/build/test commands pass locally. All eight tasks and five gates are complete; unresolved gameplay/contract questions = 0. JSON Schema machine validation remains nonblocking debt; gameplay and deterministic replay execution remain M1 work.

Next concrete action: M1 setup/reset and specified RNG implementation with seed-0 golden tests, then transitions, observation projection, illegal-action tests, and complete-match replay verification.
