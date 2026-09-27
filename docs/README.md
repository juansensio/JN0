# Janus Noise development docs

Janus Noise is a shared platform for small competitive games. The first MVP validates the technical stack with a deliberately minimal card game.

Read these references before implementing a feature:

- [Vision and scope](vision.md): product direction, MVP boundaries, and later ambitions.
- [Architecture](architecture.md): responsibilities, dependencies, core interfaces, and repository layout.
- [MVP game rules](game-rules.md): resolved M0 specification and required M1 rule tests.
- [MVP roadmap](roadmap.md): dependencies, work, exit gates, metrics, and progress tracking.
- [M0 implementation tasks](m0-tasks.md): ordered tasks, completion checklists, dependencies, and current gate status.
- [Core contract](core-contract.md): concrete C++ type/lifecycle semantics and validation rules.
- [C++ ↔ Godot integration](godot-integration.md): extension loading, ownership, adapter boundaries, and how to add future features.
- [Replay specification](replay.md) and [worked match](replay-example.md): deterministic RNG, versioned encoding, and complete specification fixture.
- [Development commands](development.md) and [M0 validation](m0-validation.md): build/check instructions and milestone evidence.

- [M1 validation](log/m1-validation.md): implemented core, coverage, determinism, replay parity, and exit-gate evidence.

- [M2 protocol and validation](log/m2-validation.md): baseline bots, seat-paired evaluation, worker scaling, memory stress, and CLI integration evidence.
- [Play the CLI game](../README.md): build, first games, legal-action prompts, and replays.

- [M3 offline PvE validation](log/m3-validation.md): native adapter, real scene/button games, replay parity, and Godot visual QA.

- [Tabletop presentation](client-presentation.md): Godot components, interaction, animation and debugging.
