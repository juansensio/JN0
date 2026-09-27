# Build and checks

Run from repository root. Requires CMake >= 3.20 and a C++20 compiler (Apple Clang 21 verified locally). No Godot, backend, credentials, or extra libraries.

The Makefile wraps the same commands; run them in order:

```sh
make configure
make build
make test
```

Equivalent direct commands:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Use a fresh directory for clean-build evidence. CTest must report all eight checks passing: core contracts/gameplay, bots/simulator, CLI match/replay, human play, and invalid worker/evaluation input. Explicit check failures remain enabled under Release/NDEBUG. The headless executables link `janus_core`: contract checks validate config/type contracts, and gameplay checks exercise real setup, all action transitions, illegal-action rejection, observation projection, exact replay-fixture parity, and 1,024 seeded complete games.

`.github/workflows/core.yml` runs the same sequence on push, pull request, and manual dispatch. Hosted execution requires the changes on GitHub; a workflow file and local success alone do not pass CI. M1 gameplay and M2 bot, batch determinism, paired superiority, and CLI integration checks run in the same CI sequence. Build output is ignored.

For additional local memory/undefined-behavior checks (Clang/GCC environments):

```sh
cmake -S . -B build/m1-sanitized -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON '-DCMAKE_CXX_FLAGS=-Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer' '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'
cmake --build build/m1-sanitized --parallel
ctest --test-dir build/m1-sanitized --output-on-failure
```

Full-state `Game::snapshot()` is the trusted M1 debugging representation. Replay JSON uses `parse_replay`, `encode_replay`, and `execute_replay`; see [replay.md](replay.md). No state restore API is exposed.

## M2 simulator and terminal game

Build produces `build/janus_cli`. See the root [README](../README.md) for human games, replay saving/verification, bot batches, and worker benchmarks. The simulator adds only the C++ standard threading library; no Godot or external dependency.

The sanitizer commands above also cover all M2 targets; use `build/m2-sanitized` as a fresh directory. Additional stress check:

```sh
./build/m2-sanitized/janus_cli batch --games 100000 --workers 4
```

Choose a worker count supported by your hardware. Sanitizers run much slower than Release; benchmark Release separately without other checks running. Apple Clang/macOS AddressSanitizer does not provide LeakSanitizer; report that limitation alongside peak-memory stress observations. See [M2 evidence](log/m2-validation.md).

## Godot offline PvE (M3)

Requires Godot 4.7, Python 3 for binding generation, and the checked-out
`third_party/godot-cpp` submodule. Initialize it on a fresh clone:

```sh
git submodule update --init --recursive
```

The optional `JANUS_BUILD_GODOT` defaults OFF, preserving the headless workflow.
The current extension descriptor supports macOS. Build all targets in a separate
directory, import the project, and run:

```sh
cmake -S . -B build/godot -DCMAKE_BUILD_TYPE=Debug -DJANUS_BUILD_GODOT=ON
cmake --build build/godot --parallel 8
godot --headless --editor --path client --import
godot --path client
```

Choose RandomBot or HeuristicBot, a seat, and a decimal seed; press New / Restart.
Open Match settings to change seed/bot/seat. Hover and select a hand or board
card, then confirm Play, Attack, or Defend below the hand. Only core legal actions
are available. A forced Pass appears when appropriate. The tabletop canvas scales
with the window; results and restart remain in the same screen. See
[presentation/debugging](client-presentation.md).

Save replay writes `user://last-match.json` (the full path is displayed), replacing
the previous save. Load replay opens any valid core-format JSON and presents the
verified state; restart to play again. Partial saves can be viewed, while CLI
`replay` requires a complete match. See [client instructions](../client/README.md).

Run the optional integration suite:

```sh
ctest --test-dir build/godot --output-on-failure
```

Nine checks include `godot_offline`: import, native boundary/atomic rejection,
exact fixture parity, 32 complete games through real scene selection/confirmation,
replay view and restart, both bots/seats, and headless verification of every saved
client match. Four additional animated games exercise all combat outcomes, card
hover/click/confirmation, input locking, visible-zone counts, anonymous backs, and
restart/import during animation. Tests write artifacts/logs under `build/godot`. Godot needs normal
macOS access to system certificates and its editor settings directory; a restricted
sandbox can emit platform errors even when game assertions pass. Do not suppress
those errors as evidence of a clean Godot run.

Optional visual QA renders the actual game viewport at turn, selection, defense, combat, result and 960 × 675:

```sh
godot --path client --script res://tests/render_check.gd -- "$PWD/build/godot/m3-client"
```

Use an absolute output prefix if launching outside the repository root. Generated
libraries, images/replays in build, and `.godot/` cache are ignored. Rebuild after
native changes and restart the game; hot reload is not configured. See
[the integration guide](godot-integration.md) and [M3 evidence](log/m3-validation.md).
The historical [pre-M3 smoke log](log/pre-m3-godot-smoke.md) remains unchanged.
