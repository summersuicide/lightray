#include "lightray_core.h"

lightray_result lightray_load_shader_byte_code(cstring_literal* path, i8* buffer, u64* buffer_size_in_bytes, u64 byte_code_size_limit)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) { return LIGHTRAY_RESULT_FAILED_TO_OPEN_COMPILED_SHADER_BYTE_CODE; }
	u64 file_size = (u64)file.tellg();
	if (file_size == 0 || file_size > byte_code_size_limit) { return LIGHTRAY_RESULT_COMPILED_SHADER_BYTE_CODE_SIZE_LIMIT_HAS_BEEN_EXCEEDED; }
	(*buffer_size_in_bytes) = file_size;
	file.seekg(0);
	file.read(buffer, file_size);
	if (file.fail()) { return LIGHTRAY_RESULT_FAILED_TO_LOAD_COMPILED_SHADER_BYTE_CODE; }
	file.close();

	return LIGHTRAY_RESULT_SUCCESS;
}

u64 lightray_get_shader_byte_code_size(cstring_literal* path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) { return UINT64_MAX; }
	u64 file_size = (u64)file.tellg();
	if (file.fail()) { return UINT64_MAX; }
	file.close();

	return file_size;
}

void lightray_hide_cursor(GLFWwindow* window)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void lightray_unhide_cursor(GLFWwindow* window)
{
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

bool lightray_should_tick(GLFWwindow* window)
{
	return !glfwWindowShouldClose(window);
}

bool lightray_key_pressed(GLFWwindow* window, lightray_key_binding key, u8* key_tick_data)
{
	if (LIGHTRAY_KEY_BOUND(key))
	{
		if (glfwGetKey(window, SUNDER_CAST2(i32)key) == GLFW_PRESS)
		{
			if ((*key_tick_data) & 1U << LIGHTRAY_BITS_KEY_CAN_BE_PRESSED_BIT)
			{
				(*key_tick_data) &= ~(1U << LIGHTRAY_BITS_KEY_CAN_BE_PRESSED_BIT); // can_press = false;
				return true;
			}
		}

		else
		{
			(*key_tick_data) |= 1U << LIGHTRAY_BITS_KEY_CAN_BE_PRESSED_BIT; // can_press = true;
		}
	}

	return false;
}

bool lightray_key_down(GLFWwindow* window, lightray_key_binding key)
{
	if (LIGHTRAY_KEY_BOUND(key))
	{
		return glfwGetKey(window, SUNDER_CAST2(i32)key) == GLFW_PRESS;
	}

	return false;

}

bool lightray_mouse_button_pressed(GLFWwindow* window, lightray_key_binding mouse_button, u8* key_tick_data)
{
	if (LIGHTRAY_KEY_BOUND(mouse_button))
	{
		if (glfwGetMouseButton(window, SUNDER_CAST2(i32)mouse_button) == GLFW_PRESS)
		{
			if ((*key_tick_data) & 1U << LIGHTRAY_BITS_KEY_CAN_BE_PRESSED_BIT)
			{
				(*key_tick_data) &= ~(1U << LIGHTRAY_BITS_KEY_CAN_BE_PRESSED_BIT); // can_press = false;
				return true;
			}
		}

		else
		{
			(*key_tick_data) |= 1U << LIGHTRAY_BITS_KEY_CAN_BE_PRESSED_BIT; // can_press = true;
		}
	}

	return false;
}

glm::mat4 lightray_construct_projection_matrix(f32 desired_fov, f32 aspect_ratio, f32 near_plane, f32 far_plane)
{
	glm::mat4 projection{};
	const f32 desired_fov_clamped = glm::clamp(desired_fov, 40.0f, 170.0f);

	projection = glm::perspectiveRH_ZO(glm::radians(desired_fov_clamped), aspect_ratio, near_plane, far_plane);
	projection[1][1] *= -1;

	return projection;
}

f32 lightray_compute_aspect_ratio(f32 width, f32 height)
{
	return width / height;
}

glm::vec3 lightray_to_world_forward_from_euler(const glm::vec3& euler_radians, const glm::vec3& local_forward)
{
	const glm::quat rotation = glm::quat(euler_radians);
	const glm::vec3 world_forward = rotation * local_forward;

	return glm::normalize(world_forward);
}

glm::vec3 lightray_to_world_forward_from_model(const glm::mat4& model_matrix, const glm::vec3& local_forward)
{
	const glm::vec3 normalized_world_forward = glm::normalize(glm::vec3(model_matrix * glm::vec4(local_forward, 0.0f)));

	return normalized_world_forward;
}

glm::vec3 lightray_get_camera_forward(const glm::mat4& camera_view_matrix)
{
	return -glm::vec3(camera_view_matrix[0][2], camera_view_matrix[1][2], camera_view_matrix[2][2]);
}

glm::vec3 lightray_get_camera_right(const glm::mat4& camera_view_matrix)
{
	return glm::vec3(camera_view_matrix[0][0], camera_view_matrix[1][0], camera_view_matrix[2][0]);
}

bool lightray_ray_triangle_intersect(const lightray_ray_t* ray, const glm::vec3* triangle_vertices, f32* out_t, f32* out_u, f32* out_v)
{
	const f64 EPSILON = 1e-8;

	glm::vec3 AB = triangle_vertices[1] - triangle_vertices[0];
	glm::vec3 AC = triangle_vertices[2] - triangle_vertices[0];
	//glm::vec3 N = cross(AB, AC); // triangle normal, not gonna use for now

	glm::vec3 AO = ray->origin - triangle_vertices[0];
	glm::vec3 P = glm::cross(ray->direction, AC);
	f64 det = dot(P, AB);

	// If determinant is near zero, ray lies in plane or is parallel
	if (fabs(det) < EPSILON)
	{
		return false;
	}

	f64 invDet = 1.0 / det;

	f64 u = dot(P, AO) * invDet;

	if (u < 0.0 || u > 1.0)
	{
		return false;
	}

	glm::vec3 Q = cross(AO, AB);
	f64 v = glm::dot(Q, ray->direction) * invDet;

	if (v < 0.0 || u + v > 1.0)
	{
		return false;
	}

	f64 t = dot(Q, AC) * invDet;

	if (t < EPSILON) // intersection behind origin
	{
		return false;
	}

	*out_t = (f32)t;
	*out_u = (f32)u;
	*out_v = (f32)v;

	if (t < 0.0f || t > ray->distance)
	{
		return false;
	}

	return true;
}

/*
lightray_buffer_index_query_result_t lightray_query_buffer_index(const u32* buffer, u32 starting_index, u32 ending_index, u32 val_at_index, bool reverse_logic)
{
	lightray_buffer_index_query_result_t res;

	if (reverse_logic)
	{
		for (u32 i = starting_index; i < ending_index; i++)
		{
			if (buffer[i] != val_at_index)
			{
				res.value_at_index = buffer[i];
				res.return_index = i;

				return res;
			}
		}

		return res;
	}

	else
	{
		for (u32 i = starting_index; i < ending_index; i++)
		{
			if (buffer[i] == val_at_index)
			{
				res.value_at_index = buffer[i];
				res.return_index = i;

				return res;
			}
		}

		return res;
	}

	return res;
}
*/

bool lightray_aabbs_intersect(const glm::vec3& position_a, const glm::vec3& scale_a, const glm::vec3& position_b, const glm::vec3& scale_b)
{
	const glm::vec3 converted_size_a = scale_a / 10.f;
	const glm::vec3 converted_size_b = scale_b / 10.f;

	const glm::vec3 min_a = position_a - converted_size_a;
	const glm::vec3 max_a = position_a + converted_size_a;

	const glm::vec3 min_b = position_b - converted_size_b;
	const glm::vec3 max_b = position_b + converted_size_b;

	return ((min_a.x <= max_b.x) && (max_a.x >= min_b.x) && (min_a.y <= max_b.y) && (max_a.y >= min_b.y) && (min_a.z <= max_b.z) && (max_a.z >= min_b.z));
}

void lightray_get_raw_vertex_positions(u32 index_buffer_offset, u32 index_count, glm::vec3* buffer, const lightray_vertex_t* vertex_buffer, const u32* index_buffer)
{
	u32 vertex_iter = 0;

	for (u32 i = index_buffer_offset; i < index_buffer_offset + index_count; i++)
	{
		buffer[vertex_iter] = vertex_buffer[index_buffer[i]].position;
		vertex_iter++;
	}
}

void lightray_compute_projected_vertex_positions(const glm::vec3* raw_vertex_positions, glm::vec3* projected_vertex_positions, u64 vertex_count, const lightray_model_t* model)
{
	for (u64 i = 0; i < vertex_count; i++)
	{
		projected_vertex_positions[i] = glm::vec3(model->model * glm::vec4(raw_vertex_positions[i], 1.0f));
	}
}

glm::vec3 lightray_gjk_find_furthest_point(const glm::vec3* tri, u64 vertex_count, const glm::vec3& direction)
{
	glm::vec3 max_point{};
	f32 max_distance = -FLT_MAX;

	for (u64 i = 0; i < vertex_count; i++ )
	{
		f32 distance = glm::dot(tri[i], direction);

		if (distance > max_distance)
		{
			max_distance = distance;
			max_point = tri[i];
		}
	}

	return max_point;
}

glm::vec3 lightray_gjk_support(const glm::vec3* vertex_positions1, u64 vertex_count1, const glm::vec3* vertex_positions2, u64 vertex_count2, const glm::vec3& direction)
{
	return lightray_gjk_find_furthest_point(vertex_positions1, vertex_count1, direction) - lightray_gjk_find_furthest_point(vertex_positions2, vertex_count2, -direction);
}

bool lightray_gjk_same_direction(const glm::vec3& direction, glm::vec3& ao)
{
	return glm::dot(direction, ao) > 0;
}

bool lightray_gjk_line(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 ab = b - a;
	glm::vec3 ao = -a;

	if (lightray_gjk_same_direction(ab, ao))
	{
		*direction = glm::cross(glm::cross(ab, ao), ab);
	}

	else
	{
		simplex[0] = a;
		*simplex_size = 1;
		*direction = ao;
	}

	return false;
}

bool lightray_gjk_triangle(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 c = simplex[2];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ao = -a;

	glm::vec3 abc = glm::cross(ab, ac);

	if (lightray_gjk_same_direction(glm::cross(abc, ac), ao))
	{
		if (lightray_gjk_same_direction(ac, ao))
		{
			simplex[0] = a;
			simplex[1] = c;
			*simplex_size = 2;

			*direction = glm::cross(glm::cross(ac, ao), ac);
		}

		else
		{
			simplex[0] = a;
			simplex[1] = b;
			*simplex_size = 2;

			return lightray_gjk_line(simplex, simplex_size, direction);
		}
	}

	else
	{
		if (lightray_gjk_same_direction(glm::cross(ab, abc), ao))
		{
			simplex[0] = a;
			simplex[1] = b;
			*simplex_size = 2;

			return lightray_gjk_line(simplex, simplex_size, direction);
		}

		else
		{
			if (lightray_gjk_same_direction(abc, ao))
			{
				*direction = abc;
			}

			else
			{
				simplex[0] = a;
				simplex[1] = c;
				simplex[2] = b;
				*simplex_size = 3;
				*direction = -abc;
			}
		}
	}

	return false;
}

bool lightray_gjk_tetrahedron(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
{
	glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 c = simplex[2];
	glm::vec3 d = simplex[3];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ad = d - a;
	glm::vec3 ao = -a;

	glm::vec3 abc = glm::cross(ab, ac);
	glm::vec3 acd = glm::cross(ac, ad);
	glm::vec3 adb = glm::cross(ad, ab);

	if (lightray_gjk_same_direction(abc, ao))
	{
		simplex[0] = a;
		simplex[1] = b;
		simplex[2] = c;

		*simplex_size = 3;

		return lightray_gjk_triangle(simplex, simplex_size, direction);
	}

	if (lightray_gjk_same_direction(acd, ao))
	{
		simplex[0] = a;
		simplex[1] = c;
		simplex[2] = d;

		*simplex_size = 3;

		return lightray_gjk_triangle(simplex, simplex_size, direction);
	}

	if (lightray_gjk_same_direction(adb, ao))
	{
		simplex[0] = a;
		simplex[1] = d;
		simplex[2] = b;

		*simplex_size = 3;

		return lightray_gjk_triangle(simplex, simplex_size, direction);
	}

	return true;
}

bool lightray_gjk_next_simplex(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
{
	switch (*simplex_size)
	{
		case 2: return lightray_gjk_line(simplex, simplex_size, direction);
			break;
		case 3: return lightray_gjk_triangle(simplex, simplex_size, direction);
			break;
		case 4: return lightray_gjk_tetrahedron(simplex, simplex_size, direction);
			break;
	}

	return false;
}

bool lightray_gjk_intersect(const glm::vec3* vertex_positions1, u64 vertex_count1, const glm::vec3* vertex_positions2, u64 vertex_count2)
{
	glm::vec3 support = lightray_gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, glm::vec3(1.0f, 0.0f, 0.0f));

	glm::vec3 simplex[4]{};
	u32 simplex_size = 1;
	simplex[0] = support;

	glm::vec3 direction = -support;

	while (true)
	{
		support = lightray_gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, direction);

		if (glm::dot(support, direction) <= 0)
		{
			return false;
		}
			
		simplex[3] = simplex[2];
		simplex[2] = simplex[1];
		simplex[1] = simplex[0];
		simplex[0] = support;

		simplex_size = glm::min(simplex_size + 1, 4U);

		if (lightray_gjk_next_simplex(simplex, &simplex_size, &direction))
		{
			return true;
		}
	};
}

glm::mat4 lightray_assimp_to_glm_mat4(const aiMatrix4x4& mat)
{
	return glm::mat4(
	mat.a1, mat.b1, mat.c1, mat.d1,
	mat.a2, mat.b2, mat.c2, mat.d2,
	mat.a3, mat.b3, mat.c3, mat.d3,
	mat.a4, mat.b4, mat.c4, mat.d4
	);

	//glm::mat4 m{};

	//for (u32 y = 0; y < 4; y++)
	//{
	//	for (u32 x = 0; x < 4; x++)
	//	{
	//		m[x][y] = mat[y][x];
	//	}
	//}

	//return m;
}

u32 lightray_assimp_get_mesh_index_count(const aiMesh* mesh)
{
	u32 index_count_iter = 0;

	for (u32 j = 0; j < mesh->mNumFaces; j++)
	{
		index_count_iter += mesh->mFaces[j].mNumIndices;
	}

	return index_count_iter;
}

u32 lightray_assimp_get_mesh_index_count_unsafe(const aiMesh* mesh)
{
	return mesh->mNumFaces * 3;
}

/*
void lightray_compute_interpolated_bone_positions(aiNodeAnim* animation_channel_buffer, const glm::mat4& parent_transform, glm::mat4* bone_offset_matrix_buffer, f32 delta_time, f64 duration)
{
	float ticksPerSecond = 30.0f;
	float timeInTicks = delta_time * ticksPerSecond;
	float animation_time = (f32)fmod(timeInTicks, duration);

	glm::vec3 res_pos_buffer[3]{};
	glm::quat res_rot_buffer[3]{};
	glm::vec3 res_scale_buffer[3]{};
		
	glm::mat4 res_matrix_buffer[3]{};

	bool should_proceed_pos = true;
	bool should_proceed_rot = true;
	bool should_proceed_scale = true;

	glm::mat4 final_bone_position_buffer[3]{};
		
	for (u32 i = 0; i < 3; i++)
		{
			// pos
			if (animation_channel_buffer[i].mNumPositionKeys == 1) {
				aiVector3D pos = animation_channel_buffer[i].mPositionKeys[0].mValue;
				res_pos_buffer[i] = glm::vec3(pos.x, pos.y, pos.z);
				should_proceed_pos = false;
			}

			if (should_proceed_pos)
			{
				u32 posIndex = 0;
				for (u32 i = 0; i < animation_channel_buffer[i].mNumPositionKeys - 1; i++)
				{
					if (animation_time < (float)animation_channel_buffer[i].mPositionKeys[i + 1].mTime) 
					{
						posIndex = i;
						break;
					}
				}

				u32 nextPosIndex = posIndex + 1;

				float t1 = (float)animation_channel_buffer[i].mPositionKeys[posIndex].mTime;
				float t2 = (float)animation_channel_buffer[i].mPositionKeys[nextPosIndex].mTime;
				float factor = (animation_time - t1) / (t2 - t1);

				aiVector3D start = animation_channel_buffer[i].mPositionKeys[posIndex].mValue;
				aiVector3D end = animation_channel_buffer[i].mPositionKeys[nextPosIndex].mValue;

				aiVector3D delta = end - start;
				aiVector3D interpolated = start + factor * delta;

				res_pos_buffer[i] = glm::vec3(interpolated.x, interpolated.y, interpolated.z);
			}

			// rot
			if (animation_channel_buffer[i].mNumRotationKeys == 1) 
			{
				aiQuaternion rot = animation_channel_buffer[i].mRotationKeys[0].mValue;
				res_rot_buffer[i] = glm::quat(rot.w, rot.x, rot.y, rot.z);
				should_proceed_rot = false;
			}

			if (should_proceed_rot)
			{
				// Find keyframes surrounding animationTime
				u32 rotIndex = 0;
				for (u32 i = 0; i < animation_channel_buffer[i].mNumRotationKeys - 1; i++) 
				{
					if (animation_time < (float)animation_channel_buffer[i].mRotationKeys[i + 1].mTime)
					{
						rotIndex = i;
						break;
					}
				}

				u32 nextRotIndex = rotIndex + 1;

				float t1 = (float)animation_channel_buffer[i].mRotationKeys[rotIndex].mTime;
				float t2 = (float)animation_channel_buffer[i].mRotationKeys[nextRotIndex].mTime;
				float factor = (animation_time - t1) / (t2 - t1);

				aiQuaternion start = animation_channel_buffer[i].mRotationKeys[rotIndex].mValue;
				aiQuaternion end = animation_channel_buffer[i].mRotationKeys[nextRotIndex].mValue;

				glm::quat qStart(start.w, start.x, start.y, start.z);
				glm::quat qEnd(end.w, end.x, end.y, end.z);

				res_rot_buffer[i] = glm::slerp(qStart, qEnd, factor);
			}

			// scale
			if (animation_channel_buffer[i].mNumScalingKeys == 1) 
			{
				aiVector3D scale = animation_channel_buffer[i].mScalingKeys[0].mValue;
				res_scale_buffer[i] = glm::vec3(scale.x, scale.y, scale.z);
				should_proceed_scale = false;
			}

			if (should_proceed_scale)
			{
				// Find surrounding keyframes
				u32 scaleIndex = 0;
				for (u32 i = 0; i < animation_channel_buffer[i].mNumScalingKeys - 1; i++)
				{
					if (animation_time < (float)animation_channel_buffer[i].mScalingKeys[i + 1].mTime)
					{
						scaleIndex = i;
						break;
					}
				}

				u32 nextScaleIndex = scaleIndex + 1;

				f32 t1 = (f32)animation_channel_buffer[i].mScalingKeys[scaleIndex].mTime;
				f32 t2 = (f32)animation_channel_buffer[i].mScalingKeys[nextScaleIndex].mTime;
				f32 factor = (animation_time - t1) / (t2 - t1);

				aiVector3D start = animation_channel_buffer[i].mScalingKeys[scaleIndex].mValue;
				aiVector3D end = animation_channel_buffer[i].mScalingKeys[nextScaleIndex].mValue;

				aiVector3D delta = end - start;
				aiVector3D interpolated = start + factor * delta;

				res_scale_buffer[i] =  glm::vec3(interpolated.x, interpolated.y, interpolated.z);
			}

			glm::mat4 temp_translation_matrix = glm::translate(glm::mat4(1.0f), res_pos_buffer[i]);
			glm::mat4 temp_rotation_matrix = glm::toMat4(res_rot_buffer[i]);
			glm::mat4 temp_scale_matrix = glm::scale(glm::mat4(1.0f), res_scale_buffer[i]);

			res_matrix_buffer[i] = temp_rotation_matrix * temp_rotation_matrix * temp_scale_matrix;

			should_proceed_pos = true;
			should_proceed_rot = true;
			should_proceed_scale = true;

			glm::mat4 bone_animated_local_transform = res_matrix_buffer[i];
			glm::mat4 bone_global = parent_transform * bone_animated_local_transform;

			final_bone_position_buffer[i] = bone_global * bone_offset_matrix_buffer[i];
		}

		//   skinnedPos += boneMatrix * vec4(originalVertexPos, 1.0) * weight; // for each vertex that is affected by a certain bone
		
			
	}
	*/

u64 lightray_generate_guid64()
{
	u64 guid = ((u64)rand() << 32) | rand();

	return guid;
}

u128_t lightray_generate_guid128()
{
	u64 high64 = ((u64)rand() << 32) | rand();
	u64 low64 = ((u64)rand() << 32) | rand();

	u128_t guid;
	guid.high64 = high64;
	guid.low64 = low64;

	return guid;
}

bool lightray_raw_mouse_input_supported()
{
	return (i32)glfwRawMouseMotionSupported();
}

void lightray_enable_raw_mouse_input(GLFWwindow* window)
{
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

void lightray_disable_raw_mouse_input(GLFWwindow* window)
{
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
}

void lightray_set_target_fps(f32 desired_fps, f32* frame_duration_s, f32* core_fps)
{
	*frame_duration_s = 1.0f / desired_fps;
	*core_fps = desired_fps;
}

void lightray_suballocate_scene(lightray_scene_t* scene, const lightray_scene_suballocation_data_t* suballocation_data)
{
	scene->total_entity_count = suballocation_data->total_entity_count;
	scene->total_mesh_count = suballocation_data->total_mesh_count;
	scene->total_instance_model_count = suballocation_data->total_instance_model_count;

	const sunder_arena_suballocation_result_t entity_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_entity_t), scene->total_entity_count), alignof(lightray_entity_t));
	scene->entity_buffer = (lightray_entity_t*)entity_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t position_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(glm::vec3), scene->total_entity_count), alignof(glm::vec3));
	scene->position_buffer = (glm::vec3*)position_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t rotation_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(glm::vec3), scene->total_entity_count), alignof(glm::vec3));
	scene->rotation_buffer = (glm::vec3*)rotation_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t scale_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(glm::vec3), scene->total_entity_count), alignof(glm::vec3));
	scene->scale_buffer = (glm::vec3*)scale_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_binding_offsets_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_binding_offsets_t), scene->total_mesh_count), alignof(lightray_mesh_binding_offsets_t));
	scene->mesh_binding_offsets= (lightray_mesh_binding_offsets_t*)mesh_binding_offsets_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_binding_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_binding_t), scene->total_instance_model_count), alignof(lightray_mesh_binding_t));
	scene->mesh_binding_buffer = (lightray_mesh_binding_t*)mesh_binding_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_binding_metadata_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_binding_metadata_t), scene->total_mesh_count), alignof(lightray_mesh_binding_metadata_t));
	scene->mesh_binding_metadata_buffer = (lightray_mesh_binding_metadata_t*)mesh_binding_metadata_buffer_suballocation_result.data;

	for (u32 i = 0; i < scene->total_entity_count; i++)
	{
		scene->scale_buffer[i] = glm::vec3(1.0f);
	}

	for (u32 i = 0; i < scene->total_entity_count; i++)
	{
		SUNDER_SET_BIT(scene->visibility_flags, i, 1ull);
	}
}

u64 lightray_compute_scene_suballocation_size(const lightray_scene_suballocation_data_t* suballocation_data, u64 alignment)
{
	const u64 entity_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_entity_t), suballocation_data->total_entity_count, alignment);
	const u64 transform_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(glm::vec3), suballocation_data->total_entity_count * 3, alignment);
	const u64 mesh_binding_offsets_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_offsets_t), suballocation_data->total_mesh_count, alignment);
	const u64 mesh_binding_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_t), suballocation_data->total_instance_model_count, alignment);
	const u64 mesh_binding_metadata_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_metadata_t), suballocation_data->total_mesh_count, alignment);

	return  sunder_align64(entity_buffer_suballocation_size + transform_buffer_suballocation_size + mesh_binding_offsets_suballocation_size + mesh_binding_buffer_suballocation_size + mesh_binding_metadata_buffer_suballocation_size, alignment);
}

lightray_entity_creation_result_t lightray_create_entity(lightray_scene_t* scene, lightray_entity_kind kind)
{
	lightray_entity_creation_result_t res{};
	res.result = LIGHTRAY_RESULT_TOTAL_ENTITY_COUNT_IS_ZERO;
	res.index = UINT32_MAX;

	if (scene->total_entity_count == 0) { return res; }

	res.result = LIGHTRAY_RESULT_ENTITY_BUFFER_OVERFLOW;
	if (scene->current_entity_count == scene->total_entity_count) { return res; }

	res.result = LIGHTRAY_RESULT_SUCCESS;
	res.index = scene->current_entity_count;

	scene->entity_buffer[scene->current_entity_count].kind = kind;
	scene->current_entity_count++;

	return res;
}

lightray_result lightray_bind_mesh(lightray_scene_t* scene, u32 entity_index, u32 mesh_index, lightray_render_target_kind kind)
{
	if (entity_index > scene->total_entity_count - 1)
	{
		return LIGHTRAY_RESULT_INVALID_ENTITY_INDEX;
	}

	if (mesh_index > scene->total_mesh_count - 1)
	{
		return LIGHTRAY_RESULT_INVALID_MESH_INDEX;
	}

	if (scene->mesh_binding_count == scene->total_instance_model_count)
	{
		return LIGHTRAY_RESULT_MESH_BINDING_OVERFLOW;
	}

	if (scene->mesh_binding_metadata_buffer[mesh_index].current_binding_count == scene->mesh_binding_metadata_buffer[mesh_index].instance_count)
	{
		return LIGHTRAY_RESULT_COUNT_OF_MESH_BINDINGS_FOR_SPECIFIED_MESH_HAS_BEEN_EXCEEDED;
	}

	scene->entity_buffer[entity_index].mesh_binding_index = mesh_index;
	scene->mesh_binding_buffer[scene->mesh_binding_count].transform_index = entity_index;
	scene->mesh_binding_buffer[scene->mesh_binding_count].instance_model_index = scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index;
	scene->entity_buffer[entity_index].instance_model_binding_index = scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index;

	scene->mesh_binding_count++;
	scene->mesh_binding_metadata_buffer[mesh_index].current_binding_count++;

	if (scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index != scene->mesh_binding_offsets[mesh_index].last_opaque_instance_model_index)
	{
		scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index++;
	}


















	//if (mesh_index > scene->total_mesh_count - 1) { return LIGHTRAY_RESULT_INVALID_MESH_INDEX; }
	//if (scene->cpu_side_instance_model_count_buffer[mesh_index] == 0) { return LIGHTRAY_RESULT_INSTANCE_COUNT_OF_SPECIFIED_MESH_IS_ZERO; }
	//if (scene->instance_model_binding_count_buffer[mesh_index] >= scene->cpu_side_instance_model_count_buffer[mesh_index]) { return LIGHTRAY_RESULT_COUNT_OF_MESH_BINDINGS_FOR_SPECIFIED_MESH_HAS_BEEN_EXCEEDED; }
	//if (kind == LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] == 0) { return LIGHTRAY_RESULT_COLLISION_MESH_COUNT_OF_SPECIFIED_MESH_IS_ZERO; }

	//scene->entity_buffer[entity_index].mesh_binding_index = (u32)mesh_index;
	//lightray_buffer_index_query_result_t suitable_index{};

	//if (kind == LIGHTRAY_RENDER_TARGET_KIND_OPAQUE_MESH)
	//{
	//	suitable_index = lightray_query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], 0, scene->cpu_side_instance_model_count_buffer[mesh_index], UINT32_MAX, true);
	//}

	//else if (kind == LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] < scene->cpu_side_instance_model_count_buffer[mesh_index])
	//{
	//	u32 start = scene->cpu_side_instance_model_count_buffer[mesh_index] - scene->wireframe_mesh_count_buffer[mesh_index];
	//	u32 elm_count = scene->wireframe_mesh_count_buffer[mesh_index] + start;

	//	suitable_index = lightray_query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], start, elm_count, UINT32_MAX, true);
	//}

	//else if (kind == LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] == scene->cpu_side_instance_model_count_buffer[mesh_index])
	//{
	//	suitable_index = lightray_query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], 0, scene->cpu_side_instance_model_count_buffer[mesh_index], UINT32_MAX, true);
	//}

	//scene->entity_tick_buffer[scene->mesh_binding_count].converted_instance_model_index = suitable_index.value_at_index; //scene->suitiable_instance_model_offset_per_mesh_buffer[mesh_index];
	//scene->entity_buffer[entity_index].converted_instance_model_buffer_index = scene->mesh_binding_count; //scene->converted_instance_model_offset_buffer[scene->mesh_binding_count];
	//scene->entity_buffer[entity_index].instance_model_binding_index = scene->entity_tick_buffer[scene->mesh_binding_count].converted_instance_model_index;
	//scene->instance_model_binding_count_buffer[mesh_index] += 1;
	//scene->entity_tick_buffer[scene->entity_transform_index_buffer_iter].transform_index = entity_index;
	//scene->entity_transform_index_buffer_iter += 1;

	//scene->free_instance_model_offsets_per_mesh_buffers[mesh_index][suitable_index.return_index] = UINT32_MAX;
	//scene->mesh_binding_count++;

	return LIGHTRAY_RESULT_SUCCESS;
}

void lightray_hide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene)
{
	/*if (!(scene->visibility_flags & (1 << entity_index)))
	{
		SUNDER_LOG("\nalready hidden, unable to hide");
		return;
	}

	u32 mesh_binding_index = scene->entity_buffer[entity_index].mesh_binding_index;
	u32 instance_model_render_count = instance_model_to_render_count_buffer[mesh_binding_index];
	u32 instance_model_index = scene->entity_buffer[entity_index].instance_model_binding_index;
	u32 last_instance_model_index = instance_model_to_render_count_buffer[mesh_binding_index] + instance_model_buffer_offsets_per_mesh[mesh_binding_index] - 1;

	if (instance_model_render_count == 0) { return; }

	u32 free_hidden_instance_model_index = entity_index;
	scene->entity_tick_buffer[free_hidden_instance_model_index].hidden_instance_model_index = instance_model_index;

	scene->visibility_flags &= ~(1U << entity_index);

	hidden_instance_model_buffer[free_hidden_instance_model_index].model = instance_model_buffer[instance_model_index].model;

	if (last_instance_model_index == instance_model_index)
	{
		scene->entity_buffer[entity_index].instance_model_binding_index = UINT32_MAX;
		instance_model_to_render_count_buffer[mesh_binding_index] -= 1;
	}

	else if (instance_model_render_count > 2)
	{
		instance_model_buffer[instance_model_index].model = instance_model_buffer[last_instance_model_index].model;

		u32 swapped_entity_index = 0;
		for (u32 i = 0; i < scene->entity_count; i++)
		{
			if (scene->entity_buffer[i].instance_model_binding_index == last_instance_model_index)
			{
				swapped_entity_index = i;
				scene->entity_buffer[i].instance_model_binding_index = instance_model_index;
			}
		}

		scene->entity_tick_buffer[scene->entity_buffer[swapped_entity_index].converted_instance_model_buffer_index].converted_instance_model_index = instance_model_index;
		scene->entity_buffer[entity_index].instance_model_binding_index = UINT32_MAX;
		instance_model_to_render_count_buffer[mesh_binding_index] -= 1;
	}

	else if (instance_model_render_count == 1)
	{
		scene->entity_buffer[entity_index].instance_model_binding_index = UINT32_MAX;
		instance_model_to_render_count_buffer[mesh_binding_index] -= 1;
	}*/
}

void lightray_unhide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene)
{
	/*if ((scene->visibility_flags & (1 << entity_index)))
	{
		SUNDER_LOG("\nalready visible, unable to uhnide");
		return;
	}

	u32 mesh_binding_index = scene->entity_buffer[entity_index].mesh_binding_index;
	u32 next_instance_model_index = instance_model_to_render_count_buffer[mesh_binding_index] + instance_model_buffer_offsets_per_mesh[mesh_binding_index];

	scene->visibility_flags |= 1U << entity_index;
	u32 hidden_index = entity_index;
	instance_model_buffer[next_instance_model_index].model = hidden_instance_model_buffer[hidden_index].model;
	scene->entity_buffer[entity_index].instance_model_binding_index = next_instance_model_index;
	instance_model_to_render_count_buffer[mesh_binding_index] += 1;
	scene->entity_tick_buffer[scene->entity_buffer[entity_index].converted_instance_model_buffer_index].converted_instance_model_index = next_instance_model_index;*/
}

void lightray_bind_entity(lightray_scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index)
{
	scene->entity_buffer[entity_to_bind_index].entity_binding_index = entity_to_bind_to_index;
}

void lightray_move_entity(lightray_scene_t* scene, u32 self_aabb_index, u32 self_collision_mesh_index, const u32* index_buffer_offsets, const u32* index_count_buffer, const u32* index_buffer, const lightray_vertex_t* vertex_buffer, lightray_model_t* instance_model_buffer, glm::vec3* self_raw_vertex_positions, glm::vec3* self_projected_vertex_positions, glm::vec3* other_raw_vertex_positions, glm::vec3* other_projected_vertex_positions, u32 entity_index, const glm::vec3& direction)
{






		//for (u32 i = 0; i < scene->aabb_count; i++)
		//{
		//	const u32 tested_aabb_index = scene->aabb_index_buffer[i];
		//	const glm::vec3 aabb_position_a = scene->position_buffer[tested_aabb_index];
		//	const glm::vec3 aabb_scale_a = scene->scale_buffer[tested_aabb_index];

		//	for (u32 j = 0; j < scene->aabb_count; j++)
		//	{
		//		if (scene->aabb_index_buffer[j] != tested_aabb_index)
		//		{
		//			const glm::vec3 aabb_position_b = scene->position_buffer[scene->aabb_index_buffer[j]];
		//			const glm::vec3 aabb_scale_b = scene->scale_buffer[scene->aabb_index_buffer[j]];

		//			const bool intersect = lightray::aabbs_intersect(aabb_position_a, aabb_scale_a, aabb_position_b, aabb_scale_b);

		//			if (intersect)
		//			{
		//				//SUNDER_LOG("\naabbs intersect");

		//				for (u32 k = 0; k < scene->collision_mesh_count; k++)
		//				{
		//					const u32 aabb_entity_binding_index_a = scene->entity_buffer[tested_aabb_index].entity_binding_index;
		//					const lightray::es::entity_kind aabb_entity_binding_kind_a = scene->entity_buffer[aabb_entity_binding_index_a].kind;
		//					const u32 aabb_entity_binding_mesh_binding_index_a = scene->entity_buffer[aabb_entity_binding_index_a].mesh_binding_index;
		//					const u32 aabb_entity_binding_instance_model_binding_index_a = scene->entity_buffer[aabb_entity_binding_index_a].instance_model_binding_index;

		//					for (u32 l = 0; l < scene->collision_mesh_count; l++)
		//					{
		//						const u32 aabb_entity_binding_index_b = scene->entity_buffer[scene->aabb_index_buffer[j]].entity_binding_index;
		//						const lightray::es::entity_kind aabb_entity_binding_kind_b = scene->entity_buffer[aabb_entity_binding_index_b].kind;
		//						const u32 aabb_entity_binding_mesh_binding_index_b = scene->entity_buffer[aabb_entity_binding_index_b].mesh_binding_index;
		//						const u32 aabb_entity_binding_instance_model_binding_index_b = scene->entity_buffer[aabb_entity_binding_index_b].instance_model_binding_index;

		//						if (aabb_entity_binding_kind_a == lightray::es::ENTITY_KIND_COLLISION_MESH && aabb_entity_binding_kind_a == aabb_entity_binding_kind_b)
		//						{
		//							const u32 general_index_buffer_offset_a = index_buffer_offsets[aabb_entity_binding_mesh_binding_index_a];
		//							const u32 general_index_count_a = index_count_buffer[aabb_entity_binding_mesh_binding_index_a];
		//							const lightray::model_t general_model_matrix_a = instance_model_buffer[aabb_entity_binding_instance_model_binding_index_a];

		//							//glm::vec3 old_position_a = scene->position_buffer[aabb_entity_binding_index_a];
		//							//glm::vec3 old_position_b = scene->position_buffer[aabb_entity_binding_index_b];

		//							//scene->position_buffer[aabb_entity_binding_index_a] += movement_direction * 1.5f;
		//							//scene->position_buffer[aabb_entity_binding_index_b] += movement_direction * 1.5f;

		//							const u32 general_index_buffer_offset_b = index_buffer_offsets[aabb_entity_binding_mesh_binding_index_b];
		//							const u32 general_index_count_b = index_count_buffer[aabb_entity_binding_mesh_binding_index_b];
		//							const lightray::model_t general_model_matrix_b = instance_model_buffer[aabb_entity_binding_instance_model_binding_index_b];

		//							lightray::get_raw_vertex_positions(general_index_buffer_offset_a, general_index_count_a, raw_vertex_positions_a, vertex_buffer, index_buffer);
		//							lightray::get_projected_vertex_positions(raw_vertex_positions_a, projected_vertex_positions_a, general_index_count_a, general_model_matrix_a);

		//							lightray::get_raw_vertex_positions(general_index_buffer_offset_b, general_index_count_b, raw_vertex_positions_b, vertex_buffer, index_buffer);
		//							lightray::get_projected_vertex_positions(raw_vertex_positions_b, projected_vertex_positions_b, general_index_count_b, general_model_matrix_b);

		//							const bool collision_meshes_intersect = lightray::gjk_intersect(projected_vertex_positions_a, general_index_count_a, projected_vertex_positions_b, general_index_count_b);

		//							if (collision_meshes_intersect)
		//							{
		//								/*scene.entity_buffer[aabb_entity_binding_index_a].flags |= 1U << lightray::es::ENTITY_BITS_IS_COLLIDING;
		//								scene.entity_buffer[aabb_entity_binding_index_b].flags |= 1U << lightray::es::ENTITY_BITS_IS_COLLIDING;

		//								scene.position_buffer[aabb_entity_binding_index_a] = old_position_a;
		//								scene.position_buffer[aabb_entity_binding_index_b] = old_position_b;*/

		//								SUNDER_LOG("\ncollision meshes intersect");

		//								break;
		//							}

		//							else
		//							{

		//								/*scene.position_buffer[aabb_entity_binding_index_a] = old_position_a;
		//								scene.position_buffer[aabb_entity_binding_index_b] = old_position_b;

		//								scene.entity_buffer[aabb_entity_binding_index_a].flags &= ~(1U << lightray::es::ENTITY_BITS_IS_COLLIDING);
		//								scene.entity_buffer[aabb_entity_binding_index_b].flags &= ~(1U << lightray::es::ENTITY_BITS_IS_COLLIDING);*/

		//								/*SUNDER_LOG("\n");
		//								SUNDER_LOG(scene.entity_buffer[aabb_entity_binding_index_a].flags);
		//								SUNDER_LOG("\n");
		//								SUNDER_LOG(scene.entity_buffer[aabb_entity_binding_index_b].flags);*/
		//							}
		//						}
		//					}
		//				}
		//			}
		//		}
		//	}
		//}

		//scene->position_buffer[entity_index] += direction;
}

u32 lightray_get_entity_bound_collision_mesh_index_count(const lightray_scene_t* scene, u32 entity_index)
{
	/*u32 collision_mesh_index_iter = 0;

	for (u32 i = 0; i < scene->collision_mesh_count; i++)
	{
		u32 current_collision_mesh_index = scene->collision_mesh_index_buffer[i];

		if (scene->entity_buffer[current_collision_mesh_index].entity_binding_index == entity_index)
		{
			collision_mesh_index_iter++;
		}
	}

	return collision_mesh_index_iter;*/

	return 0;
}

void lightray_get_entity_bound_collision_mesh_indices(const lightray_scene_t* scene, u32 entity_index, u32* collision_mesh_index_buffer)
{
	/*u32 collision_mesh_index_buffer_iter = 0;

	for (u32 i = 0; i < scene->collision_mesh_count; i++)
	{
		u32 current_collision_mesh_index = scene->collision_mesh_index_buffer[i];

		if (scene->entity_buffer[current_collision_mesh_index].entity_binding_index == entity_index)
		{
			collision_mesh_index_buffer[collision_mesh_index_buffer_iter] = current_collision_mesh_index;
			collision_mesh_index_buffer_iter++;
		}
	}*/
}

u32 lightray_get_entity_bound_aabb_index_count(const lightray_scene_t* scene, u32 collision_mesh_index)
{
	/*u32 aabb_index_count_iter = 0;

	for (u32 i = 0; i < scene->aabb_count; i++)
	{
		u32 current_aabb_index = scene->aabb_index_buffer[i];

		if (scene->entity_buffer[current_aabb_index].entity_binding_index == collision_mesh_index)
		{
			aabb_index_count_iter++;
		}
	}

	return aabb_index_count_iter;*/

	return 0;
}

void	lightray_get_entity_bound_aabb_indices(const lightray_scene_t* scene, u32 collision_mesh_index, u32* aabb_index_buffer)
{
	/*u32 aabb_index_buffer_iter = 0;

	for (u32 i = 0; i < scene->aabb_count; i++)
	{
		u32 current_aabb_index = scene->aabb_index_buffer[i];

		if (scene->entity_buffer[current_aabb_index].entity_binding_index == collision_mesh_index)
		{
			aabb_index_buffer[aabb_index_buffer_iter] = current_aabb_index;
			aabb_index_buffer_iter++;
		}
	}*/
}

u8 lightray_pack_f32_to_u8(f32 f)
{
	const f32 clamped = sunder_clamp_f32(0.0f, 1.0f, f);
	const u8 scaled = (u8)(clamped * 255.0f + 0.5f);

	return sunder_clamp_u8(0, 255, scaled);
}

f32 lightray_unpack_u8_to_f32(u8 v)
{
	return sunder_clamp_f32(0.0f, 1.0f, (f32)v / 255);
}

glm::vec3 lightray_assimp_to_glm_vec3(const aiVector3D& vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

glm::quat lightray_assimp_to_glm_quat(const aiQuaternion& quat)
{
	return glm::quat(quat.w, quat.x, quat.y, quat.z);
}

void lightray_log_animation_playback_frame_time(f32 current_frame_time, f32 animation_duration)
{
	SUNDER_LOG(current_frame_time);
	SUNDER_LOG("/");
	SUNDER_LOG(animation_duration);
}

u32 lightray_get_suitable_scale_key_index(const lightray_animation_key_vec3_t* scale_key_buffer, u32 scale_key_buffer_offset, u32 scale_key_count, f32 animation_in_ticks)
{
	for (u32 i = 0; i < scale_key_count - 1; i++)
	{
		const u32 scale_key_buffer_index = scale_key_buffer_offset + i;
		const f32 t = scale_key_buffer[scale_key_buffer_index + 1].time;

		if (animation_in_ticks < t)
		{
			return scale_key_buffer_index;
		}
	}

	return scale_key_buffer_offset;
}

u32 lightray_get_suitable_rotation_key_index(const lightray_animation_key_quat_t* rotation_key_buffer, u32 rotation_key_buffer_offset, u32 rotation_key_count, f32 animation_in_ticks)
{
	for (u32 i = 0; i < rotation_key_count - 1; i++)
	{
		const u32 rotation_key_buffer_index = rotation_key_buffer_offset + i;
		const f32 t = rotation_key_buffer[rotation_key_buffer_index + 1].time;

		if (animation_in_ticks < t)
		{
			return rotation_key_buffer_index;
		}
	}

	return rotation_key_buffer_offset;
}

u32 lightray_get_suitable_position_key_index(const lightray_animation_key_vec3_t* position_key_buffer, u32 position_key_buffer_offset, u32 position_key_count, f32 animation_in_ticks)
{
	for (u32 i = 0; i < position_key_count - 1; i++)
	{
		const u32 position_key_buffer_index = position_key_buffer_offset + i;
		const f32 t = position_key_buffer[position_key_buffer_index + 1].time;

		if (animation_in_ticks < t)
		{
			return position_key_buffer_index;
		}
	}

	return position_key_buffer_offset;
}

glm::vec3 lightray_compute_interpolated_animation_channel_position_key(const lightray_animation_channel_t* channel, const lightray_animation_key_vec3_t* position_key_buffer, f32 animation_in_ticks)
{
	const u32 position_key_buffer_offset = channel->position_key_buffer_offset;
	const u32 position_key_count = channel->position_key_count;

	if (position_key_count == 1)
	{
		return position_key_buffer[position_key_buffer_offset].vec;
	}

	const u32 key_position_index = lightray_get_suitable_position_key_index(position_key_buffer, position_key_buffer_offset, position_key_count, animation_in_ticks);
	const u32 next_key_position_index = key_position_index + 1;

	const f32 t1 = position_key_buffer[key_position_index].time;
	const f32 t2 = position_key_buffer[next_key_position_index].time;
	const f32 delta_time = t2 - t1;
	const f32 factor = (animation_in_ticks - t1) / delta_time;

	const glm::vec3 start = position_key_buffer[key_position_index].vec;
	const glm::vec3 end = position_key_buffer[next_key_position_index].vec;
	const glm::vec3 delta_vec = end - start;
	
	return start + factor * delta_vec;
}

glm::quat lightray_compute_interpolated_animation_channel_rotation_key(const lightray_animation_channel_t* channel, const lightray_animation_key_quat_t* rotation_key_buffer, f32 animation_in_ticks)
{
	const u32 rotation_key_buffer_offset = channel->rotation_key_buffer_offset;
	const u32 rotation_key_count = channel->rotation_key_count;
	
	if (rotation_key_count == 1)
	{
		return rotation_key_buffer[rotation_key_buffer_offset].quat;
	}

	const u32 rotation_key_index = lightray_get_suitable_rotation_key_index(rotation_key_buffer, rotation_key_buffer_offset, rotation_key_count, animation_in_ticks);
	const u32 next_rotation_key_index = rotation_key_index + 1;

	const f32 t1 = rotation_key_buffer[rotation_key_index].time;
	const f32 t2 = rotation_key_buffer[next_rotation_key_index].time;
	const f32 delta_time = t2 - t1;
	const f32 factor = (animation_in_ticks - t1) / delta_time;

	const glm::quat start = rotation_key_buffer[rotation_key_index].quat;
	const glm::quat end = rotation_key_buffer[next_rotation_key_index].quat;

	return glm::normalize(glm::slerp(start, end, factor));
}

glm::vec3 lightray_compute_interpolated_animation_channel_scale_key(const lightray_animation_channel_t* channel, const lightray_animation_key_vec3_t* scale_key_buffer, f32 animation_in_ticks)
{
	const u32 scale_key_buffer_offset = channel->scale_key_buffer_offset;
	const u32 scale_key_count = channel->scale_key_count;

	if (scale_key_count == 1)
	{
		return scale_key_buffer[scale_key_buffer_offset].vec;
	}

	const u32 scale_key_index = lightray_get_suitable_scale_key_index(scale_key_buffer, scale_key_buffer_offset, scale_key_count, animation_in_ticks);
	const u32 next_scale_key_index = scale_key_index + 1;

	const f32 t1 = scale_key_buffer[scale_key_index].time;
	const f32 t2 = scale_key_buffer[next_scale_key_index].time;
	const f32 delta_time = t2 - t1;
	const f32 factor = (animation_in_ticks - t1) / delta_time;

	const glm::vec3 start = scale_key_buffer[scale_key_index].vec;
	const glm::vec3 end = scale_key_buffer[next_scale_key_index].vec;
	const glm::vec3 delta_vec = end - start;

	return start + factor * delta_vec;
}

void lightray_compute_interpolated_skeleton_transform(lightray_animation_core_t* animation_core, u32 animation_index, u32 skeleton_index, u32 instance_index)
{
	const u32 current_node_count = animation_core->skeleton_buffer[skeleton_index].node_count;
	const u32 current_node_buffer_offset = animation_core->skeleton_buffer[skeleton_index].node_buffer_offset;
	const u32 current_skeletal_mesh_bone_buffer_offset = animation_core->skeleton_buffer[skeleton_index].bone_buffer_offset;
	const u32 current_animation_channel_buffer_offset = animation_core->animation_buffer[animation_index].channel_buffer_offset;
	const u32 final_bone_matrices_buffer_offset_with_respect_to_instance = lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(instance_index, animation_core->skeleton_buffer[skeleton_index].bone_count, animation_core->skeleton_buffer[skeleton_index].computed_bone_transform_matrix_buffer_offset);

	for (u32 i = 1; i < current_node_count; i++)
	{
		const u32 current_node_index = current_node_buffer_offset + i;
		const u32 current_parent_index = current_node_buffer_offset + animation_core->node_buffer[current_node_index].parent_index;
		const u32 current_bone_buffer_index = animation_core->node_buffer[current_node_index].bone_buffer_index;
		const u32 current_animation_channel_buffer_index = animation_core->node_buffer[current_node_index].animation_channel_buffer_index;

		glm::mat4 local_transform_matrix = animation_core->node_buffer[current_node_index].local_transform_matrix;

		if (current_animation_channel_buffer_index != LIGHTRAY_INVALID_NODE_ANIMATION_CHANNEL_BUFFER_INDEX)
		{
			const u32 current_animation_channel_index = current_animation_channel_buffer_offset + current_animation_channel_buffer_index;
			const lightray_animation_channel_t* current_animation_channel = &animation_core->animation_channel_buffer[current_animation_channel_index];

			const glm::vec3 scale_key = lightray_compute_interpolated_animation_channel_scale_key(current_animation_channel, animation_core->animation_scale_key_buffer, animation_core->playback_command_buffer[instance_index].ticks);
			const glm::quat rotation_key = lightray_compute_interpolated_animation_channel_rotation_key(current_animation_channel, animation_core->animation_rotation_key_buffer, animation_core->playback_command_buffer[instance_index].ticks);
			const glm::vec3 position_key = lightray_compute_interpolated_animation_channel_position_key(current_animation_channel, animation_core->animation_position_key_buffer, animation_core->playback_command_buffer[instance_index].ticks);

			const glm::mat4 identity = glm::mat4(1.0f);
			const glm::mat4 scale_matrix = glm::scale(identity, scale_key);
			const glm::mat4 rotation_matrix = glm::toMat4(rotation_key);
			const glm::mat4 translation_matrix = glm::translate(identity, position_key);

			local_transform_matrix = translation_matrix * rotation_matrix * scale_matrix;
		}

		animation_core->node_buffer[current_node_index].global_transform_matrix = animation_core->node_buffer[current_parent_index].global_transform_matrix * local_transform_matrix;

		if (current_bone_buffer_index != LIGHTRAY_INVALID_NODE_BONE_BUFFER_INDEX)
		{
			animation_core->computed_bone_matrix_buffer[final_bone_matrices_buffer_offset_with_respect_to_instance + current_bone_buffer_index] = animation_core->global_root_inverse_matrix_buffer[skeleton_index] * animation_core->node_buffer[current_node_index].global_transform_matrix * animation_core->bone_buffer[current_skeletal_mesh_bone_buffer_offset + current_bone_buffer_index].inverse_bind_pose_matrix;
		}
	}
}

u32 lightray_compute_skeletal_mesh_bone_count_with_respect_to_instance_count(u32 bone_count, u32 instance_count)
{
	return bone_count * instance_count;
}

u32 lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(u32 instance_index, u32 bone_count, u32 base_offset)
{
	return (instance_index * bone_count) + base_offset;
}

void lightray_assimp_get_node_hierarchy_metadata(const aiNode* node, u32* total_node_count, u64* aligned_name_byte_code_size, u32 alignment)
{
	(*aligned_name_byte_code_size) = sunder_update_aligned_value_u64(*aligned_name_byte_code_size, node->mName.length, alignment);
	(*total_node_count)++;
	const u32 node_children_count = node->mNumChildren;

	for (u32 i = 0; i < node_children_count; i++)
	{
		lightray_assimp_get_node_hierarchy_metadata(node->mChildren[i], total_node_count, aligned_name_byte_code_size, alignment);
	}
}

void lightray_populate_node_related_string_upon_suballocation(sunder_arena_t* arena, sunder_string_t* host, cstring_literal* string, u32 length)
{
	const sunder_arena_suballocation_result_t string_suballocation_result = sunder_suballocate_from_arena_debug(arena, length + 1, 2);
	host->data = SUNDER_CAST(char*, string_suballocation_result.data);
	host->length = length;

	for (u32 n = 0; n < length; n++)
	{
		host->data[n] = string[n];
	}

	host->data[length] = 0;
}

void lightray_assimp_execute_first_node_buffer_population_pass(const aiNode* node, lightray_node_t* node_buffer, sunder_arena_t* arena, sunder_string_t* names, u32* current_index)
{
	const u32 node_name_length = node->mName.length;
	cstring_literal* node_name = node->mName.C_Str();
	const u32 node_children_count = node->mNumChildren;
	const u32 derefed_current_index = *current_index;

	SUNDER_LOG("\n");
	SUNDER_LOG(derefed_current_index);

	SUNDER_LOG("\n");
	SUNDER_LOG(node_name);

	node_buffer[derefed_current_index].local_transform_matrix = lightray_assimp_to_glm_mat4(node->mTransformation);
	lightray_populate_node_related_string_upon_suballocation(arena, &names[derefed_current_index], node_name, node_name_length);
	(*current_index)++;

	for (u32 i = 0; i < node_children_count; i++)
	{
		lightray_assimp_execute_first_node_buffer_population_pass(node->mChildren[i], node_buffer, arena, names, current_index);
	}
}

void lightray_assimp_execute_second_node_buffer_population_pass(const aiNode* node, lightray_node_t* node_buffer, u32 node_count, const sunder_string_t* names, u32* current_index)
{
	const u32 derefed_current_index = *current_index;
	SUNDER_LOG("\n");
	SUNDER_LOG(derefed_current_index);
	const u32 node_children_count = node->mNumChildren;
	u32 parent_index = LIGHTRAY_INVALID_NODE_PARENT_INDEX;

	if (node->mParent != nullptr)
	{
		const u32 parent_node_name_length = node->mParent->mName.length;
		cstring_literal* parent_node_name = node->mParent->mName.C_Str();

		for (u32 i = 0; i < node_count; i++)
		{
			const bool parent_node_names_match = sunder_compare_strings(names[i].data, names[i].length, parent_node_name, parent_node_name_length);

			if (parent_node_names_match)
			{
				parent_index = i;
				break;
			}
		}
	}

	node_buffer[derefed_current_index].parent_index = parent_index;
	(*current_index)++;

	for (u32 i = 0; i < node_children_count; i++)
	{
		lightray_assimp_execute_second_node_buffer_population_pass(node->mChildren[i], node_buffer, node_count, names, current_index);
	}
}

void lightray_log_mesh_binding_offset_buffer(const lightray_scene_t* scene)
{
	SUNDER_LOG("\n\nmesh binding offsets\n");
	for (u32 i = 0; i < scene->total_mesh_count; i++)
	{
		SUNDER_LOG(scene->mesh_binding_offsets[i].current_opaque_instance_model_index);
		SUNDER_LOG(" ");
		SUNDER_LOG(scene->mesh_binding_offsets[i].last_opaque_instance_model_index);
		SUNDER_LOG("\n\n");
	}
}

void lightray_log_mesh_binding_metadata_buffer(const lightray_scene_t* scene)
{
	SUNDER_LOG("\n\nmesh binding metadata buffer\n");
	for (u32 i = 0; i < scene->total_mesh_count; i++)
	{
		SUNDER_LOG(scene->mesh_binding_metadata_buffer[i].current_binding_count);
		SUNDER_LOG(" ");
		SUNDER_LOG(scene->mesh_binding_metadata_buffer[i].instance_count);
		SUNDER_LOG("\n\n");
	}
}

void lightray_log_mesh_binding_buffer(const lightray_scene_t* scene)
{
	SUNDER_LOG("\n\nmesh binding buffer\n");
	for (u32 i = 0; i < scene->mesh_binding_count; i++)
	{
		SUNDER_LOG(scene->mesh_binding_buffer[i].transform_index);
		SUNDER_LOG(" ");
		SUNDER_LOG(scene->mesh_binding_buffer[i].instance_model_index);
		SUNDER_LOG("\n\n");
	}
}

u32 lightray_get_skeletal_mesh_global_mesh_index(u32 skeletal_mesh_index, u32 static_mesh_count)
{
	return static_mesh_count + skeletal_mesh_index;
}

glm::vec2 lightray_get_cursor_position(GLFWwindow* window)
{
	f64 x = 0;
	f64 y = 0;
	glfwGetCursorPos(window, &x, &y);

	return glm::vec2(x, y);
}