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

Use a fresh directory for clean-build evidence. CTest must report `core_contracts` and `core_gameplay` passing. Explicit check failures remain enabled under Release/NDEBUG. The headless executables link `janus_core`: contract checks validate config/type contracts, and gameplay checks exercise real setup, all action transitions, illegal-action rejection, observation projection, exact replay-fixture parity, and 1,024 seeded complete games.

`.github/workflows/core.yml` runs the same sequence on push, pull request, and manual dispatch. Hosted execution requires the changes on GitHub; a workflow file and local success alone do not pass CI. M1 gameplay, illegal-action, determinism, and complete-match checks run in the same CI sequence. Build output is ignored.

For additional local memory/undefined-behavior checks (Clang/GCC environments):

```sh
cmake -S . -B build/m1-sanitized -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON '-DCMAKE_CXX_FLAGS=-Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined -fno-omit-frame-pointer' '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined'
cmake --build build/m1-sanitized --parallel
ctest --test-dir build/m1-sanitized --output-on-failure
```

Full-state `Game::snapshot()` is the trusted M1 debugging representation. Replay JSON uses `parse_replay`, `encode_replay`, and `execute_replay`; see [replay.md](replay.md). No state restore API is exposed.
