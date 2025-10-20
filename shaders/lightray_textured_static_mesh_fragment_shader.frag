#version 460

layout(location = 0) in vec2 frag_uv;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_position;
layout(location = 3) in float frag_instance_layer_index;

layout(location = 0) out vec4 out_color;

layout(binding = 1) uniform sampler2DArray u_combined_sampler;

void main() 
{   
    out_color = texture(u_combined_sampler, vec3(frag_uv, frag_instance_layer_index));
}