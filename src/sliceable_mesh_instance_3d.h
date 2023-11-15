#ifndef SLICEABLE_MESH_INSTANCE_3D_H
#define SLICEABLE_MESH_INSTANCE_3D_H

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

namespace godot {

class SliceableMeshInstance3D : public MeshInstance3D {
	GDCLASS(SliceableMeshInstance3D, MeshInstance3D)

private:
	Ref<Material> m_inner_material;

protected:
	static void _bind_methods();

public:
	SliceableMeshInstance3D();
	~SliceableMeshInstance3D();

	void set_inner_material(const Ref<Material> p_inner_material);
	Ref<Material> get_inner_material() const;

	void slice_along_plane(const Plane p_plane);

private:
	Ref<ArrayMesh> slice_mesh_along_plane(const Ref<ArrayMesh> p_array_mesh, const Plane p_plane);
};

}

#endif
