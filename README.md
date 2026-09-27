# Janus Noise

A minimal two-player card game with one deterministic C++20 rules core. M2 adds a terminal game, baseline bots, batch simulation, evaluation, and benchmarks. M3 adds an optional Godot offline PvE client. The terminal game works without Godot; no backend is required.

## Build and check

Install CMake 3.20 or newer, a C++20 compiler, and Make. From this repository:

```sh
make configure
make build
make test
```

The executable is `build/janus_cli` (on Windows, use `janus_cli.exe`; a multi-configuration generator may place it in `build/Release/`). Direct CMake commands are in [development instructions](docs/development.md).

## Play with the Godot client

Build/import the optional macOS extension, then run `godot --path client`. Select either baseline bot and seat, play through core-provided action buttons, restart, and save/load replays. See [setup and controls](docs/development.md#godot-offline-pve-m3).

## Play your first terminal games

Start with RandomBot, then try HeuristicBot and the other seat:

```sh
./build/janus_cli play --seed 42 --bot random --save build/my-first-game.json
./build/janus_cli play --seed 43 --bot heuristic --save build/my-second-game.json
./build/janus_cli play --seed 44 --bot heuristic --human 1
```

You are player 0 by default. The seed fixes the deal and starting player; change it for another setup. Cards appear as `#9[4]`: card ID 9, strength 4. Each prompt lists legal actions. Type the **action number**, then Enter; do not type the card ID. Type `q` to quit. Invalid input asks again.

Both players start with three lives and four cards in hand. On your turn, play a card (up to three on your board) or attack with a board card. Playing draws a replacement while cards remain in your deck. If attacked while you have a board, choose a defender when prompted. The lower strength card is destroyed; ties destroy both. Blocked attacks cost no lives. An attack against an empty board removes one life. Zero lives loses; two forced passes draw. See [complete rules](docs/game-rules.md).

A completed game saves its configuration, seed, actions, and expected result when `--save` is supplied. Quitting early does not save a complete replay. Verify a saved game:

```sh
./build/janus_cli replay --file build/my-first-game.json
```

For initial manual testing, try both bots and both seats, attack an empty board, select a defender, fill your board, and enter an invalid action number. Note the seed, steps, expected behavior, and observed behavior when reporting a problem; include a saved replay if the game completed. Reusing the seed reproduces the initial setup; replaying reproduces all actions exactly.

## Automated games and benchmarks

Run a single bot game and inspect its replay:

```sh
./build/janus_cli match --seed 42 --first heuristic --second random --save build/bot-game.json
./build/janus_cli replay --file build/bot-game.json
```

Run batches, evaluate bot strength, and benchmark workers:

```sh
./build/janus_cli batch --games 10000 --seed 0 --workers 1
./build/janus_cli batch --games 10000 --seed 0 --workers 2 --first heuristic --second random
./build/janus_cli evaluate --workers 2
./build/janus_cli benchmark --games 100000 --workers 2
./build/janus_cli --help
```

Worker counts must be between one and the machine's reported hardware thread count. Benchmark without `--workers` to test through that limit, using 1, 2, 4, … and the exact limit. It reports games/s, elapsed time, and speedup; timing varies with machine load. Matching checksums verify the same indexed action traces and results across workers. Batch `first_wins` means the first configured bot; evaluation preserves that meaning when seats swap. Draws count as half a point in `score`.

Evaluation defaults to 20,000 games: HeuristicBot versus RandomBot on seeds 0–9,999, twice per seed with seats swapped. It prints a one-sided 95% lower score bound and passes only above 50%. Exit codes: 0 success, 1 invalid input/execution error, 2 evaluation failed. See [M2 protocol and evidence](docs/log/m2-validation.md) for assumptions, measurements, and limitations.

## Project references

- [Documentation index](docs/README.md)
- [Architecture](docs/architecture.md)
- [Milestones and progress](docs/roadmap.md)
- [Core contract](docs/core-contract.md)
- [Replay format](docs/replay.md)
