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

## Pre-M3 Godot smoke test

Requires Godot 4.7, Python 3 for godot-cpp binding generation, and the checked-out
`third_party/godot-cpp` submodule (`git submodule update --init --recursive` on a
fresh clone). The extension is optional: `JANUS_BUILD_GODOT` defaults to `OFF`,
so the normal Makefile workflow still builds only the core/bots/simulator/CLI.
Use a separate build directory:

```sh
cmake -S . -B build/godot -DCMAKE_BUILD_TYPE=Debug -DJANUS_BUILD_GODOT=ON
cmake --build build/godot --target janus_godot --parallel 8
godot --headless --editor --path client --import
godot --headless --path client --quit
```

The import discovers the extension on a fresh project before GDScript resolves
the native class. The macOS library is `client/bin/libjanus_godot.dylib`; generated
libraries and `client/.godot/` are ignored. The descriptor currently targets macOS
only. Successful output reports seed 42, three lives, four hand cards, eight deck
cards, an empty board per player, and four legal actions. Repeated seed-42 runs
must report the same active player. This only checks loading and setup; it does
not demonstrate complete-match replay parity or any M3 exit gate.

To inspect the dummy project interactively:

```sh
godot --editor --path client
```

Press F5 to run; the scene has no visual UI, so inspect the Output panel.
