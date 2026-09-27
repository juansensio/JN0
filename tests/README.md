# tests

`contract_checks.cpp` validates config and public type contracts. `game_checks.cpp` exercises the M1 core without Godot:

- SplitMix64 golden samples and bounded rejection; exact seed-0 deck/starter ordering.
- Play/draw, full boards, all combat comparisons, direct damage, phase/turn handoffs, passes, wins/draws, reset and terminal rejection.
- Error precedence and full-state immutability after rejection; every actor/payload/card combination at reachable states in 16 seeded matches.
- Owned observation/public projection, legal-action ordering, conservation of 24 unique cards and canonical identity/value invariants.
- Actual JSON fixture loading and exact 62-action terminal state; replay round trips, malformed/unsupported input, illegal action index, truncated log, and result mismatch.
- 1,024 complete seeded games (822 wins / 202 draws / 63,801 actions), comparing duplicate executions after every action and reconstructed full state from JSON replay. This is correctness coverage, not M2 bot/throughput benchmarking.

Run `make configure`, `make build`, `make test` from the repository root. Checks remain enabled in Release/NDEBUG. Fixtures are specification/regression inputs, not alternate rules implementations. See [M1 evidence](../docs/log/m1-validation.md).

M2 `simulator_checks.cpp` verifies bot RNG reproducibility and legality, heuristic defense priorities, exact JSON replay reconstruction of 512 bot matches, independent duplicate action traces, 2,000-game one/four-worker parity, fixed 20,000-game paired evaluation, evaluation worker parity, and invalid batch configurations. `cli_play.cmake` completes four human-input games (both seats and bots), retries invalid choices, verifies their saved replays, and checks quit. CTest also checks bot-match replay and invalid workers/odd evaluation counts. No tests contain alternate rules transitions. See [M2 evidence](../docs/log/m2-validation.md).
