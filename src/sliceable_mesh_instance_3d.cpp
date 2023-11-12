#include "sliceable_mesh_instance_3d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SliceableMeshInstance3D::_bind_methods() {
}

SliceableMeshInstance3D::SliceableMeshInstance3D() {
	// Initialize any variables here.
	time_passed = 0.0;
}

SliceableMeshInstance3D::~SliceableMeshInstance3D() {
	// Add your cleanup here.
}

void SliceableMeshInstance3D::_process(double delta) {
	time_passed += delta;

	Vector3 new_position = Vector3(10.0 + (10.0 * sin(time_passed * 2.0)), 10.0 + (10.0 * cos(time_passed * 1.5)), 0.0);

	set_position(new_position);
}
