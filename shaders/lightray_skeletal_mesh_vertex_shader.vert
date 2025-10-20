#version 460

layout(set = 0, binding = 0) uniform cvp_t
{
    mat4 view;
    mat4 projection;
}cvp;

layout(set = 0, binding = 2, std430) buffer bone_buffer_t
{
    mat4 bone_matrices[];
}bone_buffer;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in uvec4 in_bone_indices;
layout(location = 3) in uvec4 in_weights;
layout(location = 4) in mat4 in_instance_model;
layout(location = 8) in uint in_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance;

layout(location = 0) out vec3 frag_normal;
layout(location = 1) out vec3 frag_position;
layout(location = 2) out vec3 frag_color;

float unpack_u8_to_f32(uint p)
{
    return clamp(float(p) / 255.0, 0.0, 1.0);
}

void main()
{
    const uint computed_bone_transform_matrix_buffer_offset_with_respect_to_instance = in_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance;
    const uint bone_buffer_offset_x = in_bone_indices.x + computed_bone_transform_matrix_buffer_offset_with_respect_to_instance;
    const uint bone_buffer_offset_y = in_bone_indices.y + computed_bone_transform_matrix_buffer_offset_with_respect_to_instance;
    const uint bone_buffer_offset_z = in_bone_indices.z + computed_bone_transform_matrix_buffer_offset_with_respect_to_instance;
    const uint bone_buffer_offset_w = in_bone_indices.w + computed_bone_transform_matrix_buffer_offset_with_respect_to_instance;

   vec4 unpacked_weights = vec4
    (
        unpack_u8_to_f32(in_weights.x),
        unpack_u8_to_f32(in_weights.y),
        unpack_u8_to_f32(in_weights.z),
        unpack_u8_to_f32(in_weights.w)
    );

     unpacked_weights /= max(dot(unpacked_weights, vec4(1.0)), 0.0001);

     vec4 skinned_position =
    (bone_buffer.bone_matrices[bone_buffer_offset_x] * vec4(in_position, 1.0)) * unpacked_weights.x +
    (bone_buffer.bone_matrices[bone_buffer_offset_y] * vec4(in_position, 1.0)) * unpacked_weights.y +
    (bone_buffer.bone_matrices[bone_buffer_offset_z] * vec4(in_position, 1.0)) * unpacked_weights.z +
    (bone_buffer.bone_matrices[bone_buffer_offset_w] * vec4(in_position, 1.0)) * unpacked_weights.w;

     frag_color = vec3(0.0, 0.1, 0.0);

    mat3 normal_matrix = mat3(transpose(inverse(in_instance_model)));
    frag_normal = normalize(normal_matrix * in_normal);
    frag_position = vec3(in_instance_model * skinned_position);

    gl_Position = cvp.projection * cvp.view * in_instance_model * skinned_position;
}

//mat4 skin_matrix = mat4(1.0);
     //skin_matrix = bone_buffer.bone_matrices[bone_buffer_offset_x] * unpacked_weights.x +
     //bone_buffer.bone_matrices[bone_buffer_offset_y] * unpacked_weights.y +
     //bone_buffer.bone_matrices[bone_buffer_offset_z] * unpacked_weights.z +
     //bone_buffer.bone_matrices[bone_buffer_offset_w] * unpacked_weights.w;

   // const vec4 skinned_position = skin_matrix * vec4(in_position, 1.0);