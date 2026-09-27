class_name AnimationDirector
extends Node

@export var travel_duration := 0.32
@export var impact_duration := 0.16
var enabled := true
var generation := 0
var tweens: Array[Tween] = []
var ghosts: Array[CardView] = []
var impacts: Array[Control] = []

func cancel() -> void:
	generation += 1
	for tween in tweens:
		if tween.is_valid():
			tween.kill()
	tweens.clear()
	for ghost in ghosts:
		ghost.queue_free()
	ghosts.clear()
	for effect in impacts:
		effect.queue_free()
	impacts.clear()

func move(card: Control, destination: Vector2, duration: float, end_scale := 1.0) -> void:
	var tween := create_tween().set_parallel(true)
	tweens.append(tween)
	tween.set_trans(Tween.TRANS_CUBIC).set_ease(Tween.EASE_IN_OUT)
	tween.tween_property(card, "position", destination, duration)
	tween.tween_property(card, "rotation", 0.0, duration)
	tween.tween_property(card, "scale", Vector2.ONE * end_scale, duration)
	# Timer also completes if restart cancels the tween.
	await get_tree().create_timer(duration).timeout

func ghost(table: TableView, data: Dictionary, position: Vector2, back := false) -> CardView:
	var view: CardView = preload("res://components/card_view.tscn").instantiate()
	table.effects.add_child(view)
	view.configure(data, back, false, Color("f1c879"))
	view.mouse_filter = Control.MOUSE_FILTER_IGNORE
	view.position = position
	view.pivot_offset = Vector2.ZERO
	ghosts.append(view)
	return view

func animate(table: TableView, before: Dictionary, after: Dictionary, action: Dictionary, human: int) -> void:
	if not enabled:
		return
	var ticket := generation
	var nodes := table.card_nodes()
	var actor: int = action.actor
	var friendly := actor == human
	var board: BoardView = table.own_board if friendly else table.enemy_board
	var deck: PileView = table.own_deck if friendly else table.enemy_deck
	var origin := Vector2.ZERO
	var data := {}
	if nodes.has(action.card):
		var original: CardView = nodes[action.card]
		origin = original.global_position - table.global_position
		data = {"id": original.card_id, "value": original.value}
		original.visible = false
	if action.kind == "play":
		for item in after.players[actor].board:
			if item.id == action.card:
				data = item
		if not friendly:
			origin = table.enemy_hand.position + Vector2(table.enemy_hand.size.x / 2 - 56, 80)
		var moving := ghost(table, data, origin, not friendly)
		var index: int = after.players[actor].board.size() - 1
		await move(moving, board.position + Vector2(board.size.x / 2 - 190 + index * 134, 0), travel_duration)
		if ticket != generation:
			return
		moving.face_down = false
		moving.queue_redraw()
		if after.players[actor].deck_count < before.players[actor].deck_count:
			var drawn: Dictionary = after.own_hand.back() if friendly else {}
			var draw_view := ghost(table, drawn, deck.position, true)
			var hand: HandView = table.own_hand if friendly else table.enemy_hand
			await move(draw_view, hand.position + Vector2(hand.size.x / 2 - 56, 0 if friendly else 80), travel_duration)
	elif action.kind == "attack":
		var moving := ghost(table, data, origin)
		await move(moving, Vector2(origin.x, table.size.y / 2 - 78), travel_duration)
		if ticket != generation:
			return
		if after.phase == "awaiting_defense":
			await move(moving, origin, travel_duration * 0.6)
		else:
			await pulse(table.enemy_hud if friendly else table.own_hud)
			if ticket != generation:
				return
			await move(moving, origin, travel_duration * 0.6)
	elif action.kind == "defend":
		var attacker_id: int = before.pending_attack.card
		var attacker: CardView = nodes[attacker_id]
		var attacker_origin := attacker.global_position - table.global_position
		attacker.visible = false
		var attack_view := ghost(table, {"id": attacker.card_id, "value": attacker.value}, attacker_origin)
		var defend_view := ghost(table, data, origin)
		var center := table.size / 2 - Vector2(56, 78)
		# Slight overlap makes the clash visible without hiding either value.
		move(attack_view, center - Vector2(48, 0), travel_duration)
		await move(defend_view, center + Vector2(48, 0), travel_duration)
		if ticket != generation:
			return
		burst(table, table.size / 2)
		await pulse(table.status)
		if ticket != generation:
			return
		for pair in [[attack_view, attacker_origin, 1 - actor], [defend_view, origin, actor]]:
			var destroyed := false
			for discarded in after.players[pair[2]].discard:
				if discarded.id == pair[0].card_id:
					destroyed = true
			var pile: PileView = table.own_discard if pair[2] == human else table.enemy_discard
			await move(pair[0], pile.position if destroyed else pair[1], travel_duration, 0.72 if destroyed else 1.0)
			if ticket != generation:
				return
	elif action.kind == "pass":
		await pulse(table.status)
	if ticket == generation:
		cancel()

func pulse(node: Control) -> void:
	var tween := create_tween()
	tweens.append(tween)
	tween.tween_property(node, "modulate", Color("ff9477"), impact_duration)
	tween.tween_property(node, "modulate", Color.WHITE, impact_duration)
	await get_tree().create_timer(impact_duration * 2).timeout

func burst(table: TableView, location: Vector2) -> void:
	var effect := ImpactEffect.new()
	table.effects.add_child(effect)
	effect.position = location
	effect.z_index = -1
	effect.scale = Vector2.ONE * 0.8
	impacts.append(effect)
	var tween := create_tween().set_parallel(true)
	tweens.append(tween)
	tween.tween_property(effect, "scale", Vector2.ONE * 1.3, impact_duration * 2)
	tween.tween_property(effect, "modulate:a", 0.0, impact_duration * 2)
