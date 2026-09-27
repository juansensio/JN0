# MVP game rules

These rules summarize the MVP Roadmap. M0 decisions below explicitly resolve its omissions; they are additions to the specification, not claims about the original source. M1 implements and tests rules version 1.

## Setup

- Two players, each starting with three lives.
- Each card has one numeric value used as both attack and defense.
- M0 confirms the proposed identical decks: 12 cards per player, with three copies each of values 1, 2, 3, and 4.
- Each player draws four cards initially.
- Each player's board holds at most three cards.
- All randomness, including shuffling, uses an explicit seed.

## Turn and actions

Each turn has exactly one main action: play a card or attack with an own card already on the board.

**Play:** move a card from hand to board, then immediately draw one replacement if the deck still contains cards. The board limit must be respected.

**Attack:** choose an own board card. If the defender has board cards, the defender must choose one to defend through the explicit pending-attack phase and Defend action defined below.

## Combat and results

Compare the attacking and defending values. Destroy the lower-value card; on a tie, destroy both. A blocked attack never reduces lives, including when the attacker wins the comparison.

If the opponent has no card available to defend, they lose one life. Reaching zero lives loses the match. Emptying the deck does not cause defeat; it only stops further draws.

## M0 decision record — rules version 1

The source omitted these details. M0 adopts the following choices to make the rules implementable while preserving the original combat and life rules:

- D01: Confirm the proposed deck and existing lives/hand/board limits.
- D02: Player IDs are 0 and 1. Card slot i for player p has ID `12*p+i`, with values `[1,1,1,2,2,2,3,3,3,4,4,4]`. Shuffle player 0 then player 1, draw four from each deck's front (player 0 first), then select starter with the next bounded random sample in `[0,2)`. See [replay.md](replay.md).
- D03: Play and direct Attack resolve atomically and hand off the turn. A blocked Attack records the attacking card, keeps the main-turn owner, and enters awaiting_defense; the opponent acts. Defend resolves all combat atomically, clears pending combat, and gives the defender the next main turn.
- D04: A surviving attacker can attack on any later own turn; there is no exhaustion, as none is specified by the source.
- D05: Pass is legal only if neither Play nor Attack is legal. Two consecutive main-turn passes end in a draw with no winner. Play, Attack, and Defend reset the pass counter. No voluntary pass, timer, repetition rule, resignation, or turn cap is introduced.
- D06: Each viewer sees its own ordered hand, both players' lives, hand/deck counts, ordered boards/discards, config, phase, main-turn owner, acting player, pending attack, pass/action counters, and result. Neither deck order, opponent private hand nor seed is exposed. Observation owns its values and cannot alias state. Known IDs reveal original composition, not current hidden card locations.
- D07: Destroyed cards append to their owner's public discard. Erasures preserve zone order; draws/plays append to hand/board. Reset replaces the entire match; all subsequent transitions require Actions. Terminal state has no acting player, retains the last main-turn owner, and rejects further actions without mutation.

No gameplay questions remain open for rules v1. [core-contract.md](core-contract.md) defines identity constraints, errors, action ordering, and API semantics. [replay.md](replay.md) defines RNG and versioning. Setup has three lives each, eight deck cards after dealing, main phase, no pending attack, zero pass/action counters, and ongoing result.

## Phase transitions

| Phase / actor | Action and validation | Effects | Next phase / owner |
| --- | --- | --- | --- |
| main / active | Play own hand card, board < 3 | Move; draw if possible; clear passes | main / opponent |
| main / active | Attack own board card; enemy board nonempty | Record pending attack; clear passes | awaiting_defense / same owner; opponent acts |
| main / active | Attack own board card; enemy board empty | Remove one enemy life; clear passes | main / opponent, or terminal / same owner at zero lives |
| awaiting_defense / opponent of active | Defend own board card | Compare; discard destroyed cards; clear pending/passes | main / defender |
| main / active | Pass only if no Play or Attack possible | Increment consecutive passes | main / opponent, or terminal / same owner at two passes |
| terminal / nobody | All actions invalid | Reject unchanged | terminal / unchanged |

Each accepted action increments action_count once, including defense/pass. Rejections change nothing. Blocked combat never removes lives. There is no start-of-turn draw. An attacker remains on the board after direct damage.

Termination needs no arbitrary cap: each Play consumes one of 24 initially undeployed cards; each blocked combat destroys at least one of 24 cards; each direct attack consumes one of six initial lives. A pass either precedes a resource-consuming turn or a second pass ends in a draw. Survivor reuse cannot produce an infinite match.

## Worked examples

- Play/draw: hand `[0(value 1),9(value 4)]`, deck front `3(value 2)`, empty board. Play 9 gives board `[9]`, hand `[0,3]`, and one fewer deck card; hand off.
- Attacker 4 vs defender 2: discard defender, retain attacker. Attacker 1 vs defender 3: discard attacker, retain defender. Attacker 2 vs defender 2: discard both. No lives lost; defender owns next turn.
- Direct damage: Attack 9 against an empty board with three lives leaves two lives and retains 9. Against one life it produces the attacker's win, zero enemy lives, and no next actor.
- Full board of three cards: Play is rejected unchanged; each board card can Attack.
- Empty deck: Play still moves the card but draws nothing. Empty deck alone never causes defeat.
- Empty hand and board: only Pass is legal, regardless of deck count. If the opponent also cannot act, their Pass yields a draw.
- Terminal: wins have a winner and reason zero_lives; draws have no winner and reason both_players_stuck. All further actions fail unchanged until reset.

The complete seeded match fixture and its reviewed trace are in [replay-v1.json](../tests/fixtures/replay-v1.json) and [replay-example.md](replay-example.md). M1 executes them against the game core and checks exact final state.

## Rule verification checklist

These are covered by M1 `tests/game_checks.cpp` using the resolved M0 specification:

- Initial lives, hand size, proposed deck composition, and maximum board size.
- Legal play transfers one card and draws exactly one when possible; an empty deck prevents a draw without causing defeat.
- Attacks require an own board card; a defender with board cards must select a valid one.
- Lower-value attacker destroyed, lower-value defender destroyed, and both destroyed on a tie.
- No life loss from any blocked attack; exactly one life lost on an unblocked attack.
- Loss at zero lives, terminal-state behavior, and rejection of illegal actions.
- Every agreed phase, turn transition, hidden-information boundary, and no-legal-action case.
- Complete games execute without Godot; repeated config + seed + actions reproduce the result.
