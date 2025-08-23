#version 450

layout(binding = 0) uniform cvp_t
{
    mat4 view;
    mat4 projection;
}cvp;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texture_coordinates;
layout(location = 3) in float in_alpha;

layout(location = 4) in mat4 in_instance_model;

layout(location = 0) out vec2 frag_texture_coordinates;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec3 frag_position;

void main() 
{
    gl_Position = cvp.projection * cvp.view * in_instance_model * vec4(in_position, 1.0);

    frag_texture_coordinates = in_texture_coordinates;

    mat3 normal_matrix = mat3(transpose(inverse(in_instance_model)));
    frag_normal = normalize(normal_matrix * in_normal);
    frag_position = vec3(in_instance_model * vec4(in_position, 1.0f));
}