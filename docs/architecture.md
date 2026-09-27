# Architecture

## Boundaries and invariants

All game rules exist once in a pure C++20/23 core. It owns state, turn progression, effects, seeded randomness, legal actions, observations, victory conditions, and results. It must not depend on Godot, rendering, audio, networking, operating-system services, or platform APIs.

Godot 4 owns presentation, input, animation, audio, and platform integration. GDScript is appropriate for UI; game rules must stay in C++. Integrate through GDExtension/godot-cpp.

Offline/PvE runs the core locally and requires no backend. Ranked runs the same core in an authoritative match server. Clients send actions and present permitted observations; they cannot decide results or mutate authoritative state. Never expose the complete hidden-information state as a player's observation.

Simulation advances through actions, not elapsed wall time or FPS. Every random choice uses an explicit seed. The same configuration, seed, and action sequence must reproduce the same result. Headless simulation must run without Godot, rendering, audio, networking, or an FPS limit.

One codebase produces multiple binaries and bindings. Do not fork rules by platform or execution mode. Put Steam, Apple, Google, and Nintendo specifics behind adapters so replacing a client does not require changing core rules or protocol.

## Planned repository layout

The M0 skeleton now tracks these directories with responsibility READMEs. The deterministic C++ core, replay codec/executor, and headless checks are implemented through M1; later systems remain placeholders.

| Path | Responsibility |
| --- | --- |
| `core/` | Reusable C++ rules, state, RNG, actions, observations, serialization, and results |
| `simulator/` | C++ headless CLI for batches, bots, benchmarks, and mass simulation |
| `bots/` | C++ RandomBot and HeuristicBot |
| `bindings/godot/` | GDExtension/godot-cpp integration |
| `client/` | Godot 4 presentation and platform integration |
| `bindings/python/` | Thin Python interface via pybind11 or equivalent |
| `ai/` | Python/PyTorch experiments, evaluation, self-play, and training |
| `match-server/` | Authoritative match process linking the same C++ core |
| `backend/` | TypeScript/Node services; Postgres in M5 |
| `protocol/` | Shared messages and versioning |
| `tests/` | Unit, determinism, integration, and regression tests |
| `infra/` | Build, deployment, and CI scripts |
| `assets/` | Vision's later asset pipeline and runtime assets; masters excluded from distributed builds |

The backend MVP covers minimal identity, profiles, matchmaking, server assignment, rating, leaderboard, history, catalog, inventory, entitlements, and telemetry. Broader progression, seasons, and administration belong to the longer-term vision.

## Core contract defined in M0

M0 defines `GameState`, `Observation`, `Action`, `PlayerId`, `CardId`, `GameResult`, and `GameConfig` in `core/include/janus/`. See [core-contract.md](core-contract.md) for field, visibility, validation, and lifecycle semantics. The C++20 library uses CMake >= 3.20; source lives in `core/src/`, contract checks in `tests/`. It has no external dependencies.

The finalized M0 lifecycle is `Game(config) → reset(seed) → observe(player) → legal_actions(player) → step(action) → result()`, with trusted owned `snapshot()` for debugging. M1 implements all Game methods. `snapshot()` is the owned full-state debugging representation, including ordered private zones and seed, with fieldwise equality for regression checks. Replay JSON encoding/decoding and execution live in `janus/replay.hpp`. Rewards and bindings follow later milestones.

Replays contain configuration, seed, and actor-tagged actions sufficient to reconstruct an entire match. M0 specifies [format/rules version 1](replay.md) and its complete fixture. M1 verifies deterministic C++ execution and exact fixture parity. Verify parity with Godot, server, and Python as integrations arrive. The owned full-state snapshot satisfies M1 debugging needs without adding a state restore API; shared network messages/versioning are defined in M4.

## Bots and AI

Bots choose from core-provided observations and legal actions. Implement random and heuristic C++ baselines before training. Python orchestrates training with PyTorch while C++ continues to simulate games. Python bindings must preserve direct-core results and support vectorized environments without real-time waits.

Use reproducible seeds, checkpoints, and a fixed evaluation protocol. The trained agent must beat RandomBot under that protocol and eventually load into the game as a bot. Exact encoding, algorithm, model loading, and evaluation thresholds remain to be specified.
