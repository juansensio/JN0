# M0 validation and closure — 2026-09-27

Milestone DONE: 8/8 tasks and 5/5 gates pass. Baseline revision `c0c770ff0e324989be55a46a171797981f1b015c`; closure documentation and Makefile fix are subsequent working-tree changes. M1 is active and NOT STARTED.

## Local build evidence

CMake 4.0.3; Apple Clang 21.0.0 (clang-2100.3.34.2); macOS arm64. Fresh build directory:

```sh
cmake -S . -B build-m0-clean -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build-m0-clean --parallel
ctest --test-dir build-m0-clean --output-on-failure
```

Configure/build succeeded; janus_core static library and headless contract_checks executable built. CTest: core_contracts PASS, 1/1, zero failures. Library dependency review: only C++ standard headers and core/src/config.cpp; consumer links janus_core alone. No Godot, rendering, audio, networking, OS-service, backend, or platform linkage. Later systems contain READMEs only. Build output is ignored.

Checks exercise real configuration validation (defaults, version, deck, lives, initial hand, capacity), Action payload identity, default ongoing result, distinct types, and observation value ownership. They do not prove a working game, observation projection, rule correctness, deterministic C++ execution, or replay parser correctness; those are M1 gates.

## Specification gate review

| Gate | Status | Evidence |
| --- | --- | --- |
| G0-1 | PASS | game-rules.md D01–D07, core headers, core-only clean build |
| G0-2 | PASS | Four Action payloads; complete phase table, including Pass/Defend/terminal; reviewed 62-action trace |
| G0-3 | PASS | GameState owns all zones; Observation has only own hand/public values; contract visibility policy and owned-data check |
| G0-4 | PASS | replay.md RNG/setup/encoding/errors/version policy; schema; complete seeded fixture ending in player 0 win |
| G0-5 | PASS | Clean local build/checks; CI workflow; user-confirmed hosted CI pass on 2026-09-27 |

All source-open gameplay questions have explicit decisions. All identity, ordering, configuration, action, rejection, RNG, and replay questions have documented contracts. Open gameplay/contract questions: 0. Original source files unchanged; added choices are identified as M0 decisions. No original combat/life rule was changed.

Fixture JSON parses; schema JSON parses; shapes and full trace were reviewed. A JSON Schema validator is unavailable locally, so no machine schema-validation success is claimed. The independent fixture calculation is specification evidence only; it is not a substitute for M1 replay execution on the C++ core.

## Closure evidence and handoff

On 2026-09-27 the user reported “ci / cd passes” and authorized closing M0. This supplies the previously missing hosted CI evidence for M0-07/G0-5. No run URL was supplied; independent retrieval was unavailable because the GitHub CLI is not installed. Hosted success is explicitly user-reported, not a locally observed run. The M0 gate concerns CI configure/build/test; no deployment capability is claimed.

The added Makefile wraps configure/build/test. Its command targets are now phony, preventing the existing build directory from causing `make build` to skip compilation. Local verification: `make configure`, `make build`, `make test` succeeded; core_contracts 1/1 PASS. Documentation/task tracker/roadmap now agree: M0 DONE, M1 active and NOT STARTED. No gates remain pending.

Nonblocking debt: local machine JSON Schema validation remains unavailable. M1 first implements setup/RNG with seed-0 golden tests, then transitions, visibility, illegal-action tests, and complete-match deterministic replay execution.
