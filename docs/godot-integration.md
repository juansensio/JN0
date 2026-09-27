# C++ ↔ Godot integration

M3 implements offline PvE in Godot 4.7 through the optional macOS GDExtension.
`client/main.gd` orchestrates modular tabletop components and submits choices; `JanusGame` translates values
and owns one `janus::Game`. The scene tree owns the native Node. The adapter links
`janus_bots`, which links the same pure core used by the CLI and simulator.

## Bound interface

| Method | Contract |
| --- | --- |
| `start(seed: String)` | Reset to default config; decimal unsigned 64-bit seed, including `18446744073709551615`. Invalid input leaves the match unchanged. Resets replay and RandomBot stream. |
| `observe(viewer: int)` | Owned player-visible Dictionary for seat 0/1; invalid viewers return failure. |
| `legal_actions(actor: int)` | Ordered Array of `{actor, kind, card}` from core; invalid/nonacting/terminal actors get an empty array. |
| `submit(actor, kind, card)` | Convert `play`, `attack`, `defend`, or `pass` and call core `step`. Non-pass IDs must fit uint16; legality remains in core. Pass uses card -1 in output. |
| `bot_step(actor, kind)` | `random` or `heuristic`; choose using that seat's observation and core legal actions, then use the same accepted-action path. |
| `export_replay()` | Core-encoded JSON of config, full-range seed, accepted actions, and expected result when terminal. Partial exports are supported. |
| `load_replay(json)` | Parse/execute via core codec, validate exact candidate full state, then replace the match atomically. Accept valid partial or terminal logs; reject malformed/illegal/result-mismatched input unchanged. |

Mutation calls return `{ok: bool, error: String}`. Accepted submissions/bot steps
also return `action`. Core action rejection adds numeric `code` matching
`janus::ActionError` declaration order. Invalid conversion does not reach core.
No full-state getter is bound; snapshots are confined to native replay validation.

`observe` has `ok`, `error`, `viewer`, `config`, `own_hand`, `players`,
`active_player`, `acting_player`, `phase`, `pending_attack`, `consecutive_passes`,
`action_count`, and `result`. Config exposes the core's fields including ordered
`deck_values`. Each public player has lives, deck/hand counts, ordered board and
discard. Cards are `{id, value}`. It never returns seed, deck order, or the other
hand. Returned values cannot alias core storage. Actor/winner absence is -1;
missing pending attack is an empty Dictionary; present pending attack contains
`attacker` and `card`. Phase strings are `main`, `awaiting_defense`, `terminal`;
result outcome/reason strings match the core replay format (`ongoing`/`none` for
unfinished games).

The replay export is a trusted offline debugging artifact, separate from player
observation. Its seed reconstructs private state, as required by the replay
specification; it is not a future ranked observation payload. Loading a replay
opens a static verified view. Restart begins a new match. The adapter resets its
RandomBot stream on import; recorded actions replay exactly, but continuing a
partial replay through native bot calls does not restore historical policy RNG.
The client does not offer continuation from imported replays.

## Presentation and ownership

See [tabletop presentation](client-presentation.md) for component ownership,
transition sequencing and cancellation. GDScript redraws only from `observe(human)` and `legal_actions(human)`. Every
click is checked again by core. During defense it uses `acting_player`, rather
than main-turn owner. Opponent actions run one at a time with a presentation
pause; elapsed time never enters simulation. Human/bot seats can be switched at
restart. There is no network/backend call, second game state, combat calculation,
card-value derivation, or rule-based action generation in GDScript.

`register_types.cpp` registers `JanusGame` at scene initialization, and
`_bind_methods()` exposes its public methods. The extension descriptor loads
`client/bin/libjanus_godot.dylib` through `janus_library_init`. C++ edits require
rebuilding and restarting Godot; native hot reload is not configured. Additional
platform descriptors/builds belong to M8.

See [build/run/test instructions](development.md#godot-offline-pve-m3) and
[M3 evidence](log/m3-validation.md). A feature should use an existing core operation,
add a thin bound conversion, refresh observations/legal choices, and verify the
actual Godot path. Rule changes belong in the pure core first.
