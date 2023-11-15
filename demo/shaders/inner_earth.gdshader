shader_type spatial;

uniform vec3 color_core : source_color = vec3(1.5);
uniform vec3 color_center : source_color = vec3(0.8,0.8,0);
uniform vec3 color_outside : source_color = vec3(1,0,0);

varying vec3 local_pos;

void vertex() {
	local_pos = VERTEX;
}

void fragment() {
	float dist = distance(vec3(0), local_pos) * 2.0;
	vec3 color = mix(color_center, color_outside, dist);
	color = mix(color_core, color, clamp(dist * 3.0, 0.0, 1.0));
	ALBEDO = color;
}
