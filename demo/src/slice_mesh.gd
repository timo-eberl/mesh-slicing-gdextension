extends SliceableMeshInstance3D

@export var plane_node : Node3D

@onready var original_mesh : Mesh = self.mesh
@onready var original_material : Material = self.get_surface_override_material(0)


func _process(_delta):
	# do this after every other _process to avoid z-fighting when rotating the plane
	call_deferred("slice")

func slice():
	self.mesh = original_mesh
	self.set_surface_override_material(0, original_material)
	
#	var time_start = Time.get_ticks_msec()
	self.slice_along_plane(Plane(-plane_node.global_transform.basis.z, plane_node.global_position))
#	print("Slicing ", self.get_parent().name, " took ", str(Time.get_ticks_msec() - time_start), "ms")
