# Godot offline client

Local human-versus-bot tabletop prototype using the shared C++ core. See
[build/run instructions](../docs/development.md#godot-offline-pve-m3).

1. Launch `godot --path client` after building/importing the extension.
2. Open Match settings to choose a bot, seat, or decimal seed; start a match.
3. Hover your cards to lift them. Click a hand/board card to select it, then confirm
   Play, Attack, or Defend below the hand. Click again to deselect. Legal cards glow.
4. During defense, the incoming attacker is marked. Pass appears only when legal.
5. New / Restart resets using the current settings, including during animation.

Your hand is at the bottom, opposing anonymous card backs at the top. Public cards
are on the table. Your deck is bottom right and discard bottom left; opposing piles
are mirrored. Counts and lives update after each animation. Destroyed cards move
to their owner's discard pile; plays/draws, clashes, and direct damage give feedback.
The reference canvas scales with the window. Offline play needs no backend.

Save replay in Match settings overwrites `user://last-match.json`. Its full local
path appears in the table notice. Load replay selects JSON and shows a static,
core-verified state; restart to play again. Partial and completed saves are supported.
The seed/replay is a trusted offline debug artifact, not a ranked observation.

Small component scenes/scripts live in `components/`; see
[presentation architecture/debugging](../docs/client-presentation.md).
`tests/offline_checks.gd` drives complete games through selection/confirmation.
`tests/presentation_checks.gd` checks animated games and cancellation.
`tests/render_check.gd` renders opening, selection, defense, combat, result and a
smaller window for visual QA. See [tabletop validation](../docs/log/m3-tabletop-validation.md).
