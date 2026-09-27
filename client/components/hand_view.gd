class_name HandView
extends Control
signal card_selected(card_id: int)

@export var opponent := false
@export var accent := Color("62d6be")
var cards: Array[CardView] = []

func _ready() -> void:
	resized.connect(arrange)

func present(data: Array, count: int, legal: Array) -> void:
	for card in cards:
		remove_child(card)
		card.queue_free()
	cards.clear()
	for index in range(count if opponent else data.size()):
		var card: CardView = preload("res://components/card_view.tscn").instantiate()
		add_child(card)
		card.configure({} if opponent else data[index], opponent, not opponent and data[index].id in legal, accent)
		card.selected.connect(func(id): card_selected.emit(id))
		cards.append(card)
	arrange()

func arrange() -> void:
	var spacing := minf(86, maxf(20, (size.x - 120) / maxf(1, cards.size() - 1)))
	var width := 112 + spacing * (cards.size() - 1)
	for index in range(cards.size()):
		var card := cards[index]
		var offset := float(index) - float(cards.size() - 1) / 2.0
		card.base_position = Vector2((size.x - width) / 2 + index * spacing, absf(offset) * 7)
		card.base_rotation = offset * 0.055
		card.position = card.base_position
		card.rotation = card.base_rotation
		card.pivot_offset = card.face_size / 2

func select(id: int) -> void:
	for card in cards:
		card.set_chosen(card.card_id == id)
