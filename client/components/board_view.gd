class_name BoardView
extends Control
signal card_selected(card_id: int)
@export var accent := Color("62d6be")
var cards: Array[CardView] = []

func _ready() -> void:
	resized.connect(arrange)

func present(data: Array, legal: Array, attacking: int) -> void:
	for card in cards:
		remove_child(card)
		card.queue_free()
	cards.clear()
	for item in data:
		var card: CardView = preload("res://components/card_view.tscn").instantiate()
		add_child(card)
		card.configure(item, false, item.id in legal, accent)
		card.highlighted = item.id == attacking
		card.selected.connect(func(id): card_selected.emit(id))
		cards.append(card)
	arrange()

func arrange() -> void:
	for index in range(cards.size()):
		var card := cards[index]
		card.base_position = Vector2(size.x / 2 - 190 + index * 134, 0)
		card.position = card.base_position
	queue_redraw()

func select(id: int) -> void:
	for card in cards:
		card.set_chosen(card.card_id == id)

func _draw() -> void:
	for index in range(3):
		var slot := Rect2(Vector2(size.x / 2 - 190 + index * 134, 0), Vector2(112, 156))
		var style := StyleBoxFlat.new()
		style.bg_color = Color(0.03, 0.09, 0.13, 0.28)
		style.border_color = Color(accent, 0.16)
		style.set_border_width_all(1)
		style.set_corner_radius_all(10)
		draw_style_box(style, slot)
