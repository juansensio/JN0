extends Control
# Match orchestration only. Views consume observations; JanusGame owns all rules.
var game = JanusGame.new()
var human := 0
var bot_kind := "heuristic"
var replay_mode := false
var bot_wait := 0.0
var busy := false
var session := 0
var animations_enabled := true
var table: TableView
var animation: AnimationDirector
var status_label: Label
var notice: Label
var seed_input: LineEdit
var bot_select: OptionButton
var seat_select: OptionButton
var load_dialog: FileDialog
var settings_panel: MatchSettings
var last_action := ""
var legal: Array = []
var selected_id := -1
# Explicit action controls remain inspectable and available to integration automation.
var action_buttons: Array[Button] = []

func _ready() -> void:
	add_child(game)
	set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	var background := ColorRect.new()
	background.color = Color("101d29")
	background.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	background.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(background)
	table = preload("res://components/table_view.tscn").instantiate()
	add_child(table)
	table.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	table.offset_top = 62
	table.card_selected.connect(select_card)
	status_label = table.status
	notice = table.notice
	animation = AnimationDirector.new()
	add_child(animation)
	var header := HBoxContainer.new()
	header.position = Vector2(30, 12)
	header.add_theme_constant_override("separation", 22)
	add_child(header)
	label_in(header, "JANUS NOISE", 25)
	label_in(header, "OFFLINE  /  TABLETOP", 13)
	button_in(header, "Match settings", func(): settings_panel.visible = not settings_panel.visible)
	button_in(header, "New / Restart", restart)
	settings_panel = preload("res://components/match_settings.tscn").instantiate()
	add_child(settings_panel)
	seed_input = settings_panel.seed_input
	bot_select = settings_panel.bot_select
	seat_select = settings_panel.seat_select
	settings_panel.start_requested.connect(restart)
	settings_panel.save_requested.connect(save_replay)
	settings_panel.load_requested.connect(func(): load_dialog.popup_centered_ratio(0.7))
	load_dialog = FileDialog.new()
	load_dialog.access = FileDialog.ACCESS_FILESYSTEM
	load_dialog.file_mode = FileDialog.FILE_MODE_OPEN_FILE
	load_dialog.filters = PackedStringArray(["*.json ; Janus replay"])
	load_dialog.file_selected.connect(load_replay_file)
	add_child(load_dialog)
	restart()

func label_in(parent: Node, text: String, font_size: int) -> Label:
	var label := Label.new()
	label.text = text
	label.add_theme_font_size_override("font_size", font_size)
	parent.add_child(label)
	return label

func button_in(parent: Node, text: String, callback: Callable) -> Button:
	var button := Button.new()
	button.text = text
	button.custom_minimum_size.y = 36
	button.pressed.connect(callback)
	parent.add_child(button)
	return button

func clear_actions() -> void:
	action_buttons.clear()
	for child in table.action_bar.get_children():
		table.action_bar.remove_child(child)
		child.queue_free()

func stop_presentation() -> void:
	session += 1
	animation.cancel()
	busy = false
	selected_id = -1
	status_label.modulate = Color.WHITE
	table.own_hud.modulate = Color.WHITE
	table.enemy_hud.modulate = Color.WHITE

func restart() -> void:
	var reply: Dictionary = game.start(seed_input.text)
	if not reply.ok:
		notice.text = reply.error
		return
	stop_presentation()
	human = seat_select.selected
	bot_kind = "random" if bot_select.selected == 1 else "heuristic"
	replay_mode = false
	bot_wait = 0.65
	last_action = "Hover a card, click to select, then confirm its action."
	settings_panel.hide()
	refresh()

func refresh() -> void:
	table.arrange()
	var o: Dictionary = game.observe(human)
	legal = game.legal_actions(human) if not replay_mode and not busy else []
	selected_id = -1
	table.present(o, human, legal, bot_kind)
	clear_actions()
	if o.result.outcome == "win":
		status_label.text = "You win!" if o.result.winner == human else "Opponent wins."
	elif o.result.outcome == "draw":
		status_label.text = "Draw · both players are stuck."
	elif replay_mode:
		status_label.text = "Replay loaded · %s accepted actions" % o.action_count
	elif o.phase == "awaiting_defense":
		status_label.text = "Choose a defender" if o.acting_player == human else "Opponent is choosing a defender…"
	else:
		status_label.text = "Your turn · play or attack" if o.acting_player == human else "Opponent's turn…"
	notice.text = "%s   |   Action %s" % [last_action, o.action_count]
	# Each button maps directly to a supplied legal action. Selection reveals one.
	for action in legal:
		var button := button_in(table.action_bar, "%s selected card" % action.kind.capitalize(), submit_action.bind(action))
		button.visible = action.kind == "pass"
		if action.kind == "pass":
			button.text = "Pass"
		action_buttons.append(button)

func select_card(id: int) -> void:
	if busy or replay_mode:
		return
	selected_id = id if selected_id != id else -1
	table.select(selected_id)
	for index in range(legal.size()):
		action_buttons[index].visible = legal[index].card == selected_id or legal[index].kind == "pass"

func submit_action(action: Dictionary) -> void:
	if busy or replay_mode:
		return
	var before: Dictionary = game.observe(human)
	var reply: Dictionary = game.submit(action.actor, action.kind, action.card)
	if not reply.ok:
		last_action = reply.error
		refresh()
		return
	await present_action(before, reply.action, "You")

func advance_bot() -> void:
	var before: Dictionary = game.observe(human)
	if busy or replay_mode or before.acting_player != 1 - human:
		return
	var reply: Dictionary = game.bot_step(before.acting_player, bot_kind)
	if not reply.ok:
		last_action = reply.error
		refresh()
		return
	await present_action(before, reply.action, "Opponent")

func present_action(before: Dictionary, action: Dictionary, who: String) -> void:
	var ticket := session
	busy = true
	clear_actions()
	for card in table.card_nodes().values():
		card.available = false
		card.queue_redraw()
	status_label.position.y = table.size.y * 0.19 - 38
	status_label.text = {"play": "Playing card…", "attack": "Attacking…", "defend": "Resolving combat…", "pass": "Passing…"}[action.kind]
	last_action = "%s: %s" % [who, action.kind]
	notice.text = last_action
	animation.enabled = animations_enabled
	await animation.animate(table, before, game.observe(human), action, human)
	if ticket != session:
		return
	busy = false
	bot_wait = 0.65
	refresh()

func _process(delta: float) -> void:
	if busy or replay_mode:
		return
	bot_wait -= delta
	if bot_wait <= 0.0:
		bot_wait = 0.65
		advance_bot()

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
	stop_presentation()
	replay_mode = true
	settings_panel.hide()
	last_action = "Replay verified by the shared core. Restart to play again."
	refresh()
