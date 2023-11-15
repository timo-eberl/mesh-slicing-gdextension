extends SliceableMeshInstance3D

@export var plane_node : Node3D

@onready var original_mesh : Mesh = self.mesh


func _process(_delta):
	# do this after every other _process to avoid z-fighting when rotating the plane
	call_deferred("slice")

func slice():
	self.mesh = original_mesh
	
#	var time_start = Time.get_ticks_msec()
	self.slice_along_plane(Plane(-plane_node.global_transform.basis.z, plane_node.global_position))
#	print("Slicing ", self.get_parent().name, " took ", str(Time.get_ticks_msec() - time_start), "ms")
