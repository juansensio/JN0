class_name ImpactEffect
extends Control
# Short procedural burst: no textures or rule calculations.
@export var color := Color("f1c879")

func _ready() -> void:
	mouse_filter = Control.MOUSE_FILTER_IGNORE
	queue_redraw()

func _draw() -> void:
	draw_arc(Vector2.ZERO, 84, 0, TAU, 64, color, 3)
	for index in range(12):
		var direction := Vector2.from_angle(index * TAU / 12)
		draw_line(direction * 100, direction * 128, color, 3)
