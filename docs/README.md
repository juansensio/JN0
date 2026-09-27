# Janus Noise development docs

Janus Noise is a shared platform for small competitive games. The first MVP validates the technical stack with a deliberately minimal card game.

Read these references before implementing a feature:

- [Vision and scope](vision.md): product direction, MVP boundaries, and later ambitions.
- [Architecture](architecture.md): responsibilities, dependencies, core interfaces, and repository layout.
- [MVP game rules](game-rules.md): resolved M0 specification and required M1 rule tests.
- [MVP roadmap](roadmap.md): dependencies, work, exit gates, metrics, and progress tracking.
- [M0 implementation tasks](m0-tasks.md): ordered tasks, completion checklists, dependencies, and current gate status.
- [Core contract](core-contract.md): concrete C++ type/lifecycle semantics and validation rules.
- [Replay specification](replay.md) and [worked match](replay-example.md): deterministic RNG, versioned encoding, and complete specification fixture.
- [Development commands](development.md) and [M0 validation](m0-validation.md): build/check instructions and milestone evidence.

- [M1 validation](log/m1-validation.md): implemented core, coverage, determinism, replay parity, and exit-gate evidence.
