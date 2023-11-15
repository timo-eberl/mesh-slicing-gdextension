extends Node3D

@onready var window_title = get_window().title


func _process(_delta):
	get_window().title = window_title + " | FPS: " + str(Engine.get_frames_per_second())
