# M0 core contract

Public declarations: `core/include/janus/types.hpp` and `game.hpp`. Standard: C++20; CMake >= 3.20. Only configuration validation is implemented in M0. Game methods have no definitions until M1; calls fail at link time rather than simulate placeholder gameplay.

## Types and invariants

PlayerId is 0/1; other underlying values are invalid. CardId is uint16, valid range 0–23; owner is ID / 12 and value follows the canonical slot. Each card occurs in exactly one of its owner's deck, hand, board, discard. Values are 1–4, lives 0–3, board size 0–3, hand/deck/discard counts 0–12. Zones are ordered vectors. GameConfig contains version and every setup field. Rules v1 accepts exactly its declared defaults; unsupported version takes precedence over unsupported setup. Other variants require a rules version.

GameState owns complete state, config/seed, player zones/lives, main-turn owner, phase, pending attack, pass/action counters, and result. Pending attack exists exactly in awaiting_defense and references an active-player board card; entering defense does not move it. Ongoing/draw results have no winner; wins identify the surviving player and zero_lives reason. Terminal state is immutable until reset. Pass count is 0–2; reachable finite MVP matches cannot overflow uint64 action_count.

Observation owns distinct data: PublicPlayer contains lives, deck_count, hand_count, ordered board/discard. Observation contains own_hand, config, viewer, active/acting players, phase, pending attack, pass/action counters, result. It has no deck, opponent hand, seed, RNG state, pointers, or state references. Snapshot and replays are privileged and must not be delivered to active opponents. Game never accepts externally edited state.

## Lifecycle and validation

- `Game(config)` validates, throws `std::invalid_argument` on invalid configuration, and initializes with `reset(0)`. No OS entropy.
- `reset(Seed)` replaces all state using constructor config and explicit seed.
- `observe(PlayerId)` returns an owned snapshot; invalid player throws `std::invalid_argument`.
- `legal_actions(PlayerId)` throws on invalid ID; terminal/nonacting players get an empty vector. Main actions list Play by ascending CardId then Attack by ascending CardId; only Pass when neither exists. Defense lists Defend by ascending CardId.
- `step(Action)` validates before mutation; returns error and current result. Caller authentication belongs to adapters/server, which must bind actor to authenticated caller. Rejection leaves complete state unchanged. Accepted actions resolve all forced effects atomically.
- `result()` returns GameResult; `snapshot()` returns an owned full GameState for trusted debugging/simulation.

Action is actor plus exactly one payload: Play(card), Attack(card), Defend(card), Pass(no card). Error precedence: terminal, invalid_actor, wrong_actor, wrong_phase, unknown_card (outside 0–23), wrong_zone (absent from required own zone), board_full (Play), pass_not_allowed (Pass with a main action available). Check only applicable errors. Success returns none. No implicit pass/defense.

Cloning, rewards, state serialization implementation, bindings, and network transport are outside M0. Replay encoding never depends on enum/variant memory layout.
