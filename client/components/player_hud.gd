class_name PlayerHUD
extends HBoxContainer
var name_label: Label
var lives_label: Label

func _ready() -> void:
	add_theme_constant_override("separation", 18)
	name_label = Label.new()
	name_label.add_theme_font_size_override("font_size", 18)
	add_child(name_label)
	lives_label = Label.new()
	lives_label.add_theme_font_size_override("font_size", 23)
	lives_label.add_theme_color_override("font_color", Color("f19982"))
	add_child(lives_label)

func present(title: String, lives: int, active: bool) -> void:
	name_label.text = title + ("  •" if active else "")
	lives_label.text = "♥ ".repeat(lives) + "♡ ".repeat(maxi(0, 3 - lives))
