#version 450

layout (binding = 1) uniform sampler2DArray texture_samplers;
	   
layout (location = 0) in vec3 fragment_colour;
layout (location = 1) in vec3 fragment_UVW;
	   
layout (location = 0) out vec4 output_colour;


void main() {
	output_colour = texture(texture_samplers, fragment_UVW);
}