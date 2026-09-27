extends SceneTree

func _initialize() -> void:
	call_deferred("render_scene")

func capture(path: String) -> void:
	await process_frame
	await RenderingServer.frame_post_draw
	if root.get_texture().get_image().save_png(path) != OK:
		quit(1)

func render_scene() -> void:
	var scene = load("res://main.tscn").instantiate()
	root.add_child(scene)
	scene.set_process(false)
	scene.seed_input.text = "0"
	scene.restart()
	var path := OS.get_cmdline_user_args()[0]
	await capture(path + "-turn.png")
	var captured_defense := false
	for step in range(200):
		var o: Dictionary = scene.game.observe(scene.human)
		if o.phase == "terminal":
			break
		if o.acting_player == scene.human:
			if o.phase == "awaiting_defense" and not captured_defense:
				await capture(path + "-defense.png")
				captured_defense = true
			scene.action_buttons[0].pressed.emit()
		else:
			scene.advance_bot()
	await capture(path + "-result.png")
	quit()
