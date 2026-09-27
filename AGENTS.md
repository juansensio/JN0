# Coding guidance

- Read `docs/README.md`, `docs/architecture.md`, `docs/game-rules.md`, and the active milestone in `docs/roadmap.md` before coding. The original MVP Roadmap is the operational source of truth; the vision supplies long-term direction.
- Work on the active milestone and its gates. The baseline is M0, not started. Resolve open rules and define interfaces before implementing gameplay. Do not mark a milestone done without evidence.
- Keep all rules in the pure C++20/23 core. No Godot, rendering, audio, network, OS, or platform dependencies in core logic. Use Godot 4 for presentation, Python/PyTorch for AI, and TypeScript/Node for backend services.
- Reuse the same core across offline play, authoritative servers, bots, simulator, and bindings. Use explicit RNG seeds and reproducible config + seed + action replays. Simulation must not depend on FPS or wall time.
- Separate full `GameState` from player-visible `Observation`. The core validates actions; ranked trusts the server. Offline must work without a backend. Keep platform integrations behind adapters.
- Keep changes small and within MVP scope. Use the planned `client/` directory. Do not duplicate rules, add unrequested product depth, or introduce real payments/public distribution work during MVP validation.
- Add relevant rule, illegal-action, determinism, and integration tests as their milestones require. Run the available checks appropriate to each change and report results or environment limitations honestly. No build/test commands exist yet; document them when added.
- Update the relevant docs when rules, interfaces, architecture, or milestone state change. Record completed work, evidence, gates, debt, metrics when applicable, and the next action in `docs/roadmap.md`. Preserve the original Word documents and make source conflicts explicit.
