#version 460

layout(binding = 0) uniform cvp_t
{
    mat4 view;
    mat4 projection;
}cvp;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 3) in mat4 in_instance_model;
layout(location = 7) in float in_instance_layer_index;

layout(location = 0) out vec2 frag_uv;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_position;
layout(location = 3) out float frag_instance_layer_index;

void main() 
{
    gl_Position = cvp.projection * cvp.view * in_instance_model * vec4(in_position, 1.0);

    frag_uv = in_uv;

    mat3 normal_matrix = mat3(transpose(inverse(in_instance_model)));
    frag_normal = normalize(normal_matrix * in_normal);
    frag_position = vec3(in_instance_model * vec4(in_position, 1.0f));
    frag_instance_layer_index = in_instance_layer_index;
}