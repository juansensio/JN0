# Godot binding

Optional macOS Godot 4.7 GDExtension. `JanusGame` owns the existing pure core and
reuses `janus_bots` for offline opponents. It exposes owned observations, legal
actions, validated submissions, bot steps, seeded resets, and core-format replay
import/export. All rules remain in `janus_core`.

See [interface and ownership](../../docs/godot-integration.md),
[build/run/test commands](../../docs/development.md#godot-offline-pve-m3), and
[M3 validation](../../docs/log/m3-validation.md).
