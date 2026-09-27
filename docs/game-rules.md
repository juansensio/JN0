# MVP game rules

These rules summarize the MVP Roadmap. Unresolved details below must be decided during M0; this document does not supply invented defaults.

## Setup

- Two players, each starting with three lives.
- Each card has one numeric value used as both attack and defense.
- The proposed initial deck is identical for both players: 12 cards, with three copies each of values 1, 2, 3, and 4. The roadmap labels this deck as proposed; confirm it in M0.
- Each player draws four cards initially.
- Each player's board holds at most three cards.
- All randomness, including shuffling, uses an explicit seed.

## Turn and actions

Each turn has exactly one main action: play a card or attack with an own card already on the board.

**Play:** move a card from hand to board, then immediately draw one replacement if the deck still contains cards. The board limit must be respected.

**Attack:** choose an own board card. If the defender has board cards, the defender must choose one to defend. This response is part of resolving the attack; its state/action representation must be formalized in M0.

## Combat and results

Compare the attacking and defending values. Destroy the lower-value card; on a tie, destroy both. A blocked attack never reduces lives, including when the attacker wins the comparison.

If the opponent has no card available to defend, they lose one life. Reaching zero lives loses the match. Emptying the deck does not cause defeat; it only stops further draws.

## Decisions required before M0 closes

The sources leave these implementation details open:

- First-player selection and the exact setup/shuffle ordering.
- Turn handoff and explicit phases for pending attack, defender selection, combat resolution, and terminal state.
- Behavior when the current player has no legal main action; whether passing, a draw, or another rule is needed to guarantee completion.
- Whether a surviving attacker has any exhaustion/reuse restriction on later turns; none is specified in the roadmap.
- Exact contents of player observations, including visibility of hands, deck order/counts, and pending combat.
- Card identity and ordering, action payloads, invalid-action behavior, RNG specification, and replay/config versioning.

Resolve gameplay questions in the specification before implementing assumptions. The latter contract details also need an explicit M0 design so replay and bindings behave consistently.

## Rule verification checklist

Turn these into relevant tests during M1 after M0 resolves the open cases:

- Initial lives, hand size, proposed deck composition, and maximum board size.
- Legal play transfers one card and draws exactly one when possible; an empty deck prevents a draw without causing defeat.
- Attacks require an own board card; a defender with board cards must select a valid one.
- Lower-value attacker destroyed, lower-value defender destroyed, and both destroyed on a tie.
- No life loss from any blocked attack; exactly one life lost on an unblocked attack.
- Loss at zero lives, terminal-state behavior, and rejection of illegal actions.
- Every agreed phase, turn transition, hidden-information boundary, and no-legal-action case.
- Complete games execute without Godot; repeated config + seed + actions reproduce the result.
