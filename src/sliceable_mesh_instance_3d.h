#ifndef SLICEABLE_MESH_INSTANCE_3D_H
#define SLICEABLE_MESH_INSTANCE_3D_H

#include <godot_cpp/classes/mesh_instance3d.hpp>

namespace godot {

class SliceableMeshInstance3D : public MeshInstance3D {
	GDCLASS(SliceableMeshInstance3D, MeshInstance3D)

private:
	double time_passed;

protected:
	static void _bind_methods();

public:
	SliceableMeshInstance3D();
	~SliceableMeshInstance3D();

	void _process(double delta);
};

}

#endif
