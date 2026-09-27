# Vision and scope

## Product direction

Build a family of competitive, multiplatform games on a reusable platform. Begin with simple card games and increase complexity after validating fun, retention, PvP, and economy. Keep control of game logic, servers, matchmaking, progression, and economy.

The intended product is free-to-play with offline/PvE, PvP, global ranked play, expansions, cosmetics, and digital entitlements. Steam, iOS, and Android are publishing targets. Consoles, especially Nintendo Switch, follow traction and developer/SDK access.

## Purpose of the first MVP

Validate the architecture and development workflow end to end, rather than product depth. Use a tiny 1v1 card game to exercise:

- A reusable, deterministic C++ core and fast headless simulation.
- A Godot client with offline play against bots.
- Authoritative online matches, matchmaking, rating, leaderboard, and basic history.
- A store that grants a free entitlement and persists it across client restarts.
- An AI training pipeline whose resulting model can play as an in-game bot.
- Builds using the same core across desktop, Android, and development iOS.

The integrated success flow is a new user opening an installable build, playing offline, completing ranked play, seeing their rating, and claiming free content. The same core must also run parallel headless training environments.

## MVP boundaries

Use rectangles and text for cards; no art is needed for the MVP. Keep the game deliberately small. Validate store/inventory behavior without real money. Keep casual P2P out of the online milestone unless a concrete need emerges. Avoid public distribution fees during platform validation; Switch is outside the MVP.

## Later capabilities

The wider platform may support casual host/P2P games, seasons, decks, progression, administration, paid expansions/content/cosmetics, and more complex games. Payments belong behind platform adapters: the backend validates receipts and grants global-account entitlements using the provider required by each platform.

AI can evolve from random and heuristic bots to search/MCTS where useful, neural policies, self-play, and league training. Possible applications include sparring, adaptive tutorials, coaching, and balance analysis. Future real-time games should advance through controlled ticks, independently of rendering FPS.

Art can be produced externally. Retain artistic masters and distribute only processed runtime assets. Reuse the platform for Game 2 without rewriting the core architecture.
