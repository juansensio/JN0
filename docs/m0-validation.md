# M0 validation — 2026-09-27 working tree

Milestone IN PROGRESS, not closed. M0-01–06 DONE; M0-07 pending hosted run; M0-08 waiting for closure evidence. Results apply to this working tree, not a published commit.

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
| G0-5 | PENDING | Local build/checks pass; CI workflow exists; successful hosted execution not yet recorded |

All source-open gameplay questions have explicit decisions. All identity, ordering, configuration, action, rejection, RNG, and replay questions have documented contracts. Open gameplay/contract questions: 0. Original source files unchanged; added choices are identified as M0 decisions. No original combat/life rule was changed.

Fixture JSON parses; schema JSON parses; shapes and full trace were reviewed. A JSON Schema validator is unavailable locally, so no machine schema-validation success is claimed. The independent fixture calculation is specification evidence only; it is not a substitute for M1 replay execution on the C++ core.

Hosted CI was not run for these unpublished changes. No push or workflow execution is claimed. After publishing, record the commit and successful workflow URL here, finish M0-07/M0-08 checkboxes, and activate M1 only then. M1 first implements setup/RNG with seed-0 golden tests and proceeds to transitions/visibility/replays.
