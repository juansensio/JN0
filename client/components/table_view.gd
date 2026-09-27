class_name TableView
extends Control
signal card_selected(card_id: int)
var own_hand: HandView
var enemy_hand: HandView
var own_board: BoardView
var enemy_board: BoardView
var own_deck: PileView
var enemy_deck: PileView
var own_discard: PileView
var enemy_discard: PileView
var own_hud: PlayerHUD
var enemy_hud: PlayerHUD
var status: Label
var action_bar: HBoxContainer
var notice: Label
var effects: Control

func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	clip_contents = true
	own_hand = component("hand_view")
	enemy_hand = component("hand_view")
	enemy_hand.opponent = true
	enemy_hand.accent = Color("eea389")
	own_board = component("board_view")
	enemy_board = component("board_view")
	enemy_board.accent = Color("eea389")
	own_deck = component("pile_view")
	enemy_deck = component("pile_view")
	own_discard = component("pile_view")
	own_discard.discard = true
	enemy_discard = component("pile_view")
	enemy_discard.discard = true
	own_hud = component("player_hud")
	enemy_hud = component("player_hud")
	for zone in [own_hand, own_board]:
		zone.card_selected.connect(func(id): card_selected.emit(id))
	status = text_label(22)
	status.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	action_bar = HBoxContainer.new()
	action_bar.alignment = BoxContainer.ALIGNMENT_CENTER
	add_child(action_bar)
	notice = text_label(13)
	notice.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	notice.add_theme_color_override("font_color", Color("91aab5"))
	effects = Control.new()
	effects.mouse_filter = Control.MOUSE_FILTER_IGNORE
	effects.z_index = 100
	add_child(effects)
	resized.connect(arrange)
	arrange()

func component(name: String):
	var node = load("res://components/" + name + ".tscn").instantiate()
	add_child(node)
	return node

func text_label(font_size: int) -> Label:
	var label := Label.new()
	label.add_theme_font_size_override("font_size", font_size)
	add_child(label)
	return label

func place(node: Control, rect: Rect2) -> void:
	node.position = rect.position
	node.size = rect.size

func arrange() -> void:
	if own_hand == null:
		return
	var w := size.x
	var h := size.y
	place(enemy_hand, Rect2(170, -85, w - 340, 160))
	place(enemy_board, Rect2(160, h * 0.19, w - 320, 156))
	place(own_board, Rect2(160, h * 0.51, w - 320, 156))
	place(own_hand, Rect2(170, h - 204, w - 340, 170))
	place(enemy_deck, Rect2(48, 65, 100, 150))
	place(enemy_discard, Rect2(w - 130, 65, 100, 150))
	place(own_discard, Rect2(48, h - 202, 100, 150))
	place(own_deck, Rect2(w - 130, h - 202, 100, 150))
	place(enemy_hud, Rect2(190, 80, w - 380, 34))
	place(own_hud, Rect2(190, h - 255, w - 380, 34))
	place(status, Rect2(150, h * 0.425, w - 300, 34))
	place(action_bar, Rect2(150, h - 47, w - 300, 40))
	place(notice, Rect2(150, h * 0.425 + 36, w - 300, 25))
	effects.size = size
	queue_redraw()

func present(o: Dictionary, human: int, legal: Array, bot: String) -> void:
	var own: Dictionary = o.players[human]
	var enemy: Dictionary = o.players[1 - human]
	var legal_ids: Array = legal.map(func(a): return a.card)
	var attacker: int = o.pending_attack.get("card", -1)
	own_hand.present(o.own_hand, own.hand_count, legal_ids)
	enemy_hand.present([], enemy.hand_count, [])
	own_board.present(own.board, legal_ids, attacker)
	enemy_board.present(enemy.board, [], attacker)
	own_deck.present(own.deck_count)
	enemy_deck.present(enemy.deck_count)
	own_discard.present(own.discard.size(), own.discard)
	enemy_discard.present(enemy.discard.size(), enemy.discard)
	own_hud.present("YOU · P%s · %s cards" % [human + 1, own.hand_count], own.lives, o.acting_player == human)
	enemy_hud.present("%s · P%s · %s cards" % [bot.to_upper(), 2 - human, enemy.hand_count], enemy.lives, o.acting_player == 1 - human)

func select(id: int) -> void:
	own_hand.select(id)
	own_board.select(id)

func card_nodes() -> Dictionary:
	var result := {}
	for zone in [own_hand, own_board, enemy_board]:
		for card in zone.cards:
			result[card.card_id] = card
	return result

func _draw() -> void:
	draw_rect(Rect2(Vector2.ZERO, size), Color("101d29"))
	var felt := StyleBoxFlat.new()
	felt.bg_color = Color("193440")
	felt.border_color = Color("36515a")
	felt.set_border_width_all(2)
	felt.set_corner_radius_all(80)
	felt.shadow_color = Color(0, 0, 0, 0.3)
	felt.shadow_size = 20
	draw_style_box(felt, Rect2(22, 26, size.x - 44, size.y - 52))
	draw_line(Vector2(180, size.y / 2), Vector2(size.x - 180, size.y / 2), Color(0.6, 0.8, 0.8, 0.07), 1)
