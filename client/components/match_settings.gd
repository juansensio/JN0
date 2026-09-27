class_name MatchSettings
extends PanelContainer
signal start_requested
signal save_requested
signal load_requested
@onready var seed_input: LineEdit = $Settings/Seed
@onready var bot_select: OptionButton = $Settings/Bot
@onready var seat_select: OptionButton = $Settings/Seat

func _ready() -> void:
	bot_select.add_item("HeuristicBot")
	bot_select.add_item("RandomBot")
	seat_select.add_item("You: Player 1")
	seat_select.add_item("You: Player 2")
	$Settings/Start.pressed.connect(func(): start_requested.emit())
	$Settings/Save.pressed.connect(func(): save_requested.emit())
	$Settings/Load.pressed.connect(func(): load_requested.emit())
