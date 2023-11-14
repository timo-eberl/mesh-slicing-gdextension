#include "sliceable_mesh_instance_3d.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/classes/primitive_mesh.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/placeholder_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>

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
	Ref<Mesh> mesh = this->get_mesh();

	if (auto primitive_mesh = Object::cast_to<PrimitiveMesh>(mesh.ptr())) {
		UtilityFunctions::print("mesh is of type PrimitiveMesh. Converting to ArrayMesh.");

		Ref<ArrayMesh> array_mesh { new ArrayMesh() };
		array_mesh->add_surface_from_arrays(
			Mesh::PRIMITIVE_TRIANGLES,
			primitive_mesh->get_mesh_arrays()
		);

		this->set_mesh(this->slice_mesh_along_plane(array_mesh, p_plane));
	}
	else if (auto array_mesh = Object::cast_to<ArrayMesh>(mesh.ptr())) {
		this->set_mesh(this->slice_mesh_along_plane(array_mesh, p_plane));
	}
	else if (Object::cast_to<ImmediateMesh>(mesh.ptr()) != nullptr) {
		WARN_PRINT("Cannot slice ImmediateMesh.");
		return;
	}
	else if (Object::cast_to<PlaceholderMesh>(mesh.ptr()) != nullptr) {
		WARN_PRINT("Cannot slice PlaceholderMesh.");
		return;
	}
	else {
		WARN_PRINT("Cannot slice unknown Mesh Type.");
		return;
	}
}

Ref<ArrayMesh> SliceableMeshInstance3D::slice_mesh_along_plane(const Ref<ArrayMesh> p_array_mesh, const Plane p_plane) {
	// transform the plane to object space
	Plane plane_os = this->get_global_transform().xform_inv(p_plane);

	Ref<MeshDataTool> mdt { new MeshDataTool() };
	mdt->create_from_surface(p_array_mesh, 0);

	Vector3 approx_center_of_cut = Vector3(0,0,0) + plane_os.normal * plane_os.d;

	int side_a_ctr = 0;
	int side_b_ctr = 0;
	for (int i = 0; i < mdt->get_vertex_count(); ++i) {
		Vector3 vertex = mdt->get_vertex(i);
		if (plane_os.is_point_over(vertex)) {
			// cut
			++side_a_ctr;
			// quick hack: collapse geometry to the approximated center of the cut
			mdt->set_vertex(i, approx_center_of_cut);
		}
		else {
			++side_b_ctr;
		}
	}

	UtilityFunctions::print("Side A contains ", side_a_ctr, " vertices which will be cut");
	UtilityFunctions::print("Side B contains ", side_b_ctr, " vertices");

	Ref<ArrayMesh> new_mesh { new ArrayMesh() };
	mdt->commit_to_surface(new_mesh);
	return new_mesh;
}
