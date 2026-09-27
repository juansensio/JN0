# Replay v1 worked match review

This trace was calculated in M0 and is now reproduced by the M1 C++ gameplay test, including exact final zones, counters, and result. Seed 0, rules/config 1. RNG/setup ordering gives:

- Player 0 shuffled IDs: `[4, 1, 6, 8, 0, 5, 2, 3, 11, 9, 10, 7]`; initial hand `[4, 1, 6, 8]`.
- Player 1 shuffled IDs: `[18, 23, 19, 12, 17, 20, 16, 15, 14, 13, 21, 22]`; initial hand `[18, 23, 19, 12]`.
- Starter: 0; lives `[3, 3]`, empty boards/discards, eight deck cards each.

The fixture stores all 62 actor-tagged actions. Draws below derive solely from the seeded deck, never extra replay inputs. Every Play uses a hand card on an empty board; every Attack uses an own board card; every Defend uses a board card belonging to the opponent of the main-turn owner; every Pass requires empty hand and board. Defense destroys at least one card without life damage.

| Action # | Actor | Action | Forced effects | Next actor |
| --- | --- | --- | --- | --- |
| 1 | 0 | play 6 (value 3) | draw 0 | 1 |
| 2 | 1 | play 23 (value 4) | draw 17 | 0 |
| 3 | 0 | attack 6 (value 3) | await defense | 1 |
| 4 | 1 | defend 23 (value 4) | 3 vs 4; discard [6]; no life loss | 1 |
| 5 | 1 | attack 23 (value 4) | direct damage; lives [2, 3] | 0 |
| 6 | 0 | play 8 (value 3) | draw 5 | 1 |
| 7 | 1 | attack 23 (value 4) | await defense | 0 |
| 8 | 0 | defend 8 (value 3) | 4 vs 3; discard [8]; no life loss | 0 |
| 9 | 0 | play 4 (value 2) | draw 2 | 1 |
| 10 | 1 | attack 23 (value 4) | await defense | 0 |
| 11 | 0 | defend 4 (value 2) | 4 vs 2; discard [4]; no life loss | 0 |
| 12 | 0 | play 5 (value 2) | draw 3 | 1 |
| 13 | 1 | attack 23 (value 4) | await defense | 0 |
| 14 | 0 | defend 5 (value 2) | 4 vs 2; discard [5]; no life loss | 0 |
| 15 | 0 | play 3 (value 2) | draw 11 | 1 |
| 16 | 1 | attack 23 (value 4) | await defense | 0 |
| 17 | 0 | defend 3 (value 2) | 4 vs 2; discard [3]; no life loss | 0 |
| 18 | 0 | play 11 (value 4) | draw 9 | 1 |
| 19 | 1 | attack 23 (value 4) | await defense | 0 |
| 20 | 0 | defend 11 (value 4) | 4 vs 4; discard [23, 11]; no life loss | 0 |
| 21 | 0 | play 9 (value 4) | draw 10 | 1 |
| 22 | 1 | play 18 (value 3) | draw 20 | 0 |
| 23 | 0 | attack 9 (value 4) | await defense | 1 |
| 24 | 1 | defend 18 (value 3) | 4 vs 3; discard [18]; no life loss | 1 |
| 25 | 1 | play 19 (value 3) | draw 16 | 0 |
| 26 | 0 | attack 9 (value 4) | await defense | 1 |
| 27 | 1 | defend 19 (value 3) | 4 vs 3; discard [19]; no life loss | 1 |
| 28 | 1 | play 20 (value 3) | draw 15 | 0 |
| 29 | 0 | attack 9 (value 4) | await defense | 1 |
| 30 | 1 | defend 20 (value 3) | 4 vs 3; discard [20]; no life loss | 1 |
| 31 | 1 | play 15 (value 2) | draw 14 | 0 |
| 32 | 0 | attack 9 (value 4) | await defense | 1 |
| 33 | 1 | defend 15 (value 2) | 4 vs 2; discard [15]; no life loss | 1 |
| 34 | 1 | play 16 (value 2) | draw 13 | 0 |
| 35 | 0 | attack 9 (value 4) | await defense | 1 |
| 36 | 1 | defend 16 (value 2) | 4 vs 2; discard [16]; no life loss | 1 |
| 37 | 1 | play 17 (value 2) | draw 21 | 0 |
| 38 | 0 | attack 9 (value 4) | await defense | 1 |
| 39 | 1 | defend 17 (value 2) | 4 vs 2; discard [17]; no life loss | 1 |
| 40 | 1 | play 21 (value 4) | draw 22 | 0 |
| 41 | 0 | attack 9 (value 4) | await defense | 1 |
| 42 | 1 | defend 21 (value 4) | 4 vs 4; discard [9, 21]; no life loss | 1 |
| 43 | 1 | play 22 (value 4) | no draw | 0 |
| 44 | 0 | play 10 (value 4) | draw 7 | 1 |
| 45 | 1 | attack 22 (value 4) | await defense | 0 |
| 46 | 0 | defend 10 (value 4) | 4 vs 4; discard [22, 10]; no life loss | 0 |
| 47 | 0 | play 7 (value 3) | no draw | 1 |
| 48 | 1 | play 12 (value 1) | no draw | 0 |
| 49 | 0 | attack 7 (value 3) | await defense | 1 |
| 50 | 1 | defend 12 (value 1) | 3 vs 1; discard [12]; no life loss | 1 |
| 51 | 1 | play 13 (value 1) | no draw | 0 |
| 52 | 0 | attack 7 (value 3) | await defense | 1 |
| 53 | 1 | defend 13 (value 1) | 3 vs 1; discard [13]; no life loss | 1 |
| 54 | 1 | play 14 (value 1) | no draw | 0 |
| 55 | 0 | attack 7 (value 3) | await defense | 1 |
| 56 | 1 | defend 14 (value 1) | 3 vs 1; discard [14]; no life loss | 1 |
| 57 | 1 | pass | no hand/board; consecutive passes 1 | 0 |
| 58 | 0 | attack 7 (value 3) | direct damage; lives [2, 2] | 1 |
| 59 | 1 | pass | no hand/board; consecutive passes 1 | 0 |
| 60 | 0 | attack 7 (value 3) | direct damage; lives [2, 1] | 1 |
| 61 | 1 | pass | no hand/board; consecutive passes 1 | 0 |
| 62 | 0 | attack 7 (value 3) | direct damage; lives [2, 0] | none |

Expected result: `{'outcome': 'win', 'winner': 0, 'reason': 'zero_lives'}`. Final lives `[2, 0]`; boards `[[7], []]`; hands `[[1, 0, 2], []]`; decks `[[], []]`; discards `[[6, 8, 4, 5, 3, 11, 9, 10], [23, 18, 19, 20, 15, 16, 17, 21, 22, 12, 13, 14]]`. Phase terminal, pending absent, action_count 62, passes 0, active player 0, acting player absent.

Review: config captures every setup choice; seed supplies all random choices; no hidden manual input or engine timing is required. The trace includes defender selection and a terminal action. M1 reproduces these exact zones, counters, and result in C++, proves repeated-run determinism, and tests rejection without mutation. Cross-runtime parity belongs to later milestones.
