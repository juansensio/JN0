# Headless simulator

`janus_simulator` links the same core and baseline bots; `janus_cli` provides play, match, batch, evaluate, benchmark, and replay commands. See the root [README](../README.md) for running games.

`run_match` returns a complete Replay and final GameState for trusted replay/debug consumers. Bots receive only observations and legal actions. `run_batch` uses bounded worker threads and assigns seed `first_seed + index` to each game. For paired evaluation, games `2*i` and `2*i+1` share seed `first_seed + i`, swap bot seats, and report wins by policy identity. Random policy streams are derived from the game seed and physical seat using fixed domain constants; they never share mutable RNG state with core or workers.

Every worker owns its game, policies, and local counters. Workers claim indices atomically and join before reduction. Aggregate counts and the XOR of indexed action-trace digests do not depend on scheduling; seconds use a monotonic clock only for measurement. No wall time changes simulation. Memory scales with workers, not total games; only one match replay per active worker is retained. Errors propagate after joining all workers. Timing and checksum are diagnostics, not authoritative replay hashes.

Benchmark runs the same seed range at 1, 2, 4, … workers and the requested/hardware limit. It rejects worker-dependent checksums and reports scaling relative to one worker. No Godot, window, rendering, network, or third-party runtime dependency is linked. Statistical protocol and measured evidence are in [M2 validation](../docs/log/m2-validation.md).
