# Build and checks

Run from repository root. Requires CMake >= 3.20 and a C++20 compiler (Apple Clang 21 verified locally). No Godot, backend, credentials, or extra libraries.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Use a fresh directory for clean-build evidence. CTest must report core_contracts passing. Explicit check failures remain enabled under Release/NDEBUG. The headless executable includes the Game API and links janus_core, testing real config validation, payload identity, and owned observation data. It does not call or claim to test unimplemented Game methods.

`.github/workflows/core.yml` runs the same sequence on push, pull request, and manual dispatch. Hosted execution requires the changes on GitHub; a workflow file and local success alone do not pass CI. M1 adds gameplay, illegal-action, determinism, and complete-match checks. Build output is ignored.
