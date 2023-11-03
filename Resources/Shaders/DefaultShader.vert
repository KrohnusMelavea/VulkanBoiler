#version 450

layout (binding = 0) uniform UBO {
	mat4 view;
	mat4 projection;
} ubo;

layout (location = 0) in vec3 vertex_XYZ;
layout (location = 1) in vec3 vertex_colour;
layout (location = 2) in vec2 vertex_UV;
	   
layout (location = 3) in vec3 instanced_translation;
layout (location = 4) in vec3 instanced_rotation;
layout (location = 5) in vec3 instanced_scale;
layout (location = 6) in uint instanced_texture_index;
	   
layout (location = 0) out vec3 fragment_colour;
layout (location = 1) out vec3 fragment_UVW;

mat4 invert_matrix(const mat4 m) {
	return mat4(
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[1][2], m[3][1],
		m[2][1], m[0][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]
	);
}
mat4 scale_matrix(const vec3 v) {
	return mat4(
		v.x, 0.0, 0.0, 0.0,
		0.0, v.y, 0.0, 0.0,
		0.0, 0.0, v.z, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
}
mat4 rotation_matrix_x(const float a) {
	const float c_a = cos(a); 
	const float s_a = sin(a);
	return mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, c_a,-s_a, 0.0,
		0.0, s_a, c_a, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
}
mat4 rotation_matrix_y(const float a) {
	const float c_a = cos(a);
	const float s_a = sin(a);
	return mat4(
		c_a, s_a, 0.0, 0.0,
	   -s_a, c_a, 0.0, 0.0,
	    0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	);
}
mat4 rotation_matrix_z(const float a) {
	const float c_a = cos(a);
	const float s_a = sin(a);
	return mat4(
		c_a, 0.0, s_a, 0.0,
		0.0, 1.0, 0.0, 0.0,
	   -s_a, 0.0, c_a, 0.0,
	    0.0, 0.0, 0.0, 1.0
	);
}
mat4 translation_matrix(const vec3 v) {
	return mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		v.x, v.y, v.z, 1.0
	);
}

vec3 Rxyz(const vec3 V, const vec3 A) {
	const float cx = cos(A.x), cy = cos(A.y), cz = cos(A.z);
	const float sx = sin(A.x), sy = sin(A.y), sz = sin(A.z);
	const float x = V.x, y = V.y, z = V.z;

	return vec3(
		(x*cy + (y*sx + z*cx) * sy) * cz - (y*cx - z*sx) * sz,
		(x*cy + (y*sx + z*cx) * sy) * sz + (y*cx - z*sx) * cz,
		(y*sx + z*cx) * cy - x*sy
	);
}
vec4 Rxyzw(const vec4 V, const vec3 A) {
	const float cx = cos(A.x), cy = cos(A.y), cz = cos(A.z);
	const float sx = sin(A.x), sy = sin(A.y), sz = sin(A.z);
	const float x = V.x, y = V.y, z = V.z;

	return vec4(
		(x*cy + (y*sx + z*cx) * sy) * cz - (y*cx - z*sx) * sz,
		(x*cy + (y*sx + z*cx) * sy) * sz + (y*cx - z*sx) * cz,
		(y*sx + z*cx) * cy - x*sy,
		1.0
	);
}

void main() {
	gl_Position = ubo.projection * ubo.view * translation_matrix(instanced_translation) * Rxyzw(scale_matrix(instanced_scale) * vec4(vertex_XYZ, 1.0), instanced_rotation);
	fragment_colour = vertex_colour;
	fragment_UVW = vec3(vertex_UV, instanced_texture_index);
}
