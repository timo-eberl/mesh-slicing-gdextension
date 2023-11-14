extends SliceableMeshInstance3D

@export var plane_node : Node3D
@onready var original_mesh : Mesh = self.mesh

func _process(_delta):
	self.mesh = original_mesh
	self.slice_along_plane(Plane(-plane_node.global_transform.basis.z, plane_node.global_position))
