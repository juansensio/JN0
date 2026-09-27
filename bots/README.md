# Baseline bots

`janus_bots` links the pure rules core. `janus/bots.hpp` exposes RandomBot and HeuristicBot. Both choose from supplied legal actions using only the acting player's owned Observation; neither receives GameState, deck order, opponent hand, or the game seed.

RandomBot takes an explicit independent policy seed and uses SplitMix64 with rejection sampling for uniform action selection. It is reproducible without the standard library's implementation-dependent distributions. HeuristicBot is deterministic: establish strong cards, prefer direct damage and attacks that beat every visible defender, avoid losing attacks while deployment is possible, and defend with the cheapest winning card, a tie, or the weakest sacrifice. Equal scores retain core legal-action order. These are policy preferences, not duplicated action validation or combat transitions.

Callers must supply the core legal actions for the observation's actor. Empty lists throw `std::invalid_argument`; callers stop on terminal results. See [M2 evaluation](../docs/log/m2-validation.md) and [CLI instructions](../README.md).
