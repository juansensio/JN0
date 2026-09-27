# M2 protocol and validation

Date: 2026-09-27. Revision: working tree. Local platform: macOS, Apple Clang 21, CMake 4.0.3, 12 reported hardware threads. Original Word sources are unchanged. Rules/config/replay version remain 1. M2 introduces no rule changes.

## Fixed baseline evaluation protocol

Policies and evaluation were specified before measuring bot strength. HeuristicBot is a visible-board policy, with no search, training, or hidden-state access. No parameter tuning or repeated selection against the evaluation seeds occurred.

- Evaluate HeuristicBot versus RandomBot on **10,000 seed pairs**, seeds 0–9,999.
- Each pair uses the same core seed twice, putting HeuristicBot in each seat once. The starter comes from the core seed; therefore each policy gets both starter roles within each pair.
- RandomBot has reproducible independent policy RNG state derived from game seed and physical seat, using fixed domain-separated mixing. Core RNG is unchanged.
- Count wins, losses, and draws by **policy identity**, regardless of seat. Game score is 1 for win, 0.5 for draw, 0 for loss. Pair score is the average of its two games.
- Treat pairs, not individual games, as sampling units. For N pair scores in [0,1], report the one-sided 95% Hoeffding lower bound `mean_score - sqrt(log(20)/(2*N))`. Pass only when this exceeds 0.5. The criterion is fixed; do not stop early when significance is reached.
- Statistical interpretation assumes pseudorandom seed-pair trials behave independently; the deterministic fixed seed range is a reproducible empirical baseline, not a proof about all seeds/opponents. Paired games need not be independent of each other. Draws contribute half a point rather than being discarded.
- The CLI accepts alternate even game counts and seed ranges for experiments. The milestone gate uses the fixed defaults above. Evaluation failure exits 2; invalid inputs/errors exit 1.

Command:

```sh
./build/janus_cli evaluate --workers 4
```

Result: **18,086 heuristic wins, 1,910 random wins, 4 draws** in 20,000 games, 1,048,112 accepted actions. Heuristic win rate 90.43%; score **90.44%**; one-sided 95% lower score bound **89.2161%**, above 50%: PASS. Scheduling-independent checksum `7568651671136312358`. The automated checks reproduce this evaluation on one and four workers and compare results and action digests.

## Correctness and terminal interaction

Required `make configure`, `make build`, `make test`: PASS; Release CTest **8/8**.

Fresh Debug configuration with `-Wall -Wextra -Wpedantic -Werror`, AddressSanitizer and UndefinedBehaviorSanitizer: build PASS, CTest **8/8**, no diagnostics. See [development instructions](../development.md) for configuration.

Checks cover seeded random choice and legal selection, empty action-list rejection, heuristic winning/tied/losing defense preferences, 512 complete bot matches (256 seeds × two policy combinations) with duplicate traces and exact JSON replay/final-state parity, 2,000 RandomBot games with one/four-worker aggregate parity, paired evaluation and worker parity, zero games/workers, odd pair counts, and seed-range overflow. Existing M0/M1 checks continue to pass.

CLI integration completes **four human-input games**, both seats versus both baseline bots on seed 42, using the first listed legal action at each prompt. It first enters invalid text/zero/out-of-range choices and verifies retry, then verifies all four completed replay files through the strict core replay executor. Quit is checked separately. Bot match saving/verification and malformed worker/evaluation commands are also covered. This exercises terminal interaction automatically; it does not claim an independent human usability study.

## Throughput and worker scaling

Release benchmark command, with other checks finished before the final timed run:

```sh
./build/janus_cli benchmark --games 1000000
```

Each worker count runs the **same one million RandomBot-vs-RandomBot games**, seeds 0–999,999. Every run yields 405,229 player-0 wins, 405,573 player-1 wins, 189,198 draws and 62,300,744 actions, checksum `1400597037708632625`. Benchmark checks worker parity internally. No games fail or crash. Timings are local observations, not performance targets; hardware thread count is the practical tested limit. The table below records the final local run.

| Workers | Seconds | Games/s | Speedup |
| ---: | ---: | ---: | ---: |
| 1 | 20.6874 | 48,338.6 | 1.000× |
| 2 | 21.5677 | 46,365.7 | 0.959× |
| 4 | 8.32184 | 120,166 | 2.486× |
| 8 | 6.28075 | 159,217 | 3.294× |
| 12 | 6.73963 | 148,376 | 3.070× |

Eight workers performed best in this run; twelve regressed slightly, and two were slower than one. These are observations for this machine, not a monotonic scaling guarantee. Threads live in the simulator; bots and core do not use clocks. Timing uses `steady_clock` only around batches. The executable links only libc++ and libSystem according to `otool -L build/janus_cli`; there is no Godot/window/network linkage.

## Stress and memory

A one-worker Release run of 10,000 games peaks at **2,129,920 bytes RSS**; a separate one-million-game run peaks at **2,146,304 bytes RSS** (difference 16,384 bytes). Both exit 0; per-child peak RSS comes from Python `os.wait4`, in bytes on macOS. Memory stays approximately flat despite 100× more games. `/usr/bin/time -l` could not collect its complete metrics in the sandbox (`sysctl kern.clockrate` denied), so these RSS values use the successful per-child measurement instead.

Additional Debug ASan/UBSan stress: **100,000 games / 6,232,914 actions on four workers**, no diagnostics or crashes, checksum `17783092386721152555`. Both workers and match replay objects have bounded lifetimes; batches retain counters rather than all game states. This and the flat Release RSS show no evident accumulating leak. Enabling `ASAN_OPTIONS=detect_leaks=1` explicitly reports “detect_leaks is not supported on this platform” on this runtime, so no exhaustive leak-detector result is claimed. Allocator caches/sanitizer overhead are not benchmarked as leaks.

## Gate audit and remaining debt

All five M2 exit gates have local evidence: headless/no Godot linkage; thousands of consecutive games without errors or evident memory accumulation; measured games/s and scaling through hardware limit; automatic RandomBot-vs-RandomBot completion; statistically defined HeuristicBot superiority.

Hosted Ubuntu CI has not been run on these changes; its existing CMake/CTest sequence includes all eight checks. Windows/Linux execution, Godot integration, and Python parity are later work. No graphical client, backend, payments, training, or new game rules were added. The root [README](../../README.md) explains terminal games and initial manual testing.

Next action: M3 Godot offline PvE using the same core and observation/legal-action contracts. M7 is now dependency-eligible, but training is outside M2.
