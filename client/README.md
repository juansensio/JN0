# Godot offline client

M3 provides a local human-versus-bot game using the shared C++ core. See
[build/run instructions](../docs/development.md#godot-offline-pve-m3).

1. Launch `godot --path client` after building/importing the extension.
2. Select HeuristicBot or RandomBot and either seat. Player 1/2 in the UI maps to
   core player 0/1. Enter a decimal seed and press New / Restart.
3. Click Play on a hand card or Attack on a board card. When attacked, click Defend.
   Only legal choices appear. Pass appears only when the core permits it.
4. The result appears at the top. New / Restart uses the current controls.

Cards show numeric value and stable ID; both boards, lives, deck/hand/discard counts,
turn and pending attack are visible. Opponent hand identities are private. Small
windows scroll. Bots run locally; no account, server or internet is required.

Save replay overwrites `user://last-match.json`; its full local path appears below
the hand. Load replay selects JSON and shows the core-verified state. It supports
partial saves and completed matches. Loaded views do not advance automatically;
restart to resume normal play. The terminal CLI can verify completed saved matches.
The seed/replay is a trusted offline debug artifact, not a ranked observation.

`tests/offline_checks.gd` drives the real scene/buttons through both bots and seats.
`tests/render_check.gd` creates viewport artifacts for visual QA. See
[M3 validation](../docs/log/m3-validation.md).
