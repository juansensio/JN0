# Pre-M3 Godot smoke validation — 2026-09-27

Scope: the user's pasted macOS loading/setup test only. M3 offline PvE remains
NOT STARTED; none of its complete-match gates are claimed.

Environment: macOS, Apple Clang 21, installed Godot
`4.7.2.stable.official.ed1daf0bf`; existing godot-cpp submodule revision
`507ed9d840c01a3c5b2a39af8bb4000bfac30bf5`, API 4.7.

- `make configure`, `make build`, `make test`: PASS, CTest 8/8.
- Normal build cache: `JANUS_BUILD_GODOT=OFF`.
- Separate Debug configure with `JANUS_BUILD_GODOT=ON`: PASS.
- Build target `janus_godot`: PASS; produces `client/bin/libjanus_godot.dylib`.
- First `godot --headless --editor --path client --import`: cache discovery
  completed, then signal-11 crash on exit (exit 134). Cause unresolved.
- Subsequent identical editor import: PASS, exit 0.
- Two `godot --headless --path client --quit` runs after import: PASS, exit 0,
  identical diagnostics:

```text
[Godot] Starting dummy scene
[JN0] JanusGame::_ready()
[JN0] Core game instantiated. seed=42
[JN0] active_player=1
[JN0] P0 lives=3 hand=4 deck=8 board=0
[JN0] P1 lives=3 hand=4 deck=8 board=0
[JN0] legal actions for active player=4
[Godot] Native game node attached
```

The initial unimported run could not resolve `JanusGame`. In the filesystem sandbox
it also could not create Godot's standard application data/log directory. Import
and verification runs were then executed with approved normal filesystem access.
The editor import discovers `res://bin/janus.gdextension` before the scene loads.

No core rules changed. GDScript only creates/attaches the Node. Full state remains
inside the trusted C++ diagnostic; no private card data is exposed to GDScript.
No interactive editor/visual check, actions, bots, complete matches, other platforms,
or replay parity have been validated by this smoke test. Original Word sources are
unchanged. See [instructions](../development.md#pre-m3-godot-smoke-test).
