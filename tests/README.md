# tests

`contract_checks.cpp` validates config and public type contracts. `game_checks.cpp` exercises the M1 core without Godot:

- SplitMix64 golden samples and bounded rejection; exact seed-0 deck/starter ordering.
- Play/draw, full boards, all combat comparisons, direct damage, phase/turn handoffs, passes, wins/draws, reset and terminal rejection.
- Error precedence and full-state immutability after rejection; every actor/payload/card combination at reachable states in 16 seeded matches.
- Owned observation/public projection, legal-action ordering, conservation of 24 unique cards and canonical identity/value invariants.
- Actual JSON fixture loading and exact 62-action terminal state; replay round trips, malformed/unsupported input, illegal action index, truncated log, and result mismatch.
- 1,024 complete seeded games (822 wins / 202 draws / 63,801 actions), comparing duplicate executions after every action and reconstructed full state from JSON replay. This is correctness coverage, not M2 bot/throughput benchmarking.

Run `make configure`, `make build`, `make test` from the repository root. Checks remain enabled in Release/NDEBUG. Fixtures are specification/regression inputs, not alternate rules implementations. See [M1 evidence](../docs/log/m1-validation.md).
