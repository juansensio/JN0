# Offline tabletop presentation

M3 presentation follow-up; gameplay and the native adapter contracts remain rules v1.
The 1280 × 900 reference canvas scales with the window, retaining its aspect ratio.
The table clips the opponent's upper card fan below the toolbar. Both seats use the
same player-relative layout: own deck bottom right/discard bottom left, opposing
piles mirrored. Counts come from the observation, including empty piles.

## Components and debugging

| File under `client/` | Responsibility |
| --- | --- |
| `main.gd` | Match lifecycle, observation/legal-action calls, bot pacing, action confirmation, replay I/O |
| `components/table_view.gd` / `.tscn` | Table composition/layout and observation presentation |
| `components/card_view.gd` / `.tscn` | Procedural face/back drawing, hover/selection, legal and pending-attack highlights |
| `components/hand_view.gd` / `.tscn` | Fan layout; opponent uses anonymous backs only |
| `components/board_view.gd` / `.tscn` | Three visual slots and card-selection signal |
| `components/pile_view.gd` / `.tscn` | Deck/discard count, latest public discard, empty slot |
| `components/player_hud.gd` / `.tscn` | Player/hand count, lives and actor marker |
| `components/match_settings.gd` / `.tscn` | Seed, bot, seat, restart and replay controls |
| `components/animation_director.gd` | Movement, impact, discard/draw transitions and cancellation |
| `components/impact_effect.gd` | Procedural combat burst |

Views emit `card_selected(id)`; the controller reveals the corresponding legal
confirmation control. All submitted actions still pass through `JanusGame.submit`.
The view never constructs legal choices or compares card values to resolve combat.
Card IDs remain script properties for debugging; opponents' hand views have ID -1
and value 0. No opponent private hand is requested or represented.

Use Godot's Remote scene inspector during play to inspect each component. Card
size/accent and animation travel/impact durations are exported settings. Adjust the
layout in `TableView.arrange`, fan spacing in `HandView.arrange`, and drawing in
`CardView._draw`. These are procedural prototype visuals, with no imported art.

## Transition lifecycle

1. Capture the player observation and submit a human/bot action to the shared core.
2. On acceptance, capture the next observation and lock gameplay input/bot steps.
3. Animate temporary cards using the accepted action and public zone changes:
   hand → board, deck → hand, attack/defense clash, life feedback, board → discard.
4. Clean up temporary effects, redraw the authoritative observation and unlock.

Destroyed cards are identified from the resulting public discard; Godot does not
calculate combat. Surviving cards return to their board slots. The pending attacker
is highlighted while defense is awaited. Pass gets brief status feedback.

Restart and successful replay load cancel tweens/effects and invalidate pending
continuations with a session token. Failed restart/load leaves the existing match
intact. Saving records the accepted core state, including during presentation.
Elapsed time is used only for visual pacing; replays contain core actions alone.

`animations_enabled = false` is a controller test switch for bulk match checks;
normal play enables animations. `tests/presentation_checks.gd` separately drives
full animated matches and interrupt scenarios. See [validation](log/m3-tabletop-validation.md).
