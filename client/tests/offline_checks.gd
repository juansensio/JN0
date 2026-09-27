extends SceneTree

var failures := 0
func check(condition: bool, message: String) -> void:
	if not condition:
		failures += 1
		push_error(message)

func ids(cards: Array) -> Array:
	return cards.map(func(card): return card.id)

func _initialize() -> void:
	call_deferred("run_checks")

func run_checks() -> void:
	var args := OS.get_cmdline_user_args()
	if args.size() != 2:
		push_error("Expected fixture and output directory")
		quit(1)
		return
	var game := JanusGame.new()
	root.add_child(game)
	check(game.start("18446744073709551615").ok, "Full unsigned seed range")
	var before: String = game.export_replay()
	for seed in ["", "-1", "18446744073709551616", "42x"]:
		check(not game.start(seed).ok, "Bad seed rejected")
		check(game.export_replay() == before, "Bad seed leaves game unchanged")
	check(game.start("0").ok, "Seed zero")
	var o: Dictionary = game.observe(0)
	check(not o.has("seed") and not o.has("deck") and not o.players[1].has("hand"), "Hidden fields absent")
	check(o.own_hand.size() == 4 and o.players[1].hand_count == 4, "Visible hand/count conversion")
	var detached: Dictionary = game.observe(0)
	detached.own_hand.clear()
	check(game.observe(0).own_hand.size() == 4, "Owned observation")
	check(not game.observe(-1).ok and game.legal_actions(2).is_empty(), "Invalid viewer/actor")
	before = game.export_replay()
	for invalid in [[2, "play", 0], [0, "unknown", 0], [0, "play", -1], [0, "play", 65536], [0, "pass", -1], [0, "attack", 0]]:
		check(not game.submit(invalid[0], invalid[1], invalid[2]).ok, "Illegal action rejected")
		check(game.export_replay() == before, "Illegal action unchanged")
	check(not game.bot_step(0, "unknown").ok, "Invalid bot rejected")
	check(not game.bot_step(1, "random").ok and game.export_replay() == before, "Wrong bot actor unchanged")
	check(not game.load_replay("{}").ok and game.export_replay() == before, "Malformed replay atomic")
	var fixture := FileAccess.get_file_as_string(args[0])
	var parsed: Dictionary = JSON.parse_string(fixture)
	check(game.load_replay(fixture).ok, "Fixture import")
	o = game.observe(0)
	check(o.phase == "terminal" and o.acting_player == -1 and o.active_player == 0 and o.action_count == 62, "Fixture lifecycle")
	check(o.result == {"outcome": "win", "winner": 0, "reason": "zero_lives"}, "Fixture result")
	check(o.players[0].lives == 2 and o.players[1].lives == 0, "Fixture lives")
	check(ids(o.own_hand) == [1, 0, 2] and game.observe(1).own_hand.is_empty(), "Fixture hands")
	check(ids(o.players[0].board) == [7] and o.players[1].board.is_empty(), "Fixture boards")
	check(o.players[0].deck_count == 0 and o.players[1].deck_count == 0, "Fixture decks")
	check(ids(o.players[0].discard) == [6, 8, 4, 5, 3, 11, 9, 10], "Fixture first discard")
	check(ids(o.players[1].discard) == [23, 18, 19, 20, 15, 16, 17, 21, 22, 12, 13, 14], "Fixture second discard")
	before = game.export_replay()
	check(not game.submit(0, "attack", 7).ok and game.export_replay() == before, "Terminal rejection")
	check(game.legal_actions(0).is_empty() and game.legal_actions(1).is_empty(), "Terminal legal actions")
	check(game.start("0").ok, "Fixture manual replay reset")
	for action in parsed.actions:
		check(game.submit(action.actor, action.type, action.get("card", -1)).ok, "Fixture submit path")
	check(game.export_replay() == before, "Submitted fixture export parity")
	var bad: Dictionary = parsed.duplicate(true)
	bad.expected_result.winner = 1
	check(not game.load_replay(JSON.stringify(bad)).ok and game.export_replay() == before, "Wrong expected result atomic")
	bad = parsed.duplicate(true)
	bad.actions[0].card = 65535
	check(not game.load_replay(JSON.stringify(bad)).ok and game.export_replay() == before, "Illegal replay atomic")
	var partial := JanusGame.new()
	check(partial.start("18446744073709551615").ok, "Partial seed")
	var actor: int = partial.observe(0).acting_player
	check(partial.bot_step(actor, "random").ok, "Partial bot step")
	check(game.load_replay(partial.export_replay()).ok and game.observe(0) == partial.observe(0), "Partial replay roundtrip")
	partial.free()
	game.queue_free()

	# Exercise the real scene and its button callbacks, including pending defense.
	var scene = load("res://main.tscn").instantiate()
	root.add_child(scene)
	var completed := 0
	var defenses := 0
	for bot in range(2):
		for seat in range(2):
			for seed in range(8):
				scene.bot_select.selected = bot
				scene.seat_select.selected = seat
				scene.seed_input.text = str(seed)
				scene.restart()
				var steps := 0
				while scene.game.observe(seat).phase != "terminal" and steps < 200:
					var state: Dictionary = scene.game.observe(seat)
					if state.acting_player == seat:
						check(not scene.action_buttons.is_empty(), "Human has rendered legal buttons")
						if scene.action_buttons.is_empty():
							break
						if state.phase == "awaiting_defense":
							defenses += 1
						scene.action_buttons[0].pressed.emit()
					else:
						scene.advance_bot()
					steps += 1
				var final: Dictionary = scene.game.observe(seat)
				check(final.phase == "terminal", "UI complete match")
				check(scene.action_buttons.is_empty(), "Terminal has no action buttons")
				check(scene.status_label.text in ["You win!", "Opponent wins.", "Draw · both players are stuck."], "Result rendered")
				var replay: String = scene.game.export_replay()
				var file := FileAccess.open(args[1].path_join("m3-%s-%s-%s.json" % [bot, seat, seed]), FileAccess.WRITE)
				check(file != null, "Write parity artifact")
				if file:
					file.store_string(replay)
					file.close()
				var copy := JanusGame.new()
				check(copy.load_replay(replay).ok, "Generated replay import")
				check(copy.observe(0) == scene.game.observe(0) and copy.observe(1) == scene.game.observe(1), "Generated exact observation parity")
				copy.free()
				scene.load_replay_file(args[1].path_join("m3-%s-%s-%s.json" % [bot, seat, seed]))
				check(scene.replay_mode and scene.game.observe(seat) == final, "UI replay load")
				scene.advance_bot()
				check(scene.game.export_replay() == replay, "Replay view stays unchanged")
				completed += 1
				# Release detached visual nodes between games.
				await process_frame
	check(defenses > 0, "Human defense buttons exercised")
	scene.queue_free()
	print("M3 Godot checks: ", completed, " complete UI games, ", defenses, " human defenses, failures=", failures)
	quit(0 if failures == 0 else 1)
