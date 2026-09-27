class_name PileView
extends Control
@export var discard := false
@export var accent := Color("62d6be")
var count := 0
var top_card: CardView
var caption: Label

func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	top_card = preload("res://components/card_view.tscn").instantiate()
	add_child(top_card)
	top_card.pivot_offset = Vector2.ZERO
	top_card.scale = Vector2.ONE * 0.72
	top_card.mouse_filter = Control.MOUSE_FILTER_IGNORE
	caption = Label.new()
	caption.position = Vector2(-20, 121)
	caption.size = Vector2(145, 32)
	caption.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	caption.add_theme_font_size_override("font_size", 14)
	add_child(caption)

func present(amount: int, cards: Array = []) -> void:
	count = amount
	top_card.configure(cards.back() if discard and not cards.is_empty() else {}, not discard, false, accent)
	top_card.visible = count > 0
	caption.text = ("DISCARD  ·  " if discard else "DECK  ·  ") + str(count)
	queue_redraw()

func _draw() -> void:
	var style := StyleBoxFlat.new()
	style.bg_color = Color(0.02, 0.06, 0.1, 0.3)
	style.border_color = Color(accent, 0.25)
	style.set_border_width_all(1)
	style.set_corner_radius_all(8)
	draw_style_box(style, Rect2(0, 0, 81, 113))
	if count > 1:
		draw_style_box(style, Rect2(4, 4, 81, 113))
