class_name CardView
extends Control

signal selected(card_id: int)

@export var face_size := Vector2(112, 156)
@export var accent := Color("62d6be")
var card_id := -1
var value := 0
var face_down := false
var available := false
var highlighted := false
var chosen := false
var hovered := false
var base_position := Vector2.ZERO
var base_rotation := 0.0
var motion: Tween

func _ready() -> void:
	custom_minimum_size = face_size
	size = face_size
	pivot_offset = face_size / 2
	mouse_entered.connect(_hover.bind(true))
	mouse_exited.connect(_hover.bind(false))

func configure(card: Dictionary, back: bool, legal: bool, color: Color) -> void:
	card_id = card.get("id", -1)
	value = card.get("value", 0)
	face_down = back
	available = legal
	accent = color
	mouse_default_cursor_shape = Control.CURSOR_POINTING_HAND if legal else Control.CURSOR_ARROW
	queue_redraw()

func _gui_input(event: InputEvent) -> void:
	if available and event is InputEventMouseButton and event.button_index == MOUSE_BUTTON_LEFT and event.pressed:
		selected.emit(card_id)
		accept_event()

func _hover(on: bool) -> void:
	hovered = on and not face_down
	z_index = 30 if hovered or chosen else 0
	if motion:
		motion.kill()
	motion = create_tween().set_parallel(true)
	motion.tween_property(self, "position", base_position + Vector2(0, -22 if hovered or chosen else 0), 0.14)
	motion.tween_property(self, "scale", Vector2.ONE * (1.07 if hovered else 1.0), 0.14)
	motion.tween_property(self, "rotation", 0.0 if hovered or chosen else base_rotation, 0.14)
	queue_redraw()

func set_chosen(on: bool) -> void:
	chosen = on
	_hover(hovered)

func _draw() -> void:
	var rect := Rect2(Vector2.ZERO, face_size)
	var style := StyleBoxFlat.new()
	style.bg_color = Color("24394b") if face_down else Color("eee9db")
	style.border_color = accent if available or chosen or highlighted else Color("81909a")
	style.set_border_width_all(3 if available or highlighted or chosen else 1)
	style.set_corner_radius_all(10)
	style.shadow_color = Color(0, 0, 0, 0.35)
	style.shadow_size = 5
	style.shadow_offset = Vector2(0, 4)
	draw_style_box(style, rect)
	var font := ThemeDB.fallback_font
	if face_down:
		draw_rect(rect.grow(-10), Color("547181"), false, 1)
		for y in range(24, 140, 16):
			draw_line(Vector2(18, y), Vector2(94, y - 12), Color("395567"), 2)
		draw_circle(face_size / 2, 25, Color("182e40"))
		draw_string(font, Vector2(34, 85), "JN", HORIZONTAL_ALIGNMENT_LEFT, -1, 25, Color("a6c9d0"))
		return
	var ink := Color("213445")
	draw_string(font, Vector2(12, 29), str(value), HORIZONTAL_ALIGNMENT_LEFT, -1, 23, ink)
	draw_string(font, Vector2(84, 144), str(value), HORIZONTAL_ALIGNMENT_LEFT, -1, 23, ink)
	draw_circle(Vector2(56, 75), 33, Color("d8dfd8"))
	draw_arc(Vector2(56, 75), 33, 0, TAU, 48, accent.darkened(0.2), 2)
	draw_string(font, Vector2(36, 93), str(value), HORIZONTAL_ALIGNMENT_CENTER, 40, 49, ink)
	draw_string(font, Vector2(12, 121), "ATK / DEF", HORIZONTAL_ALIGNMENT_CENTER, 88, 11, ink.lightened(0.2))
	if highlighted:
		draw_string(font, Vector2(8, 52), "ATTACKING", HORIZONTAL_ALIGNMENT_CENTER, 96, 11, Color("ad472e"))
