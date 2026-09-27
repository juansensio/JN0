# Architecture

## Boundaries and invariants

All game rules exist once in a pure C++20/23 core. It owns state, turn progression, effects, seeded randomness, legal actions, observations, victory conditions, and results. It must not depend on Godot, rendering, audio, networking, operating-system services, or platform APIs.

Godot 4 owns presentation, input, animation, audio, and platform integration. GDScript is appropriate for UI; game rules must stay in C++. Integrate through GDExtension/godot-cpp.

Offline/PvE runs the core locally and requires no backend. Ranked runs the same core in an authoritative match server. Clients send actions and present permitted observations; they cannot decide results or mutate authoritative state. Never expose the complete hidden-information state as a player's observation.

Simulation advances through actions, not elapsed wall time or FPS. Every random choice uses an explicit seed. The same configuration, seed, and action sequence must reproduce the same result. Headless simulation must run without Godot, rendering, audio, networking, or an FPS limit.

One codebase produces multiple binaries and bindings. Do not fork rules by platform or execution mode. Put Steam, Apple, Google, and Nintendo specifics behind adapters so replacing a client does not require changing core rules or protocol.

## Planned repository layout

These are target directories; application scaffolding has not yet been created.

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

## Core contract to define in M0

Define `GameState`, `Observation`, `Action`, `PlayerId`, `CardId`, `GameResult`, and `GameConfig`. Separate complete internal state from player-visible observations, and express every game transition through an action.

The MVP conceptual lifecycle is `reset(seed) → observe → legal_actions → step → result`. The vision additionally anticipates `observe(player)`, `legal_actions(player)`, rewards, completion state, cloning, and serialization for AI. These describe requirements, not finalized C++ signatures. Define concrete types and semantics during M0.

Replays contain configuration, seed, and actions sufficient to reconstruct an entire match. Specify the format in M0 and verify parity between direct C++, Godot, server, and Python execution as those integrations arrive. State serialization or an equivalent debugging representation is required in M1; shared network messages/versioning are defined in M4.

## Bots and AI

Bots choose from core-provided observations and legal actions. Implement random and heuristic C++ baselines before training. Python orchestrates training with PyTorch while C++ continues to simulate games. Python bindings must preserve direct-core results and support vectorized environments without real-time waits.

Use reproducible seeds, checkpoints, and a fixed evaluation protocol. The trained agent must beat RandomBot under that protocol and eventually load into the game as a bot. Exact encoding, algorithm, model loading, and evaluation thresholds remain to be specified.
