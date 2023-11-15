#include "sliceable_mesh_instance_3d.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/classes/primitive_mesh.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/placeholder_mesh.hpp>
#include <godot_cpp/classes/mesh_data_tool.hpp>
#include <godot_cpp/classes/surface_tool.hpp>

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

	// create helper tools
	Ref<MeshDataTool> mdt { new MeshDataTool() };
	mdt->create_from_surface(p_array_mesh, 0);
	Ref<SurfaceTool> st { new SurfaceTool() };
	st->begin(Mesh::PRIMITIVE_TRIANGLES);

	int n_completely_removed = 0;
	int n_two_thirds_removed = 0;
	int n_one_third_removed = 0;
	int n_not_removed = 0;

	for (size_t i = 0; i < mdt->get_face_count(); ++i) {
		Vector3 verts [3] = {
			mdt->get_vertex(mdt->get_face_vertex(i, 0)),
			mdt->get_vertex(mdt->get_face_vertex(i, 1)),
			mdt->get_vertex(mdt->get_face_vertex(i, 2)),
		};
		bool verts_are_above [3] = {
			plane_os.is_point_over(verts[0]),
			plane_os.is_point_over(verts[1]),
			plane_os.is_point_over(verts[2]),
		};
		int n_of_verts_above = 0;
		for (size_t i = 0; i < 3; i++) {
			if (verts_are_above[i]) ++n_of_verts_above;
		}
		switch (n_of_verts_above) {
			case 3: { // all vertices are above -> face is completely removed
				++n_completely_removed;
				break;
			}
			case 2: { // two vertices are above and one below -> face is removed and one new face is created
				++n_two_thirds_removed;

				Vector3 a0, a1, b, n0, n1; // above (remove), below (keep), new (add)

				// ensure the order stays the same!
				if (!verts_are_above[0]) { b = verts[0]; a0 = verts[1]; a1 = verts[2]; }
				else if (!verts_are_above[1]) { b = verts[1]; a0 = verts[2]; a1 = verts[0]; }
				else if (!verts_are_above[2]) { b = verts[2]; a0 = verts[0]; a1 = verts[1]; }

				plane_os.intersects_ray(b, a0 - b, &n0); // find point on plane between b and a0
				plane_os.intersects_ray(b, a1 - b, &n1); // find point on plane between b and a1

				st->add_vertex(b);
				st->add_vertex(n0);
				st->add_vertex(n1);

				break;
			}
			case 1: { // one vertex are above and two below -> face is removed and two new faces are created
				++n_one_third_removed;

				Vector3 a, b0, b1, n0, n1; // above (remove), below (keep), new (add)

				// ensure the order stays the same!
				if (verts_are_above[0]) { a = verts[0]; b0 = verts[1]; b1 = verts[2]; }
				else if (verts_are_above[1]) { a = verts[1]; b0 = verts[2]; b1 = verts[0]; }
				else if (verts_are_above[2]) { a = verts[2]; b0 = verts[0]; b1 = verts[1]; }

				plane_os.intersects_ray(a, b0 - a, &n0); // find point on plane between a and b0
				plane_os.intersects_ray(a, b1 - a, &n1); // find point on plane between a and b1

				st->add_vertex(b1);
				st->add_vertex(n1);
				st->add_vertex(n0);

				st->add_vertex(b1);
				st->add_vertex(n0);
				st->add_vertex(b0);

				break;
			}
			case 0: { // all vertices are below -> face is kept
				++n_not_removed;

				for (size_t i = 0; i < 3; i++) {
					st->add_vertex(verts[i]);
				}

				break;
			}
		}
	}

	// UtilityFunctions::print("mdt->get_vertex_count(): ", mdt->get_vertex_count());
	
	// Ref<MeshDataTool> mdt_new { new MeshDataTool() };
	// mdt_new->create_from_surface(st->commit(), 0);
	// UtilityFunctions::print("mdt_new->get_vertex_count(): ", mdt_new->get_vertex_count());

	// shrinks the vertex array by creating an index array
	st->index();

	// Ref<MeshDataTool> mdt_new_optimized { new MeshDataTool() };
	// mdt_new_optimized->create_from_surface(st->commit(), 0);
	// UtilityFunctions::print("mdt_new_optimized->get_vertex_count(): ", mdt_new_optimized->get_vertex_count());

	return st->commit();
}
