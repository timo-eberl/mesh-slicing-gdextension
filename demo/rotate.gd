extends Node3D

@export var degree_per_second = 30


func _process(delta):
	self.global_transform = self.global_transform.rotated(
		Vector3(0,1,0), deg_to_rad(degree_per_second)*delta
	)
