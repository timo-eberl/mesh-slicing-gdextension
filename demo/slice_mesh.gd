extends SliceableMeshInstance3D

@export var plane_node : Node3D


func _ready():
	self.slice_along_plane(Plane(-plane_node.global_transform.basis.z, plane_node.global_position))
