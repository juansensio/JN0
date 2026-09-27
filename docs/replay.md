# Replay format 1 / rules 1

[Schema](../protocol/replay-v1.schema.json), [complete fixture](../tests/fixtures/replay-v1.json), [trace review](replay-example.md). Replay is config + explicit seed + ordered accepted actions. Reset/setup is implicit; no state snapshot, external choices, FPS, callbacks, or timestamps are needed. Replay structs are declared in game.hpp; parsing/execution are M1 work.

## RNG and setup

Use SplitMix64 with uint64 modular arithmetic. Initialize state to seed. For each next sample:

```text
state = state + 0x9e3779b97f4a7c15
z = state
z = (z xor (z >> 30)) * 0xbf58476d1ce4e5b9
z = (z xor (z >> 27)) * 0x94d049bb133111eb
return z xor (z >> 31)
```

All additions/products wrap modulo 2^64; shifts are unsigned. For bounded(n), n > 0, compute threshold `(2^64 - n) mod n`; draw x until x >= threshold, then return x mod n. Rejected samples still advance state. Avoid std::shuffle and standard distributions whose consumption is not portable.

Fisher–Yates: for i = 11 down to 1, swap positions i and bounded(i+1). Shuffle canonical player 0 deck then player 1 deck using one continuous stream. Draw four from each front without random calls. First player is bounded(2). No randomness occurs after setup in rules v1. Seed 0's first three raw samples are `e220a8397b1dcdaf`, `6e789e6aa1b965f4`, `06c45d188009454f` (hex). Seed 0 starter and full shuffled ID orders are in the fixture review, for M1 golden tests.

## Encoding

UTF-8 JSON object, strict field names/types; object key order and whitespace are insignificant. Reject duplicate keys and unknown fields. `format_version` and `config.rules_version` are integers 1. Config includes starting_lives 3, initial_hand_size 4, board_capacity 3, deck_values `[1,1,1,2,2,2,3,3,3,4,4,4]`. Config validation must match core validation.

Seed is a decimal string of uint64 (0 through 18446744073709551615), no sign, whitespace, or leading zeros except `"0"`; this avoids JavaScript numeric precision loss. Actor is integer 0/1. Card is integer 0–23. Action type is `play`, `attack`, `defend`, or `pass`; card is required for the first three and forbidden for pass. Actions are ordered, never sorted. Failed attempts are not recorded because they do not transition state.

Optional expected_result is a test assertion, never an input deciding a winner: outcome `win` with winner 0/1 and reason `zero_lives`, or outcome `draw` with winner null and reason `both_players_stuck`. The JSON schema limits shapes and ranges; the loader must additionally enforce seed upper bound, supported versions/config, and sequence legality.

## Compatibility and failures

Reject malformed JSON/fields/ranges before execution. Reject unknown format/rules versions rather than silently upgrade. Execute only in a fresh Game reset from the supplied config/seed. Reject the first illegal action with its zero-based index/core error; never skip or auto-correct it. Actions after terminal are illegal. Complete-match replay must end terminal; truncated/ongoing logs may be diagnostic but cannot pass complete-match validation. If expected_result is present, require exact agreement. Return no partially reconstructed match on failure; do not mutate an existing game.

Any change to setup, RNG consumption, identity/order, transition, visibility, or outcomes requires a new rules version. Encoding changes require a format version. The complete fixture was cross-checked as an M0 specification trace; executing it on the real C++ core and proving determinism remain M1 gates.
