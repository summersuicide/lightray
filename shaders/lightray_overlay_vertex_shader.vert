#version 450

layout(set = 0, binding = 0) uniform cvp_t
{
	mat4 view;
	mat4 projection;
}cvp;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in mat4 in_instance_model_matrix;
layout(location = 6) in vec2 in_uv_min;
layout(location = 7) in vec2 in_uv_max;
layout(location = 8) in vec3 in_color;

layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec3 frag_color;

void main()
{
	gl_Position = cvp.projection * cvp.view * in_instance_model_matrix * vec4(in_position, 1.0);

	frag_uv = mix(in_uv_min, in_uv_max, in_uv);
	frag_color = in_color;
}