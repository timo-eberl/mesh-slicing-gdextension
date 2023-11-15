shader_type spatial;

uniform sampler2D tex_albedo;

void vertex() {
	// convert local coordinates to uv
	UV.x = VERTEX.x + 0.5;
	UV.y = -VERTEX.y + 0.5;
}

void fragment() {
	ALBEDO = texture(tex_albedo, UV).xyz;
}
