extends Node

func _ready():
	print("[Godot] Starting dummy scene")
	var game = JanusGame.new()
	game.name = "Game"
	add_child(game)
	print("[Godot] Native game node attached")
