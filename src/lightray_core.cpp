#include "lightray_core.h"

void lightray_get_shader_byte_code(const std::filesystem::path& shader_path, i8* buffer, u64* buffer_size_in_bytes, u64 byte_code_size_limit)
{
	std::ifstream file(shader_path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) { return; }
	u64 file_size = (u64)file.tellg();
	if (file_size == 0 || file_size > byte_code_size_limit) { return; }
	(*buffer_size_in_bytes) = file_size;
	file.seekg(0);
	file.read(buffer, file_size);
	if (file.fail()) { return; }
	file.close();
}

std::filesystem::path lightray_get_project_root_path()
{
	char buffer[1024];
	GetModuleFileNameA(NULL, buffer, 1024);
	std::filesystem::path executable_path = std::filesystem::path(buffer).parent_path();
	SUNDER_LOG("\nEXE DIRECTORY");
	SUNDER_LOG(executable_path);

	return executable_path.parent_path().parent_path();
}

u64 lightray_get_shader_byte_code_size(const std::filesystem::path& shader_path)
{
	std::ifstream file(shader_path, std::ios::ate | std::ios::binary);
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

bool lightray_is_key_pressed(GLFWwindow* window, i32 key, u8* key_tick_data)
{
	if (glfwGetKey(window, key) == GLFW_PRESS)
	{
		if ((*key_tick_data) & 1U << BITS_KEY_CAN_BE_PRESSED)
		{
			(*key_tick_data) &= ~(1U << BITS_KEY_CAN_BE_PRESSED); // can_press = false;
			return true;
		}
	}

	else
	{
		(*key_tick_data) |= 1U << BITS_KEY_CAN_BE_PRESSED; // can_press = true;
	}

	return false;
}

bool lightray_is_key_down(GLFWwindow* window, i32 key)
{
	return glfwGetKey(window, key) == GLFW_PRESS;
}

bool lightray_is_mouse_button_pressed(GLFWwindow* window, i32 mouse_button, u8* key_tick_data)
{
	if (glfwGetMouseButton(window, mouse_button) == GLFW_PRESS)
	{
		if ((*key_tick_data) & 1U << BITS_KEY_CAN_BE_PRESSED)
		{
			(*key_tick_data) &= ~(1U << BITS_KEY_CAN_BE_PRESSED); // can_press = false;
			return true;
		}
	}

	else
	{
		(*key_tick_data) |= 1U << BITS_KEY_CAN_BE_PRESSED; // can_press = true;
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

void lightray_get_projected_vertex_positions(const glm::vec3* raw_vertex_positions, glm::vec3* projected_vertex_positions, u64 vertex_count, const lightray_model_t& model)
{
	for (u64 i = 0; i < vertex_count; i++)
	{
		projected_vertex_positions[i] = glm::vec3(model.model * glm::vec4(raw_vertex_positions[i], 1.0f));
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
}

void lightray_assimp_get_scene_node_count_helper(aiNode* node, u32* node_count)
{
	*node_count += node->mNumChildren;

	for (u32 i = 0; i < node->mNumChildren; i++)
	{
		lightray_assimp_get_scene_node_count_helper(node->mChildren[i], node_count);
	}
}

u32 lightray_assimp_get_scene_node_count(aiScene* scene)
{
	u32 node_count = 0;
	lightray_assimp_get_scene_node_count_helper(scene->mRootNode, &node_count);

	return node_count;
}

void lightray_assimp_get_scene_node_names(aiNode* node, cstring_literal** name_buffer)
{
	u32 name_buffer_iter = 0;
	name_buffer[name_buffer_iter] = node->mName.C_Str();
	name_buffer_iter++;

	for (u32 i = 0; i < node->mNumChildren; i++)
	{
		lightray_assimp_get_scene_node_names(node->mChildren[i], name_buffer);
	}
}

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

void lightray_allocate_scene(lightray_scene_t* scene, lightray_scene_allocation_data_t* scene_allocation_data)
{
	scene->total_entity_count = scene_allocation_data->total_entity_count;
	scene->total_mesh_count = scene_allocation_data->total_mesh_count;
	scene->cpu_side_instance_model_count_buffer = scene_allocation_data->cpu_side_instance_model_count_buffer;
	scene->instance_model_count = scene_allocation_data->instance_model_count;
	scene->wireframe_mesh_count_buffer = scene_allocation_data->wireframe_mesh_count_buffer;
	//scene->instance_count_buffer = scene_allocation_data->instance_count_buffer;

	sunder_arena_suballocation_result_t entity_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(lightray_entity_t), alignof(lightray_entity_t));
	scene->entity_buffer = (lightray_entity_t*)entity_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t position_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(glm::vec3), alignof(glm::vec3));
	scene->position_buffer = (glm::vec3*)position_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t rotation_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(glm::vec3), alignof(glm::vec3));
	scene->rotation_buffer = (glm::vec3*)rotation_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t scale_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(glm::vec3), alignof(glm::vec3));
	scene->scale_buffer = (glm::vec3*)scale_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t light_color_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_light_count * sizeof(glm::vec3), alignof(glm::vec3));
	scene->light_color_buffer = (glm::vec3*)light_color_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t entity_transform_index_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(u32), alignof(u32));
	scene->entity_transform_index_buffer = (u32*)entity_transform_index_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t converted_instance_model_offset_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->instance_model_count * sizeof(u32), alignof(u32));
	scene->converted_instance_model_offset_buffer = (u32*)converted_instance_model_offset_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t cpu_side_instance_model_buffer_offsets_per_mesh_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_mesh_count * sizeof(u32), alignof(u32));
	scene->cpu_side_instance_model_buffer_offsets_per_mesh = (u32*)cpu_side_instance_model_buffer_offsets_per_mesh_suballoc_result.data;

	sunder_arena_suballocation_result_t instance_model_binding_count_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_mesh_count * sizeof(u32), alignof(u32));
	scene->instance_model_binding_count_buffer = (u32*)instance_model_binding_count_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t hidden_instance_model_index_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->instance_model_count * sizeof(u32), alignof(u32));
	scene->hidden_instance_model_index_buffer = (u32*)hidden_instance_model_index_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t free_instance_model_offsets_per_mesh_buffers_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_mesh_count * sizeof(u32*), alignof(u32*));
	scene->free_instance_model_offsets_per_mesh_buffers = (u32**)free_instance_model_offsets_per_mesh_buffers_suballoc_result.data;

	for (u32 i = 0; i < scene_allocation_data->total_mesh_count; i++)
	{
		sunder_arena_suballocation_result_t free_instance_model_offsets_per_mesh_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, (u32)scene->cpu_side_instance_model_count_buffer[i] * sizeof(u32), alignof(u32));
		scene->free_instance_model_offsets_per_mesh_buffers[i] = (u32*)free_instance_model_offsets_per_mesh_buffer_suballoc_result.data;
	}

	sunder_arena_suballocation_result_t aabb_index_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(u32), alignof(u32));
	scene->aabb_index_buffer = (u32*)aabb_index_buffer_suballoc_result.data;

	sunder_arena_suballocation_result_t collision_mesh_index_buffer_suballoc_result = sunder_suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(u32), alignof(u32));
	scene->collision_mesh_index_buffer = (u32*)collision_mesh_index_buffer_suballoc_result.data;

	for (u32 i = 0; i < scene_allocation_data->total_entity_count; i++)
	{
		scene->entity_buffer[i].mesh_binding_index = UNDEFINED_VALUE_MESH_BINDING_INDEX;
		scene->entity_buffer[i].instance_model_binding_index = UNDEFINED_VALUE_INSTANCE_MODEL_BINDING_INDEX;
		scene->entity_buffer[i].entity_binding_index = UNDEFINED_VALUE_ENTITY_BINDING_INDEX;
		scene->entity_buffer[i].converted_instance_model_buffer_index = UNDEFINED_VALUE_CONVERTED_INSTANCE_MODEL_BUFFER_INDEX;
		scene->aabb_index_buffer[i] = UNDEFINED_VALUE_AABB_INDEX;
		scene->collision_mesh_index_buffer[i] = UNDEFINED_VALUE_COLLISION_MESH_INDEX;
	}

	for (u32 i = 0; i < scene->total_mesh_count; i++)
	{
		scene->cpu_side_instance_model_buffer_offsets_per_mesh[i] = scene_allocation_data->cpu_side_instance_model_buffer_offsets_per_mesh[i];
	}

	for (u32 i = 0; i < scene_allocation_data->total_mesh_count; i++)
	{
		u32 offset = 0;

		for (u32 j = 0; j < (u32)scene->cpu_side_instance_model_count_buffer[i]; j++)
		{
			scene->free_instance_model_offsets_per_mesh_buffers[i][j] = scene->cpu_side_instance_model_buffer_offsets_per_mesh[i] + offset;
			offset++;
		}

		offset = 0;
	}

		/*
		for (u32 i = 0; i < scene_allocation_data->total_mesh_count; i++)
		{
			SUNDER_LOG("\n");
			for (u32 j = 0; j < (u32)scene->cpu_side_instance_model_count_buffer[i]; j++)
			{
				SUNDER_LOG(scene->free_instance_model_offsets_per_mesh_buffers[i][j]);
				SUNDER_LOG(" ");
			}
		}
		*/

	for (u32 i = 0; i < scene->total_entity_count; i++)
	{
		scene->scale_buffer[i].x = 1.0f;
		scene->scale_buffer[i].y = 1.0f;
		scene->scale_buffer[i].z = 1.0f;
	}

	for (u32 i = 0; i < scene->instance_model_count; i++)
	{
		scene->hidden_instance_model_index_buffer[i] = UINT32_MAX;
	}

	for (u32 i = 0; i < scene->instance_model_count; i++)
	{
		scene->visibility_flags |= 1 << i;
	}
}

lightray_entity_creation_result_t create_entity(lightray_scene_t* scene, lightray_entity_kind kind)
{
	lightray_entity_creation_result_t res;
	res.result = RESULT_TOTAL_ENTITY_COUNT_IS_ZERO;
	res.index = UINT32_MAX;
	if (scene->total_entity_count == 0) { return res; }

	res.result = RESULT_ENTITY_BUFFER_OVERFLOW;
	if (scene->entity_count == scene->total_entity_count) { return res; }

	res.result = RESULT_SUCCESS;
	res.index = scene->entity_count;

	scene->entity_buffer[scene->entity_count].kind = kind;

	if (kind == ENTITY_KIND_AABB)
	{
		scene->aabb_index_buffer[scene->aabb_count] = scene->entity_count;
		scene->aabb_count++;
	}

	if (kind == ENTITY_KIND_COLLISION_MESH)
	{
		scene->collision_mesh_index_buffer[scene->collision_mesh_count] = scene->entity_count;
		scene->collision_mesh_count++;
	}

	scene->entity_count++;

	return res;
	
}

lightray_result bind_mesh(lightray_scene_t* scene, u32 entity_index, u32 mesh_index, lightray_render_target_kind kind)
{
	if (mesh_index > scene->total_mesh_count - 1) { return RESULT_INVALID_MESH_INDEX; }
	if (scene->cpu_side_instance_model_count_buffer[mesh_index] == 0) { return RESULT_INSTANCE_COUNT_OF_SPECIFIED_MESH_IS_ZERO; }
	if (scene->instance_model_binding_count_buffer[mesh_index] >= scene->cpu_side_instance_model_count_buffer[mesh_index]) { return RESULT_COUNT_OF_MESH_BINDINGS_FOR_SPECIFIED_MESH_HAS_BEEN_EXCEEDED; }
	if (kind == RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] == 0) { return RESULT_COLLISION_MESH_COUNT_OF_SPECIFIED_MESH_IS_ZERO; }

	scene->entity_buffer[entity_index].mesh_binding_index = (u32)mesh_index;
	lightray_buffer_index_query_result_t suitable_index{};

	if (kind == RENDER_TARGET_KIND_OPAQUE_MESH)
	{
		suitable_index = lightray_query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], 0, scene->cpu_side_instance_model_count_buffer[mesh_index], UINT32_MAX, true);
	}

	else if (kind == RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] < scene->cpu_side_instance_model_count_buffer[mesh_index])
	{
		u32 start = scene->cpu_side_instance_model_count_buffer[mesh_index] - scene->wireframe_mesh_count_buffer[mesh_index];
		u32 elm_count = scene->wireframe_mesh_count_buffer[mesh_index] + start;

		suitable_index = lightray_query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], start, elm_count, UINT32_MAX, true);
	}

	else if (kind == RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] == scene->cpu_side_instance_model_count_buffer[mesh_index])
	{
		suitable_index = lightray_query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], 0, scene->cpu_side_instance_model_count_buffer[mesh_index], UINT32_MAX, true);
	}

	scene->converted_instance_model_offset_buffer[scene->mesh_binding_count] = suitable_index.value_at_index; //scene->suitiable_instance_model_offset_per_mesh_buffer[mesh_index];
	scene->entity_buffer[entity_index].converted_instance_model_buffer_index = scene->mesh_binding_count; //scene->converted_instance_model_offset_buffer[scene->mesh_binding_count];
	scene->entity_buffer[entity_index].instance_model_binding_index = scene->converted_instance_model_offset_buffer[scene->mesh_binding_count];
	scene->instance_model_binding_count_buffer[mesh_index] += 1;
	scene->entity_transform_index_buffer[scene->entity_transform_index_buffer_iter] = entity_index;
	scene->entity_transform_index_buffer_iter += 1;

	scene->free_instance_model_offsets_per_mesh_buffers[mesh_index][suitable_index.return_index] = UINT32_MAX;
	scene->mesh_binding_count++;

	return RESULT_SUCCESS;
}

void lightray_hide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene)
{
	if (!(scene->visibility_flags & (1 << entity_index)))
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
	scene->hidden_instance_model_index_buffer[free_hidden_instance_model_index] = instance_model_index;

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

		scene->converted_instance_model_offset_buffer[scene->entity_buffer[swapped_entity_index].converted_instance_model_buffer_index] = instance_model_index;
		scene->entity_buffer[entity_index].instance_model_binding_index = UINT32_MAX;
		instance_model_to_render_count_buffer[mesh_binding_index] -= 1;
	}

	else if (instance_model_render_count == 1)
	{
		scene->entity_buffer[entity_index].instance_model_binding_index = UINT32_MAX;
		instance_model_to_render_count_buffer[mesh_binding_index] -= 1;
	}
}

void lightray_unhide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene)
{
	if ((scene->visibility_flags & (1 << entity_index)))
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
	scene->converted_instance_model_offset_buffer[scene->entity_buffer[entity_index].converted_instance_model_buffer_index] = next_instance_model_index;
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
	u32 collision_mesh_index_iter = 0;

	for (u32 i = 0; i < scene->collision_mesh_count; i++)
	{
		u32 current_collision_mesh_index = scene->collision_mesh_index_buffer[i];

		if (scene->entity_buffer[current_collision_mesh_index].entity_binding_index == entity_index)
		{
			collision_mesh_index_iter++;
		}
	}

	return collision_mesh_index_iter;
}

void lightray_get_entity_bound_collision_mesh_indices(const lightray_scene_t* scene, u32 entity_index, u32* collision_mesh_index_buffer)
{
	u32 collision_mesh_index_buffer_iter = 0;

	for (u32 i = 0; i < scene->collision_mesh_count; i++)
	{
		u32 current_collision_mesh_index = scene->collision_mesh_index_buffer[i];

		if (scene->entity_buffer[current_collision_mesh_index].entity_binding_index == entity_index)
		{
			collision_mesh_index_buffer[collision_mesh_index_buffer_iter] = current_collision_mesh_index;
			collision_mesh_index_buffer_iter++;
		}
	}
}

u32 lightray_get_entity_bound_aabb_index_count(const lightray_scene_t* scene, u32 collision_mesh_index)
{
	u32 aabb_index_count_iter = 0;

	for (u32 i = 0; i < scene->aabb_count; i++)
	{
		u32 current_aabb_index = scene->aabb_index_buffer[i];

		if (scene->entity_buffer[current_aabb_index].entity_binding_index == collision_mesh_index)
		{
			aabb_index_count_iter++;
		}
	}

	return aabb_index_count_iter;
}

void lightray_get_entity_bound_aabb_indices(const lightray_scene_t* scene, u32 collision_mesh_index, u32* aabb_index_buffer)
{
	u32 aabb_index_buffer_iter = 0;

	for (u32 i = 0; i < scene->aabb_count; i++)
	{
		u32 current_aabb_index = scene->aabb_index_buffer[i];

		if (scene->entity_buffer[current_aabb_index].entity_binding_index == collision_mesh_index)
		{
			aabb_index_buffer[aabb_index_buffer_iter] = current_aabb_index;
			aabb_index_buffer_iter++;
		}
	}
}