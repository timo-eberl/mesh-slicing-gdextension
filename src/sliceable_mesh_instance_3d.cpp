#include "sliceable_mesh_instance_3d.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/plane.hpp>
#include <godot_cpp/classes/primitive_mesh.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/placeholder_mesh.hpp>

using namespace godot;

void SliceableMeshInstance3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_inner_material"), &SliceableMeshInstance3D::get_inner_material);
	ClassDB::bind_method(D_METHOD("set_inner_material", "p_inner_material"), &SliceableMeshInstance3D::set_inner_material);
	ClassDB::add_property("SliceableMeshInstance3D", PropertyInfo(Variant::OBJECT, "inner_material", PROPERTY_HINT_RESOURCE_TYPE, "BaseMaterial3D,ShaderMaterial"), "set_inner_material", "get_inner_material");

	ClassDB::bind_method(D_METHOD("slice_along_plane", "p_plane"), &SliceableMeshInstance3D::slice_along_plane);
	ClassDB::bind_method(D_METHOD("slice_along_plane_indexed", "p_plane"), &SliceableMeshInstance3D::slice_along_plane_indexed);
}

SliceableMeshInstance3D::SliceableMeshInstance3D() : m_inner_material() { }

SliceableMeshInstance3D::~SliceableMeshInstance3D() { }

void SliceableMeshInstance3D::set_inner_material(const Ref<Material> p_inner_material) {
	m_inner_material = p_inner_material;
}

Ref<Material> SliceableMeshInstance3D::get_inner_material() const {
	return m_inner_material;
}

void SliceableMeshInstance3D::slice_along_plane(const Plane p_plane) {
	this->slice_along_plane_p(p_plane, false);
}

void SliceableMeshInstance3D::slice_along_plane_indexed(const Plane p_plane) {
	this->slice_along_plane_p(p_plane, true);
}

void SliceableMeshInstance3D::slice_along_plane_p(const Plane p_plane, const bool indexed) {
	Ref<Mesh> mesh = this->get_mesh();

	if (auto primitive_mesh = Object::cast_to<PrimitiveMesh>(mesh.ptr())) {
		// mesh is of type PrimitiveMesh -> convert to ArrayMesh
		Ref<ArrayMesh> array_mesh { new ArrayMesh() };
		array_mesh->add_surface_from_arrays(
			Mesh::PRIMITIVE_TRIANGLES,
			primitive_mesh->get_mesh_arrays()
		);
		for (size_t i = 0; i < array_mesh->get_surface_count(); i++) {
			array_mesh->surface_set_material(i, mesh->surface_get_material(i));
		}

		this->set_mesh(this->slice_mesh_along_plane(array_mesh, p_plane, indexed));
	}
	else if (auto array_mesh = Object::cast_to<ArrayMesh>(mesh.ptr())) {
		this->set_mesh(this->slice_mesh_along_plane(array_mesh, p_plane, indexed));
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

void add_lid(
	const Ref<SurfaceTool> p_st, const Vector3 &p_lid_normal,
	const Vector3 &p_v0, const Vector3 &p_v1, const Vector3 &p_v2
) {
	p_st->set_normal(p_lid_normal); p_st->set_uv(Vector2(0,0)); p_st->add_vertex(p_v0);
	p_st->set_normal(p_lid_normal); p_st->set_uv(Vector2(0,0)); p_st->add_vertex(p_v1);
	p_st->set_normal(p_lid_normal); p_st->set_uv(Vector2(0,0)); p_st->add_vertex(p_v2);
}

Ref<ArrayMesh> SliceableMeshInstance3D::slice_mesh_along_plane(
	const Ref<ArrayMesh> p_array_mesh, const Plane p_plane, const bool indexed
) const {
	// transform the plane to object space
	Plane plane_os = this->get_global_transform().xform_inv(p_plane);

	Ref<ArrayMesh> new_mesh { new ArrayMesh };

	int32_t surface_count = p_array_mesh->get_surface_count();

	if (surface_count == 0) {
		WARN_PRINT("Mesh has no surface.");
		return new_mesh;
	}

	// this point is used for adding the "lid"
	// it will be set to the first created vertex (which is on the edge of the slice).
	Vector3 pos_on_lid;
	// keep track if pos_on_lid has been set. the first created vertex will set it.
	bool pos_on_lid_defined = false;

	// surface tool for the lid. all surfaces will add to it and only then will a new surface be created from it.
	Ref<SurfaceTool> st_lid { new SurfaceTool() };
	st_lid->begin(Mesh::PRIMITIVE_TRIANGLES);

	int32_t created_surface_count = 0;
	for (size_t i = 0; i < surface_count; i++) {
		Ref<MeshDataTool> mdt { new MeshDataTool() };
		mdt->create_from_surface(p_array_mesh, i);

		// surface tool for the sliced surface
		Ref<SurfaceTool> st_sliced { new SurfaceTool() };
		st_sliced->begin(Mesh::PRIMITIVE_TRIANGLES);

		// will add new mesh data to the surface tools
		slice_surface_along_plane(mdt, st_sliced, st_lid, pos_on_lid, pos_on_lid_defined, plane_os);

		// shrinks the vertex array by creating an index array (triangle list)
		// has a high performance penalty for big meshes
		if (indexed) st_sliced->index();
		// commit sliced surface as a new surface
		st_sliced->commit(new_mesh);
		// if a surface was added, set material
		if (new_mesh->get_surface_count() > created_surface_count) {
			new_mesh->surface_set_material(created_surface_count, mdt->get_material());
			++created_surface_count;
		}
	}

	// shrinks the vertex array by creating an index array (triangle list)
	// has a high performance penalty for big meshes
	if (indexed) st_lid->index();
	// commit lid as a new surface
	st_lid->commit(new_mesh);
	// if a surface was added, set material
	if (new_mesh->get_surface_count() > created_surface_count) {
		new_mesh->surface_set_material(created_surface_count, m_inner_material);
	}

	return new_mesh;
}

void SliceableMeshInstance3D::slice_surface_along_plane(
	const Ref<MeshDataTool> p_mdt, const Ref<SurfaceTool> p_st_sliced, const Ref<SurfaceTool> p_st_lid,
	Vector3 &p_pos_on_lid, bool &p_pos_on_lid_defined, const Plane p_plane_os
) const {

	Vector3 lid_normal = p_plane_os.normal;

	for (size_t face_idx = 0; face_idx < p_mdt->get_face_count(); ++face_idx) {
		// vertex data
		int32_t verts_indices [3];
		Vector3 verts [3];
		bool verts_are_above [3];
		Vector3 verts_normals [3];
		Vector2 verts_uvs [3];

		int32_t n_of_verts_above = 0;

		for (size_t i = 0; i < 3; ++i) {
			verts_indices[i] = p_mdt->get_face_vertex(face_idx, i);
			verts[i] = p_mdt->get_vertex(verts_indices[i]);
			verts_are_above[i] = p_plane_os.is_point_over(verts[i]);
			verts_normals[i] = p_mdt->get_vertex_normal(verts_indices[i]);
			verts_uvs[i] = p_mdt->get_vertex_uv(verts_indices[i]);

			if (verts_are_above[i]) ++n_of_verts_above;
		}

		switch (n_of_verts_above) {
			case 3: { // all vertices are above -> face is completely removed
				break;
			}
			case 0: { // all vertices are below -> face is kept
				for (size_t i = 0; i < 3; i++) {
					p_st_sliced->set_normal(verts_normals[i]); p_st_sliced->set_uv(verts_uvs[i]); p_st_sliced->add_vertex(verts[i]);
				}
				break;
			}
			case 2: { // two vertices are above and one below -> remove face, create one new face, create lid

				int32_t a0, a1, b; Vector3 n0, n1; // above (remove), below (keep), new (add)
				// ensure the winding order stays the same!
				if (!verts_are_above[0]) { b = 0; a0 = 1; a1 = 2; }
				else if (!verts_are_above[1]) { b = 1; a0 = 2; a1 = 0; }
				else if (!verts_are_above[2]) { b = 2; a0 = 0; a1 = 1; }

				p_plane_os.intersects_ray(verts[b], verts[a0] - verts[b], &n0); // find point on plane between b and a0
				p_plane_os.intersects_ray(verts[b], verts[a1] - verts[b], &n1); // find point on plane between b and a1

				float n0_interpolator = Math::sqrt( verts[b].distance_squared_to(n0) / verts[b].distance_squared_to(verts[a0]) );
				float n1_interpolator = Math::sqrt( verts[b].distance_squared_to(n1) / verts[b].distance_squared_to(verts[a1]) );

				// interpolate uvs of new verts
				Vector2 n0_uv = verts_uvs[b].lerp(verts_uvs[a0], n0_interpolator);
				Vector2 n1_uv = verts_uvs[b].lerp(verts_uvs[a1], n1_interpolator);

				// interpolate normals of new verts (an slerp would be better, but lerp does just fine for this)
				Vector3 n0_normal = verts_normals[b].lerp(verts_normals[a0], n0_interpolator);
				Vector3 n1_normal = verts_normals[b].lerp(verts_normals[a1], n1_interpolator);

				// previous order was b -> a0 -> a1, so new order is b -> n0 -> n1
				p_st_sliced->set_normal(verts_normals[b]); p_st_sliced->set_uv(verts_uvs[b]); p_st_sliced->add_vertex(verts[b]);
				p_st_sliced->set_normal(n0_normal); p_st_sliced->set_uv(n0_uv); p_st_sliced->add_vertex(n0);
				p_st_sliced->set_normal(n1_normal); p_st_sliced->set_uv(n1_uv); p_st_sliced->add_vertex(n1);

				if (p_pos_on_lid_defined) { add_lid(p_st_lid, lid_normal, n0, p_pos_on_lid, n1); }
				else { p_pos_on_lid = n0; p_pos_on_lid_defined = true; } // no need to add a lid

				break;
			}
			case 1: { // one vertex are above and two below -> remove face, create two new faces, create lid

				int32_t a, b0, b1; Vector3 n0, n1; // above (remove), below (keep), new (add)
				// ensure the winding order stays the same!
				if (verts_are_above[0]) { a = 0; b0 = 1; b1 = 2; }
				else if (verts_are_above[1]) { a = 1; b0 = 2; b1 = 0; }
				else if (verts_are_above[2]) { a = 2; b0 = 0; b1 = 1; }

				p_plane_os.intersects_ray(verts[a], verts[b0] - verts[a], &n0); // find point on plane between a and b0
				p_plane_os.intersects_ray(verts[a], verts[b1] - verts[a], &n1); // find point on plane between a and b1

				float n0_interpolator = Math::sqrt( verts[a].distance_squared_to(n0) / verts[a].distance_squared_to(verts[b0]) );
				float n1_interpolator = Math::sqrt( verts[a].distance_squared_to(n1) / verts[a].distance_squared_to(verts[b1]) );

				// interpolate uvs of new verts
				Vector2 n0_uv = verts_uvs[a].lerp(verts_uvs[b0], n0_interpolator);
				Vector2 n1_uv = verts_uvs[a].lerp(verts_uvs[b1], n1_interpolator);

				// interpolate normals of new verts (an slerp would be better, but lerp does just fine for this)
				Vector3 n0_normal = verts_normals[a].lerp(verts_normals[b0], n0_interpolator);
				Vector3 n1_normal = verts_normals[a].lerp(verts_normals[b1], n1_interpolator);

				// previous order was b1 -> a -> b0, so the first triangle is b1 -> n1 -> n0
				p_st_sliced->set_normal(verts_normals[b1]); p_st_sliced->set_uv(verts_uvs[b1]); p_st_sliced->add_vertex(verts[b1]);
				p_st_sliced->set_normal(n1_normal); p_st_sliced->set_uv(n1_uv); p_st_sliced->add_vertex(n1);
				p_st_sliced->set_normal(n0_normal); p_st_sliced->set_uv(n0_uv); p_st_sliced->add_vertex(n0);

				// the second triangle is b1 -> n0 -> b0
				p_st_sliced->set_normal(verts_normals[b1]); p_st_sliced->set_uv(verts_uvs[b1]); p_st_sliced->add_vertex(verts[b1]);
				p_st_sliced->set_normal(n0_normal); p_st_sliced->set_uv(n0_uv); p_st_sliced->add_vertex(n0);
				p_st_sliced->set_normal(verts_normals[b0]); p_st_sliced->set_uv(verts_uvs[b0]); p_st_sliced->add_vertex(verts[b0]);

				if (p_pos_on_lid_defined) { add_lid(p_st_lid, lid_normal, n1, p_pos_on_lid, n0); }
				else { p_pos_on_lid = n0; p_pos_on_lid_defined = true; } // no need to add a lid

				break;
			}
		}
	}
}
