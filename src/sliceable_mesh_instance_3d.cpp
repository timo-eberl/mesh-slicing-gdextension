#include "sliceable_mesh_instance_3d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SliceableMeshInstance3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_amplitude"), &SliceableMeshInstance3D::get_amplitude);
	ClassDB::bind_method(D_METHOD("set_amplitude", "p_amplitude"), &SliceableMeshInstance3D::set_amplitude);
	ClassDB::add_property("SliceableMeshInstance3D", PropertyInfo(Variant::FLOAT, "amplitude"), "set_amplitude", "get_amplitude");

	ClassDB::bind_method(D_METHOD("get_speed"), &SliceableMeshInstance3D::get_speed);
	ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &SliceableMeshInstance3D::set_speed);
	ClassDB::add_property("SliceableMeshInstance3D", PropertyInfo(Variant::FLOAT, "speed", PROPERTY_HINT_RANGE, "0,20,0.01"), "set_speed", "get_speed");

	ClassDB::bind_method(D_METHOD("slice_along_plane"), &SliceableMeshInstance3D::slice_along_plane);
}

SliceableMeshInstance3D::SliceableMeshInstance3D() : time_passed(0.0), amplitude(10.0), speed(1.0), m_cutting_plane() { }

SliceableMeshInstance3D::~SliceableMeshInstance3D() {
	// Add your cleanup here.
}

void SliceableMeshInstance3D::_process(double delta) {
	time_passed += speed * delta;

	Vector3 new_position = Vector3(
		amplitude + (amplitude * sin(time_passed * 2.0)),
		amplitude + (amplitude * cos(time_passed * 1.5)),
		0.0
	);

	// set_position(new_position);
}

void SliceableMeshInstance3D::set_amplitude(const double p_amplitude) {
	amplitude = p_amplitude;
}

double SliceableMeshInstance3D::get_amplitude() const {
	return amplitude;
}

void SliceableMeshInstance3D::set_speed(const double p_speed) {
	speed = p_speed;
}

double SliceableMeshInstance3D::get_speed() const {
	return speed;
}

void SliceableMeshInstance3D::slice_along_plane(const Plane p_plane) {
	UtilityFunctions::print("Cutting along plane ", p_plane);
}
