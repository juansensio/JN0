extends Control

var game = JanusGame.new()
var human := 0
var bot_kind := "heuristic"
var replay_mode := false
var bot_wait := 0.0
var title_label: Label
var status_label: Label
var notice: Label
var opponent_info: Label
var own_info: Label
var opponent_board: HBoxContainer
var own_board: HBoxContainer
var hand: HBoxContainer
var actions: HBoxContainer
var seed_input: LineEdit
var bot_select: OptionButton
var seat_select: OptionButton
var load_dialog: FileDialog
var last_action := ""
var action_buttons: Array[Button] = []

func _ready() -> void:
	add_child(game)
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	var background := ColorRect.new()
	background.color = Color("101924")
	background.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	background.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(background)
	var scroll := ScrollContainer.new()
	scroll.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(scroll)
	var margin := MarginContainer.new()
	margin.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	for side in ["left", "right", "top", "bottom"]:
		margin.add_theme_constant_override("margin_" + side, 20)
	scroll.add_child(margin)
	var column := VBoxContainer.new()
	column.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	column.add_theme_constant_override("separation", 12)
	margin.add_child(column)
	title_label = label_in(column, "JANUS NOISE   /   Offline PvE", 28)
	label_in(column, "Play a card or attack. When attacked, choose a card to defend.", 16)
	var toolbar := HBoxContainer.new()
	column.add_child(toolbar)
	label_in(toolbar, "Seed", 16)
	seed_input = LineEdit.new()
	seed_input.text = "42"
	seed_input.custom_minimum_size.x = 190
	toolbar.add_child(seed_input)
	bot_select = OptionButton.new()
	bot_select.add_item("HeuristicBot")
	bot_select.add_item("RandomBot")
	toolbar.add_child(bot_select)
	seat_select = OptionButton.new()
	seat_select.add_item("You: Player 1")
	seat_select.add_item("You: Player 2")
	toolbar.add_child(seat_select)
	button_in(toolbar, "New / Restart", restart)
	button_in(toolbar, "Save replay", save_replay)
	button_in(toolbar, "Load replay", func(): load_dialog.popup_centered_ratio(0.7))
	status_label = label_in(column, "", 22)
	opponent_info = label_in(column, "", 18)
	opponent_board = HBoxContainer.new()
	column.add_child(opponent_board)
	own_info = label_in(column, "", 18)
	own_board = HBoxContainer.new()
	column.add_child(own_board)
	label_in(column, "Your hand", 18)
	hand = HBoxContainer.new()
	column.add_child(hand)
	actions = HBoxContainer.new()
	column.add_child(actions)
	notice = label_in(column, "", 16)
	notice.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	load_dialog = FileDialog.new()
	load_dialog.access = FileDialog.ACCESS_FILESYSTEM
	load_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	load_dialog.filters = PackedStringArray(["*.json ; Janus replay"])
	load_dialog.file_selected.connect(load_replay_file)
	add_child(load_dialog)
	restart()

func label_in(parent: Node, text: String, size: int) -> Label:
	var label := Label.new()
	label.text = text
	label.add_theme_font_size_override("font_size", size)
	label.add_theme_color_override("font_color", Color("e5edf5"))
	parent.add_child(label)
	return label

func button_in(parent: Node, text: String, callback: Callable) -> Button:
	var button := Button.new()
	button.text = text
	button.custom_minimum_size.y = 40
	button.pressed.connect(callback)
	parent.add_child(button)
	return button

func clear_row(row: Node) -> void:
	for child in row.get_children():
		row.remove_child(child)
		child.queue_free()

func restart() -> void:
	var reply: Dictionary = game.start(seed_input.text)
	if not reply.ok:
		notice.text = reply.error
		return
	human = seat_select.selected
	bot_kind = "random" if bot_select.selected == 1 else "heuristic"
	replay_mode = false
	bot_wait = 0.4
	last_action = "New match. All play runs locally."
	refresh()

func card_view(row: Node, card: Dictionary, choices: Array) -> void:
	var panel := PanelContainer.new()
	panel.custom_minimum_size = Vector2(142, 120)
	var style := StyleBoxFlat.new()
	style.bg_color = Color("23374a")
	style.border_color = Color("568da7")
	style.set_border_width_all(2)
	style.set_corner_radius_all(8)
	style.content_margin_left = 12
	style.content_margin_right = 12
	style.content_margin_top = 10
	style.content_margin_bottom = 10
	panel.add_theme_stylebox_override("panel", style)
	row.add_child(panel)
	var column := VBoxContainer.new()
	panel.add_child(column)
	label_in(column, "Value %s" % card.value, 24)
	label_in(column, "Card #%s" % card.id, 14)
	for action in choices:
		if action.card == card.id:
			var button := button_in(column, action.kind.capitalize(), submit_action.bind(action))
			action_buttons.append(button)

func refresh() -> void:
	var o: Dictionary = game.observe(human)
	var legal: Array = game.legal_actions(human) if not replay_mode else []
	action_buttons.clear()
	for row in [opponent_board, own_board, hand, actions]:
		clear_row(row)
	var other := 1 - human
	var enemy: Dictionary = o.players[other]
	var own: Dictionary = o.players[human]
	opponent_info.text = "Opponent · %s · Player %s   |   Lives %s   Hand %s   Deck %s   Discard %s" % [bot_kind.capitalize(), other + 1, enemy.lives, enemy.hand_count, enemy.deck_count, enemy.discard.size()]
	own_info.text = "You · Player %s   |   Lives %s   Deck %s   Discard %s" % [human + 1, own.lives, own.deck_count, own.discard.size()]
	for card in enemy.board:
		card_view(opponent_board, card, [])
	if enemy.board.is_empty():
		label_in(opponent_board, "Opponent board is empty", 16)
	for card in own.board:
		card_view(own_board, card, legal)
	if own.board.is_empty():
		label_in(own_board, "Your board is empty", 16)
	for card in o.own_hand:
		card_view(hand, card, legal)
	if o.own_hand.is_empty():
		label_in(hand, "Your hand is empty", 16)
	for action in legal:
		if action.kind == "pass":
			action_buttons.append(button_in(actions, "Pass", submit_action.bind(action)))
	if o.result.outcome == "win":
		status_label.text = "You win!" if o.result.winner == human else "Opponent wins."
	elif o.result.outcome == "draw":
		status_label.text = "Draw · both players are stuck."
	elif replay_mode:
		status_label.text = "Replay loaded · %s accepted actions" % o.action_count
	elif o.phase == "awaiting_defense":
		status_label.text = "Choose a defender · incoming card #%s" % o.pending_attack.card if o.acting_player == human else "Opponent is choosing a defender…"
	else:
		status_label.text = "Your turn · play or attack" if o.acting_player == human else "Opponent's turn…"
	notice.text = "%s   |   Action %s" % [last_action, o.action_count]

func submit_action(action: Dictionary) -> void:
	var reply: Dictionary = game.submit(action.actor, action.kind, action.card)
	if reply.ok:
		last_action = "You: %s%s" % [action.kind, " #%s" % action.card if action.card >= 0 else ""]
	else:
		last_action = reply.error
	bot_wait = 0.4
	refresh()

func advance_bot() -> void:
	var o: Dictionary = game.observe(human)
	if replay_mode or o.acting_player != 1 - human:
		return
	var reply: Dictionary = game.bot_step(o.acting_player, bot_kind)
	if reply.ok:
		last_action = "Opponent: %s%s" % [reply.action.kind, " #%s" % reply.action.card if reply.action.card >= 0 else ""]
	else:
		last_action = reply.error
	refresh()

func _process(delta: float) -> void:
	bot_wait -= delta
	if bot_wait <= 0.0:
		advance_bot()
		bot_wait = 0.4

func save_replay() -> void:
	var file := FileAccess.open("user://last-match.json", FileAccess.WRITE)
	if file == null:
		notice.text = "Could not save replay."
		return
	file.store_string(game.export_replay())
	notice.text = "Replay saved: " + ProjectSettings.globalize_path("user://last-match.json")

func load_replay_file(path: String) -> void:
	var file := FileAccess.open(path, FileAccess.READ)
	if file == null:
		notice.text = "Could not open replay."
		return
	var reply: Dictionary = game.load_replay(file.get_as_text())
	if not reply.ok:
		notice.text = reply.error
		return
	replay_mode = true
	last_action = "Replay verified by the shared core. Restart to play again."
	refresh()
