# C++ ↔ Godot integration

The current integration is a macOS/Godot 4.7 smoke test. It loads the shared core
and prints seeded setup data. The feature workflow below guides future M3 work;
observation conversion, action submission, signals, bots, and replay loading are
not implemented yet.

## Layers and ownership

```text
client/                    GDScript: scenes, input, display, animation
    ↓ bound methods        ↑ converted observations/results (future)
bindings/godot/            C++ JanusGame Node: Godot/core adapter
    ↓ core API
core/                      C++ janus::Game: state, legality, rules, RNG
```

`JanusGame` owns one `janus::Game` by value. Creating another Node creates another
independent game. The scene tree owns the Node after `add_child(game)`; destroying
the Node also destroys its core game. Avoid copying game state into a second
authoritative model in GDScript.

The adapter translates data and delegates operations. Core code never includes
Godot headers. GDScript presents core-provided choices and results; combat,
draws, turn changes, legality, and victory stay in `janus::Game`. Animations may
delay input or presentation, but simulation never depends on frame rate or time.

## How the current project loads

| File | Role |
| --- | --- |
| Root `CMakeLists.txt` | Optional `JANUS_BUILD_GODOT` switch, default OFF; makes core position-independent and includes godot-cpp/adapter when enabled |
| `bindings/godot/CMakeLists.txt` | Links `janus_core` and godot-cpp into `client/bin/libjanus_godot.dylib` |
| `client/bin/janus.gdextension` | Tells Godot which library to load and names its `janus_library_init` entry point |
| `bindings/godot/src/register_types.cpp` | Initializes the binding and registers `JanusGame` at scene initialization level |
| `bindings/godot/src/janus_game.hpp` | Declares the native Node, `GDCLASS`, and owned core game |
| `bindings/godot/src/janus_game.cpp` | Exposes `smoke_test` through `_bind_methods()` and delegates setup to the core |
| `client/main.gd` | Creates `JanusGame.new()` and attaches it to the dummy scene |

Godot imports the descriptor and loads the library, then calls the exported entry
point. Registration makes `JanusGame` available to GDScript. When the dummy scene
attaches the Node, its `_ready()` calls `smoke_test()`, which resets seed 42 and
prints counts. Calling `smoke_test()` again resets the match; it is a diagnostic,
not a read-only status query.

The C++ declaration alone does not expose a method to GDScript. For example, the
existing binding is:

```cpp
ClassDB::bind_method(D_METHOD("smoke_test"), &JanusGame::smoke_test);
```

For an additional native class, add its sources to the adapter target and register
it in `register_types.cpp`. Ordinary helper classes that GDScript never creates
do not need Godot registration.

## Adding a gameplay feature

1. Identify the existing core operation in [the contract](core-contract.md).
   If the feature changes rules, implement and test it in core first, following
   [the resolved rules](game-rules.md) and active milestone scope.
2. Add a thin public adapter method and bind it in `_bind_methods()`. Convert
   Godot input to core types, validate conversion, call core, and translate the
   returned error/result. Do not let invalid enum values or narrowing integer
   conversions reach core accidentally.
3. Convert owned core output into Godot-supported values such as `Dictionary`,
   `Array`, integers, strings, and booleans. Define field names, enum mapping,
   optional-value representation, and integer ranges explicitly. Preserve card
   IDs and zone order; never use a visual card position as its identity.
4. Update GDScript to call the adapter and redraw from the returned observation.
   Signals can notify UI after accepted transitions or reset; register them in
   the adapter when introduced. Refresh legal actions after every accepted step
   so a stale UI selection is still checked by the core.
5. Verify core behavior and the actual Godot call path. Record evidence and any
   changed interface in the docs before claiming a milestone gate.

The intended offline flow is `reset(seed) → observe(viewer) + legal_actions(actor)
→ step(action) → refresh observation/actions/result`. These are core API names,
not currently bound GDScript methods. During pending defense, the acting player
differs from the main-turn owner; use the observation's acting-player field.

Return `Observation` to player UI. `snapshot()` contains deck order, both private
hands, and seed; its current use is confined to trusted C++ smoke diagnostics.
Never make it the general UI data source. Keep returned values independent of
mutable core storage, and report rejected actions without changing the displayed
authoritative state.

For bots, reuse `janus_bots` with that bot's observation and legal actions, then
submit its chosen action through the same core step path. For replay loading,
reuse `janus/replay.hpp` and the existing codec/executor; compare the same config,
seed, and accepted actions with headless execution. Do not recreate either system
in GDScript. Seed conversion must preserve the core's unsigned 64-bit range if
the eventual Godot interface supports arbitrary replay seeds.

## Build, verify, and troubleshoot

Follow [development commands](development.md#pre-m3-godot-smoke-test) for dependency
initialization, separate extension build, project import, and headless run.
Rebuild `janus_godot` after C++ changes; GDScript edits do not require a native
rebuild. Restart Godot after rebuilding the library; hot reload is not configured.
Generated libraries and `.godot/` cache are ignored, while the descriptor and
scene sources are tracked. Additional platforms belong to later milestone work.

If `JanusGame` is unknown, check the library exists, import the project, and inspect
extension loading errors before debugging the script. The descriptor's symbol
must match the exported entry point, and its API minimum must be compatible with
the engine. Keep the godot-cpp submodule revision and selected API deliberate when
upgrading; rebuild and rerun checks after a change.

Run `make configure`, `make build`, and `make test` to protect the headless
workflow. For adapter changes, also build/run the extension and check conversion
errors, hidden-information boundaries, illegal-action rejection, and replay parity
as those interfaces arrive. The existing [smoke evidence](log/pre-m3-godot-smoke.md)
proves loading/setup only and records a first-import crash; it does not establish
full offline play or replay parity.
