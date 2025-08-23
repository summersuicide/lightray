#include "lightray_core.h"

namespace lightray
{
	void get_shader_byte_code(const std::filesystem::path& shader_path, i8* buffer, u32* buffer_size_in_bytes, u32 byte_code_size_limit)
	{
		std::ifstream file(shader_path, std::ios::ate | std::ios::binary);
		if (!file.is_open()) { return; }
		u64 file_size = (u64)file.tellg();
		if (file_size == 0 || file_size > byte_code_size_limit) { return; }
		(*buffer_size_in_bytes) = (u32)file_size;
		file.seekg(0);
		file.read(buffer, file_size);
		if (file.fail()) { return; }
		file.close();
	}

	std::filesystem::path get_project_root_path()
	{
		char buffer[1024];
		GetModuleFileNameA(NULL, buffer, 1024);
		std::filesystem::path executable_path = std::filesystem::path(buffer).parent_path();
		SUNDER_LOG("\nEXE DIRECTORY");
		SUNDER_LOG(executable_path);

		return executable_path.parent_path().parent_path();
	}

	u32 get_shader_byte_code_size(const std::filesystem::path& shader_path)
	{
		std::ifstream file(shader_path, std::ios::ate | std::ios::binary);
		if (!file.is_open()) { return UINT32_MAX; }
		u64 file_size = (u64)file.tellg();
		if (file.fail()) { return UINT32_MAX; }
		file.close();

		return (u32)file_size;
	}

	void hide_cursor(GLFWwindow* window)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void unhide_cursor(GLFWwindow* window)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	bool should_tick(GLFWwindow* window)
	{
		return glfwWindowShouldClose(window);
	}

	bool is_key_pressed(GLFWwindow* window, i32 key, flags8* key_tick_data)
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

	bool is_key_down(GLFWwindow* window, i32 key)
	{
		return glfwGetKey(window, key) == GLFW_PRESS;
	}

	bool is_mouse_button_pressed(GLFWwindow* window, i32 mouse_button, flags8* key_tick_data)
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

	glm::mat4 construct_projection_matrix(degrees desired_fov, f32 aspect_ratio, f32 near_plane, f32 far_plane)
	{
		glm::mat4 projection{};
		const f32 desired_fov_clamped = glm::clamp(desired_fov, 40.0f, 170.0f);

		projection = glm::perspectiveRH_ZO(glm::radians(desired_fov_clamped), aspect_ratio, near_plane, far_plane);
		projection[1][1] *= -1;

		return projection;
	}

	f32 compute_aspect_ratio(f32 width, f32 height)
	{
		return width / height;
	}

	glm::vec3 to_world_forward_from_euler(const glm::vec3& euler_radians, const glm::vec3& local_forward)
	{
		const glm::quat rotation = glm::quat(euler_radians);
		const glm::vec3 world_forward = rotation * local_forward;

		return glm::normalize(world_forward);
	}

	glm::vec3 to_world_forward_from_model(const glm::mat4& model_matrix, const glm::vec3& local_forward)
	{
		const glm::vec3 normalized_world_forward = glm::normalize(glm::vec3(model_matrix * glm::vec4(local_forward, 0.0f)));

		return normalized_world_forward;
	}

	glm::vec3 get_camera_forward(const glm::mat4& camera_view_matrix)
	{
		return -glm::vec3(camera_view_matrix[0][2], camera_view_matrix[1][2], camera_view_matrix[2][2]);
	}

	glm::vec3 get_camera_right(const glm::mat4& camera_view_matrix)
	{
		return glm::vec3(camera_view_matrix[0][0], camera_view_matrix[1][0], camera_view_matrix[2][0]);
	}

	bool ray_triangle_intersect(const ray_t* ray, const glm::vec3* triangle_vertices, f32* out_t, f32* out_u, f32* out_v)
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
			return false;

		f64 invDet = 1.0 / det;

		f64 u = dot(P, AO) * invDet;
		if (u < 0.0 || u > 1.0)
			return false;

		glm::vec3 Q = cross(AO, AB);
		f64 v = glm::dot(Q, ray->direction) * invDet;
		if (v < 0.0 || u + v > 1.0)
			return false;

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

	buffer_index_query_result_t query_buffer_index(const u32* buffer, u32 starting_index, u32 ending_index, u32 val_at_index, bool reverse_logic)
	{
		buffer_index_query_result_t res;

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

	bool aabbs_intersect(const glm::vec3& position_a, const glm::vec3& scale_a, const glm::vec3& position_b, const glm::vec3& scale_b)
	{
		const glm::vec3 converted_size_a = scale_a / 10.f;
		const glm::vec3 converted_size_b = scale_b / 10.f;

		const glm::vec3 min_a = position_a - converted_size_a;
		const glm::vec3 max_a = position_a + converted_size_a;

		const glm::vec3 min_b = position_b - converted_size_b;
		const glm::vec3 max_b = position_b + converted_size_b;

		return ((min_a.x <= max_b.x) && (max_a.x >= min_b.x) && (min_a.y <= max_b.y) && (max_a.y >= min_b.y) && (min_a.z <= max_b.z) && (max_a.z >= min_b.z));
	}

	void get_raw_vertex_positions(u32 index_buffer_offset, u32 index_count, glm::vec3* buffer, const vertex_t* vertex_buffer, const u32* index_buffer)
	{
		u32 vertex_iter = 0;

		for (u32 i = index_buffer_offset; i < index_buffer_offset + index_count; i++)
		{
			buffer[vertex_iter] = vertex_buffer[index_buffer[i]].position;
			vertex_iter++;
		}
	}

	void get_projected_vertex_positions(const glm::vec3* raw_vertex_positions, glm::vec3* projected_vertex_positions, u64 vertex_count, const model_t& model)
	{
		for (u64 i = 0; i < vertex_count; i++)
		{
			projected_vertex_positions[i] = glm::vec3(model.model * glm::vec4(raw_vertex_positions[i], 1.0f));
		}
	}

	glm::vec3 gjk_find_furthest_point(const glm::vec3* tri, u64 vertex_count, const glm::vec3& direction)
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

	glm::vec3 gjk_support(const glm::vec3* vertex_positions1, u64 vertex_count1, const glm::vec3* vertex_positions2, u64 vertex_count2, const glm::vec3& direction)
	{
		return gjk_find_furthest_point(vertex_positions1, vertex_count1, direction) - gjk_find_furthest_point(vertex_positions2, vertex_count2, -direction);
	}

	bool gjk_same_direction(const glm::vec3& direction, glm::vec3& ao)
	{
		return glm::dot(direction, ao) > 0;
	}

	bool gjk_line(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
	{
		glm::vec3 a = simplex[0];
		glm::vec3 b = simplex[1];
		glm::vec3 ab = b - a;
		glm::vec3 ao = -a;

		if (gjk_same_direction(ab, ao))
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

	bool gjk_triangle(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
	{
		glm::vec3 a = simplex[0];
		glm::vec3 b = simplex[1];
		glm::vec3 c = simplex[2];

		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 ao = -a;

		glm::vec3 abc = glm::cross(ab, ac);

		if (gjk_same_direction(glm::cross(abc, ac), ao))
		{
			if (gjk_same_direction(ac, ao))
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

				return gjk_line(simplex, simplex_size, direction);
			}
		}

		else
		{
			if (gjk_same_direction(glm::cross(ab, abc), ao))
			{
				simplex[0] = a;
				simplex[1] = b;
				*simplex_size = 2;

				return gjk_line(simplex, simplex_size, direction);
			}

			else
			{
				if (gjk_same_direction(abc, ao))
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

	bool gjk_tetrahedron(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
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

		if (gjk_same_direction(abc, ao))
		{
			simplex[0] = a;
			simplex[1] = b;
			simplex[2] = c;

			*simplex_size = 3;

			return gjk_triangle(simplex, simplex_size, direction);
		}

		if (gjk_same_direction(acd, ao))
		{
			simplex[0] = a;
			simplex[1] = c;
			simplex[2] = d;

			*simplex_size = 3;

			return gjk_triangle(simplex, simplex_size, direction);
		}

		if (gjk_same_direction(adb, ao))
		{
			simplex[0] = a;
			simplex[1] = d;
			simplex[2] = b;

			*simplex_size = 3;

			return gjk_triangle(simplex, simplex_size, direction);
		}

		return true;
	}

	bool gjk_next_simplex(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction)
	{
		switch (*simplex_size)
		{
			case 2: return gjk_line(simplex, simplex_size, direction);
				break;
			case 3: return gjk_triangle(simplex, simplex_size, direction);
				break;
			case 4: return gjk_tetrahedron(simplex, simplex_size, direction);
				break;
		}

		return false;
	}

	bool gjk_intersect(const glm::vec3* vertex_positions1, u64 vertex_count1, const glm::vec3* vertex_positions2, u64 vertex_count2)
	{
		glm::vec3 support = gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, glm::vec3(1.0f, 0.0f, 0.0f));

		glm::vec3 simplex[4]{};
		u32 simplex_size = 1;
		simplex[0] = support;

		glm::vec3 direction = -support;

		while (true)
		{
			support = gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, direction);

			if (glm::dot(support, direction) <= 0)
			{
				return false;
			}
			
			simplex[3] = simplex[2];
			simplex[2] = simplex[1];
			simplex[1] = simplex[0];
			simplex[0] = support;

			simplex_size = glm::min(simplex_size + 1, 4U);

			if (gjk_next_simplex(simplex, &simplex_size, &direction))
			{
				return true;
			}
		};
	}

	glm::mat4 assimp_to_glm_mat4(const aiMatrix4x4& mat)
	{
		return glm::mat4(
			mat.a1, mat.b1, mat.c1, mat.d1,
			mat.a2, mat.b2, mat.c2, mat.d2,
			mat.a3, mat.b3, mat.c3, mat.d3,
			mat.a4, mat.b4, mat.c4, mat.d4
		);
	}

	void assimp_get_scene_node_count_helper(aiNode* node, u32* node_count)
	{
		*node_count += node->mNumChildren;

		for (u32 i = 0; i < node->mNumChildren; i++)
		{
			assimp_get_scene_node_count_helper(node->mChildren[i], node_count);
		}
	}

	u32 assimp_get_scene_node_count(aiScene* scene)
	{
		u32 node_count = 0;
		assimp_get_scene_node_count_helper(scene->mRootNode, &node_count);

		return node_count;
	}

	void assimp_get_scene_node_names(aiNode* node, cstring_literal** name_buffer)
	{
		u32 name_buffer_iter = 0;
		name_buffer[name_buffer_iter] = node->mName.C_Str();
		name_buffer_iter++;

		for (u32 i = 0; i < node->mNumChildren; i++)
		{
			assimp_get_scene_node_names(node->mChildren[i], name_buffer);
		}
	}

	void compute_interpolated_bone_positions(aiNodeAnim* animation_channel_buffer, const glm::mat4& parent_transform, glm::mat4* bone_offset_matrix_buffer, f32 delta_time, f64 duration)
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

	u64 generate_guid64()
	{
		u64 guid = ((u64)rand() << 32) | rand();

		return guid;
	}

	u128_t generate_guid128()
	{
		u64 high64 = ((u64)rand() << 32) | rand();
		u64 low64 = ((u64)rand() << 32) | rand();

		u128_t guid;
		guid.high64 = high64;
		guid.low64 = low64;

		return guid;
	}

} // lightray namespace brace