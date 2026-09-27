extends SceneTree
# Exercise the asynchronous presentation path, including interrupted animations.
var failures := 0
var actions := 0
var kinds := {}
var combat_outcomes := {}

func check(ok: bool, message: String) -> void:
	if not ok:
		failures += 1
		push_error(message)

func _initialize() -> void:
	call_deferred("run_checks")

func settle(scene: Control) -> void:
	var frames := 0
	while scene.busy and frames < 240:
		await process_frame
		frames += 1
	check(not scene.busy, "Animation queue finishes")
	check(scene.animation.ghosts.is_empty(), "Transient cards cleaned up")

func choose(scene: Control) -> void:
	var action: Dictionary = scene.legal[0]
	if action.kind != "pass":
		var card: CardView = scene.table.card_nodes()[action.card]
		card.mouse_entered.emit()
		check(card.hovered, "Card hover feedback")
		var click := InputEventMouseButton.new()
		click.button_index = MOUSE_BUTTON_LEFT
		click.pressed = true
		card._gui_input(click)
		check(scene.selected_id == action.card, "Card click selects")
	check(scene.action_buttons[0].visible, "Selection reveals confirmation")
	scene.action_buttons[0].pressed.emit()

func verify_view(scene: Control) -> void:
	var o: Dictionary = scene.game.observe(scene.human)
	var table: TableView = scene.table
	check(table.own_hand.cards.size() == o.own_hand.size(), "Own hand synchronized")
	check(table.enemy_hand.cards.size() == o.players[1 - scene.human].hand_count, "Opponent backs synchronized")
	for card in table.enemy_hand.cards:
		check(card.face_down and card.card_id == -1 and card.value == 0, "Opponent hand remains anonymous")
	check(table.own_deck.count == o.players[scene.human].deck_count, "Own deck count")
	check(table.enemy_deck.count == o.players[1 - scene.human].deck_count, "Enemy deck count")
	check(table.own_discard.count == o.players[scene.human].discard.size(), "Own discard count")
	check(table.enemy_discard.count == o.players[1 - scene.human].discard.size(), "Enemy discard count")
	for card in table.card_nodes().values():
		check(card.visible, "Persistent card restored after animation")

func run_checks() -> void:
	var scene = load("res://main.tscn").instantiate()
	root.add_child(scene)
	scene.set_process(false)
	scene.animation.travel_duration = 0.005
	scene.animation.impact_duration = 0.005
	for bot in range(2):
		for seat in range(2):
			scene.bot_select.selected = bot
			scene.seat_select.selected = seat
			scene.seed_input.text = "0"
			scene.restart()
			for step in range(200):
				var before: Dictionary = scene.game.observe(seat)
				verify_view(scene)
				if before.phase == "terminal":
					break
				if before.acting_player == seat:
					choose(scene)
				else:
					scene.advance_bot()
				var after: Dictionary = scene.game.observe(seat)
				var replay: Dictionary = JSON.parse_string(scene.game.export_replay())
				var action: Dictionary = replay.actions.back()
				kinds[action.type] = true
				if action.type == "defend":
					var lost0: int = after.players[0].discard.size() - before.players[0].discard.size()
					var lost1: int = after.players[1].discard.size() - before.players[1].discard.size()
					combat_outcomes["%s/%s" % [lost0, lost1]] = true
				if action.type == "attack" and before.players[1 - action.actor].board.is_empty():
					kinds["direct"] = true
				check(scene.busy, "Input locked while animating")
				var count: int = after.action_count
				scene.advance_bot()
				scene.submit_action({"actor": seat, "kind": "pass", "card": -1})
				check(scene.game.observe(seat).action_count == count, "Duplicate input blocked")
				await settle(scene)
				actions += 1
			check(scene.game.observe(seat).phase == "terminal", "Animated match completes")
	for kind in ["play", "attack", "defend", "direct"]:
		check(kinds.has(kind), "Animated action coverage: " + kind)
	check(combat_outcomes.size() == 3, "Both survivor directions and ties animated")
	# Restart and replay import invalidate in-flight animation continuations.
	scene.seat_select.selected = 0
	scene.restart()
	scene.animation.travel_duration = 0.08
	choose(scene)
	check(scene.busy, "Restart test starts in flight")
	scene.restart()
	var fresh: String = scene.game.export_replay()
	await create_timer(0.3).timeout
	check(not scene.busy and scene.game.export_replay() == fresh, "Restart cancels stale work")
	verify_view(scene)
	choose(scene)
	var fixture: String = OS.get_cmdline_user_args()[0]
	scene.load_replay_file(fixture)
	var imported: String = scene.game.export_replay()
	await create_timer(0.3).timeout
	check(scene.replay_mode and not scene.busy and scene.game.export_replay() == imported, "Import cancels stale work")
	verify_view(scene)
	print("Tabletop checks: 4 animated matches, ", actions, " actions, combat outcomes=", combat_outcomes.size(), ", failures=", failures)
	scene.queue_free()
	await process_frame
	quit(0 if failures == 0 else 1)
