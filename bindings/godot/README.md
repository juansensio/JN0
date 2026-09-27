# bindings/godot

Optional macOS GDExtension smoke-test adapter. `JanusGame` is a native Node
owning the existing `janus::Game`; `_ready()` resets seed 42 and prints setup
counts and the number of core-provided legal actions. It adds no game rules.

The full snapshot is used only inside this trusted diagnostic; no private card
data is returned to GDScript. Future player-facing presentation must use
`Observation`. This is a pre-M3 loading check, not offline PvE or replay parity.
See [build/run instructions](../../docs/development.md#pre-m3-godot-smoke-test).
See [the integration guide](../../docs/godot-integration.md) for loading,
ownership, method binding, and future observation/action interfaces.
