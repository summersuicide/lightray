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

glm::mat4 lightray_construct_perspective_projection_matrix(f32 desired_fov, f32 aspect_ratio, f32 near_plane, f32 far_plane, bool vulkan_y_flip)
{

	glm::mat4 projection{};
	const f32 desired_fov_clamped = glm::clamp(desired_fov, 40.0f, 170.0f);

	projection = glm::perspectiveRH_ZO(glm::radians(desired_fov_clamped), aspect_ratio, near_plane, far_plane);
	std::cout << "-----------------------------------------------------------------------\n";

	if (vulkan_y_flip)
	{
		projection[1][1] *= -1;
	}

	lightray_log_glm_matrix(projection);

	///////////////////////////////////////////////
	const sunder_m4_t pm = sunder_m4_perspective(desired_fov_clamped, aspect_ratio, near_plane, far_plane);
	SUNDER_LOG("\n\n");
	sunder_log_m4(pm);

	/*
	projection[0][0] = pm.x0;
	projection[1][0] = pm.x1;
	projection[2][0] = pm.x2;
	projection[3][0] = pm.x3;

	projection[0][1] = pm.y0;
	projection[1][1] = pm.y1;
	projection[2][1] = pm.y2;
	projection[3][1] = pm.y3;

	projection[0][2] = pm.z0;
	projection[1][2] = pm.z1;
	projection[2][2] = pm.z2;
	projection[3][2] = pm.z3;

	projection[0][3] = pm.w0;
	projection[1][3] = pm.w1;
	projection[2][3] = pm.w2;
	projection[3][3] = pm.w3;
	*/

	projection = lightray_copy_sunder_m4_to_glm(pm);

	lightray_log_glm_matrix(projection);

	///////////////////////////////////////////////

	return projection;
}

glm::mat4 lightray_construct_orthographic_projection_matrix(f32 left, f32 right, f32 bottom, f32 top, f32 near_plane, f32 far_plane, bool vulkan_y_flip)
{
	glm::mat4 projection{};

	if (vulkan_y_flip)
	{
		projection = glm::orthoRH(left, right, top, bottom, near_plane, far_plane);

		SUNDER_LOG("\n\n\n\n");

		lightray_log_glm_matrix(projection);

		SUNDER_LOG("\n\nsunder\n");

		sunder_m4_t pm = sunder_m4_orthographic(left, right, bottom, top, near_plane, far_plane);
		sunder_log_m4(pm);

		projection = lightray_copy_sunder_m4_to_glm(pm);

		lightray_log_glm_matrix(projection);
	}

	else
	{
		projection = glm::orthoRH(left, right, bottom, top, near_plane, far_plane);
	}

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

b32 lightray_ray_triangle_intersect(const lightray_ray_t* ray, const sunder_v3_t* triangle_vertices, f32* out_t, f32* out_u, f32* out_v)
{
	//const f64 EPSILON = 1e-8;

	const sunder_v3_t ray_origin = sunder_v3(ray->origin.x, ray->origin.y, ray->origin.z);
	const sunder_v3_t ray_direction = sunder_v3(ray->direction.x, ray->direction.y, ray->direction.z);

	//sunder_v3_t AB = triangle_vertices[1] - triangle_vertices[0];
	//sunder_v3_t AC = triangle_vertices[2] - triangle_vertices[0];
	////glm::vec3 N = cross(AB, AC); // triangle normal, not gonna use for now

	//sunder_v3_t AO = ray_origin - triangle_vertices[0];
	//sunder_v3_t P = sunder_cross_v3(ray_direction, AC);
	//f64 det = sunder_dot_v3(P, AB);

	//// If determinant is near zero, ray lies in plane or is parallel
	//if (fabs(det) < EPSILON)
	//{
	//	return SUNDER_FALSE;
	//}

	//f64 invDet = 1.0 / det;

	const f64 EPSILON = 1e-8;

	const sunder_v3_t AB = triangle_vertices[1] - triangle_vertices[0];
	const sunder_v3_t AC = triangle_vertices[2] - triangle_vertices[0];
	const sunder_v3_t P = sunder_cross_v3(ray_direction, AC);
	const sunder_v3_t N = sunder_cross_v3(AB, AC);

	const f64 det = sunder_dot_v3(AB, P);

	if (fabs(det) < EPSILON)
	{
		return SUNDER_FALSE;
	}

	const f64 invDet = 1.0f / det;

	const sunder_v3_t T = ray_origin - triangle_vertices[0];

	f64 u = sunder_dot_v3(T, P) * invDet;
	if (u < 0.0 || u > 1.0) return SUNDER_FALSE;

	sunder_v3_t Q = sunder_cross_v3(T, AB);
	f64 v = sunder_dot_v3(ray_direction, Q) * invDet;
	if (v < 0.0 || u + v > 1.0) return SUNDER_FALSE;

	f64 t = sunder_dot_v3(AC, Q) * invDet;

	// Use t along **ray direction**
	if (t < EPSILON) return SUNDER_FALSE;

	const f32 normal_dot_ray = sunder_dot_v3(N, ray_direction);

	if (normal_dot_ray > 0)
	{
		return SUNDER_FALSE;
	}

	*out_t = (f32)t;
	*out_u = (f32)u;
	*out_v = (f32)v;

	return SUNDER_TRUE;

	/*
	f64 u = sunder_dot_v3(P, AO) * invDet;

	if (u < 0.0 || u > 1.0)
	{
		return SUNDER_FALSE;
	}

	sunder_v3_t Q = sunder_cross_v3(AO, AB);
	f64 v = sunder_dot_v3(Q, ray_direction) * invDet;

	if (v < 0.0 || u + v > 1.0)
	{
		return SUNDER_FALSE;
	}

	f64 t = sunder_dot_v3(Q, AC) * invDet;

	if (t < EPSILON) // intersection behind origin
	{
		return SUNDER_FALSE;
	}

	*out_t = (f32)t;
	*out_u = (f32)u;
	*out_v = (f32)v;

	if (t < 0.0f || t > ray->distance)
	{
		return SUNDER_FALSE;
	}

	return SUNDER_TRUE;
	*/
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

void lightray_compute_aabb_min_max(const sunder_v3_t& position, const sunder_v3_t& scale, sunder_v3_t* min, sunder_v3_t* max)
{
	const sunder_v3_t converted_size = scale / 10.0f;
	*min = position - converted_size;
	*max = position + converted_size;
}

b32 lightray_aabbs_intersect(const sunder_v3_t& position_a, const sunder_v3_t& scale_a, const sunder_v3_t& position_b, const sunder_v3_t& scale_b)
{
	//const glm::vec3 converted_size_a = scale_a / 10.f;
	//const glm::vec3 converted_size_b = scale_b / 10.f;

	//const glm::vec3 min_a = position_a - converted_size_a;
	//const glm::vec3 max_a = position_a + converted_size_a;

	//const glm::vec3 min_b = position_b - converted_size_b;
	//const glm::vec3 max_b = position_b + converted_size_b;

	sunder_v3_t min_a{};
	sunder_v3_t max_a{};

	lightray_compute_aabb_min_max(position_a, scale_a, &min_a, &max_a);

	sunder_v3_t min_b{};
	sunder_v3_t max_b{};

	lightray_compute_aabb_min_max(position_b, scale_b, &min_b, &max_b);

	return ((min_a.x <= max_b.x) && (max_a.x >= min_b.x) && (min_a.y <= max_b.y) && (max_a.y >= min_b.y) && (min_a.z <= max_b.z) && (max_a.z >= min_b.z));
}

void lightray_get_raw_vertex_positions(u32 index_buffer_offset, u32 index_count, sunder_v3_t* buffer, const lightray_vertex_t* vertex_buffer, const u32* index_buffer)
{
	u32 vertex_iter = 0;

	for (u32 i = index_buffer_offset; i < index_buffer_offset + index_count; i++)
	{
		buffer[vertex_iter] = sunder_v3(vertex_buffer[index_buffer[i]].position.x, vertex_buffer[index_buffer[i]].position.y, vertex_buffer[index_buffer[i]].position.z);
		vertex_iter++;
	}
}

void lightray_compute_projected_vertex_positions(const sunder_v3_t* raw_vertex_positions, sunder_v3_t* projected_vertex_positions, u32 vertex_count, const lightray_model_t* model)
{
	for (u64 i = 0; i < vertex_count; i++)
	{
		const glm::vec3 v = model->model * glm::vec4(glm::vec3(raw_vertex_positions[i].x, raw_vertex_positions[i].y, raw_vertex_positions[i].z), 1.0f);
		projected_vertex_positions[i] = sunder_v3(v.x, v.y, v.z);
	}
}

sunder_v3_t lightray_gjk_find_furthest_point(const sunder_v3_t* tri, u32 vertex_count, const sunder_v3_t& direction)
{
	sunder_v3_t max_point{};
	f32 max_distance = -FLT_MAX;

	for (u32 i = 0; i < vertex_count; i++)
	{
		const f32 distance = sunder_dot_v3(tri[i], direction);

		if (distance > max_distance)
		{
			max_distance = distance;
			max_point = tri[i];
		}
	}

	return max_point;
}

sunder_v3_t lightray_gjk_support(const sunder_v3_t* vertex_positions1, u32 vertex_count1, const sunder_v3_t* vertex_positions2, u32 vertex_count2, const sunder_v3_t& direction)
{
	return lightray_gjk_find_furthest_point(vertex_positions1, vertex_count1, direction) - lightray_gjk_find_furthest_point(vertex_positions2, vertex_count2, -direction);
}

b32 lightray_gjk_same_direction(const sunder_v3_t& direction, sunder_v3_t& ao)
{
	return sunder_dot_v3(direction, ao) > 0;
}

b32 lightray_gjk_line(sunder_v3_t* simplex, u32* simplex_size, sunder_v3_t* direction)
{
	sunder_v3_t a = simplex[0];
	sunder_v3_t b = simplex[1];
	sunder_v3_t ab = b - a;
	sunder_v3_t ao = -a;

	if (lightray_gjk_same_direction(ab, ao))
	{
		*direction = sunder_cross_v3(sunder_cross_v3(ab, ao), ab);
	}

	else
	{
		simplex[0] = a;
		*simplex_size = 1;
		*direction = ao;
	}

	return SUNDER_FALSE;
}

b32 lightray_gjk_triangle(sunder_v3_t* simplex, u32* simplex_size, sunder_v3_t* direction)
{
	sunder_v3_t a = simplex[0];
	sunder_v3_t b = simplex[1];
	sunder_v3_t c = simplex[2];

	sunder_v3_t ab = b - a;
	sunder_v3_t ac = c - a;
	sunder_v3_t ao = -a;

	sunder_v3_t abc = sunder_cross_v3(ab, ac);

	if (lightray_gjk_same_direction(sunder_cross_v3(abc, ac), ao))
	{
		if (lightray_gjk_same_direction(ac, ao))
		{
			simplex[0] = a;
			simplex[1] = c;
			*simplex_size = 2;

			*direction = sunder_cross_v3(sunder_cross_v3(ac, ao), ac);
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
		if (lightray_gjk_same_direction(sunder_cross_v3(ab, abc), ao))
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

	return SUNDER_FALSE;
}

b32 lightray_gjk_tetrahedron(sunder_v3_t* simplex, u32* simplex_size, sunder_v3_t* direction)
{
	sunder_v3_t a = simplex[0];
	sunder_v3_t b = simplex[1];
	sunder_v3_t c = simplex[2];
	sunder_v3_t d = simplex[3];

	sunder_v3_t ab = b - a;
	sunder_v3_t ac = c - a;
	sunder_v3_t ad = d - a;
	sunder_v3_t ao = -a;

	sunder_v3_t abc = sunder_cross_v3(ab, ac);
	sunder_v3_t acd = sunder_cross_v3(ac, ad);
	sunder_v3_t adb = sunder_cross_v3(ad, ab);

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

	return SUNDER_TRUE;
}

b32 lightray_gjk_next_simplex(sunder_v3_t* simplex, u32* simplex_size, sunder_v3_t* direction)
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

	return SUNDER_FALSE;
}

b32 lightray_gjk_intersect(const sunder_v3_t* vertex_positions1, u32 vertex_count1, const sunder_v3_t* vertex_positions2, u32 vertex_count2)
{
	sunder_v3_t support = lightray_gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, sunder_v3(1.0f, 0.0f, 0.0f));

	sunder_v3_t simplex[4]{};
	u32 simplex_size = 1;
	simplex[0] = support;

	sunder_v3_t direction = -support;

	while (true)
	{
		support = lightray_gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, direction);

		if (sunder_dot_v3(support, direction) <= 0)
		{
			return SUNDER_FALSE;
		}
			
		simplex[3] = simplex[2];
		simplex[2] = simplex[1];
		simplex[1] = simplex[0];
		simplex[0] = support;

		simplex_size = std::min(simplex_size + 1, 4U);

		if (lightray_gjk_next_simplex(simplex, &simplex_size, &direction))
		{
			return SUNDER_TRUE;
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

	scene->total_collidable_entity_count = suballocation_data->collidable_entity_count;
	scene->grid_cell_index_per_collidable_entity_count = suballocation_data->grid_cell_index_per_collidable_entity_count;

	scene->total_grid_cell_index_count = scene->total_collidable_entity_count * scene->grid_cell_index_per_collidable_entity_count;

	scene->index_buffer = suballocation_data->index_buffer;
	scene->vertex_buffer = suballocation_data->vertex_buffer;

	scene->total_collision_mesh_batch_count = scene->total_collidable_entity_count;
	scene->total_collision_mesh_count = scene->total_collidable_entity_count * suballocation_data->collision_mesh_per_batch_count;
	scene->collision_mesh_per_batch_count = suballocation_data->collision_mesh_per_batch_count;

	scene->total_has_already_collided_with_bitmask_count = SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->collidable_entity_count, suballocation_data->collidable_entity_count);
	scene->has_already_collided_with_bitmask_per_collision_attribute_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->collidable_entity_count);

	scene->total_collision_layer_index_count = SUNDER_CAST2(u32)scene->total_collidable_entity_count * 64;

	scene->total_game_side_entity_kind_bitmask_count = SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->game_side_entity_kind_count, suballocation_data->collidable_entity_count);
	scene->game_side_entity_kind_bitmask_per_collision_attribute_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->game_side_entity_kind_count);

	scene->total_raycast_grid_cell_subarena_index_count = suballocation_data->total_raycast_grid_cell_index_subarena_index_count;
	scene->current_raycast_grid_cell_index_subarena_offset = 0;

	scene->total_has_already_been_traced_bitmask_count = SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->collidable_entity_count, suballocation_data->total_per_frame_raycast_count);
	scene->has_already_traced_bitmask_per_raycast_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->collidable_entity_count);

	scene->total_pierce_layer_test_data_count = suballocation_data->total_raycast_pierce_layer_test_data_count;

	const sunder_arena_suballocation_result_t entity_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_entity_t), scene->total_entity_count), alignof(lightray_entity_t));
	scene->entity_buffer = (lightray_entity_t*)entity_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t position_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), scene->total_entity_count), alignof(sunder_v3_t));
	scene->position_buffer = (sunder_v3_t*)position_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t rotation_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), scene->total_entity_count), alignof(sunder_v3_t));
	scene->rotation_buffer = (sunder_v3_t*)rotation_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t scale_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), scene->total_entity_count), alignof(sunder_v3_t));
	scene->scale_buffer = (sunder_v3_t*)scale_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_binding_offsets_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_binding_offsets_t), scene->total_mesh_count), alignof(lightray_mesh_binding_offsets_t));
	scene->mesh_binding_offsets= (lightray_mesh_binding_offsets_t*)mesh_binding_offsets_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_binding_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_binding_t), scene->total_instance_model_count), alignof(lightray_mesh_binding_t));
	scene->mesh_binding_buffer = (lightray_mesh_binding_t*)mesh_binding_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_binding_metadata_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_binding_metadata_t), scene->total_mesh_count), alignof(lightray_mesh_binding_metadata_t));
	scene->mesh_binding_metadata_buffer = (lightray_mesh_binding_metadata_t*)mesh_binding_metadata_buffer_suballocation_result.data;


	const sunder_arena_suballocation_result_t collision_attribute_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_collision_attribute_t), scene->total_collidable_entity_count), alignof(lightray_collision_attribute_t));
	scene->collision_attribute_buffer = (lightray_collision_attribute_t*)collision_attribute_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t grid_cell_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_grid_cell_index_count), alignof(u32));
	scene->grid_cell_index_buffer = (u32*)grid_cell_index_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t collision_mesh_batch_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_collision_mesh_batch_t), scene->total_collision_mesh_batch_count), alignof(lightray_collision_mesh_batch_t));
	scene->collision_mesh_batch_buffer = (lightray_collision_mesh_batch_t*)collision_mesh_batch_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t collision_mesh_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_collision_mesh_t), scene->total_collision_mesh_count), alignof(lightray_collision_mesh_t));
	scene->collision_mesh_buffer = (lightray_collision_mesh_t*)collision_mesh_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t has_already_collided_with_bitmask_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_has_already_collided_with_bitmask_count), alignof(u64));
	scene->has_already_collided_with_bitmask_buffer = (u64*)has_already_collided_with_bitmask_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t collision_layer_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_collision_layer_index_count), alignof(u32));
	scene->collision_layer_index_buffer = (u32*)collision_layer_index_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t game_side_entity_kind_bitmask_buffer_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_game_side_entity_kind_bitmask_count), alignof(u64));
	scene->game_side_entity_kind_bitmask_buffer = (u64*)game_side_entity_kind_bitmask_buffer_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t raycast_grid_cell_index_subarena_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_raycast_grid_cell_subarena_index_count), alignof(u32));
	scene->raycast_grid_cell_index_subarena = SUNDER_CAST(u32*, raycast_grid_cell_index_subarena_suballocation_result.data);

	const sunder_arena_suballocation_result_t has_already_been_traced_bitmask_subarena_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_has_already_been_traced_bitmask_count), alignof(u64));
	scene->has_already_been_traced_bitmask_subarena = SUNDER_CAST(u64*, has_already_been_traced_bitmask_subarena_suballocation_result.data);

	const sunder_arena_suballocation_result_t raycast_pierce_layer_test_data_subarena_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_raycast_pierce_layer_test_data_t), scene->total_pierce_layer_test_data_count), alignof(lightray_raycast_pierce_layer_test_data_t));
	scene->pierce_layer_test_data_subarena = SUNDER_CAST(lightray_raycast_pierce_layer_test_data_t*, raycast_pierce_layer_test_data_subarena_suballocation_result.data);

	const sunder_arena_suballocation_result_t quat_rotation_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_quat_t), scene->total_entity_count), alignof(sunder_quat_t));
	scene->quat_rotation_buffer = SUNDER_CAST(sunder_quat_t*, quat_rotation_buffer_suballocation_result.data);

	for (u32 i = 0; i < scene->total_entity_count; i++)
	{
		scene->scale_buffer[i] = sunder_v3_scalar(1.0f);
	}

	for (u32 i = 0; i < scene->total_entity_count; i++)
	{
		SUNDER_SET_BIT(scene->visibility_flags, i, 1ull);
	}

	for (u32 i = 0; i < scene->total_instance_model_count; i++)
	{
		scene->mesh_binding_buffer[i].bone_node_mapping_index = UINT32_MAX;
	}

}

u64 lightray_compute_scene_suballocation_size(const lightray_scene_suballocation_data_t* suballocation_data, u64 alignment)
{
	const u64 entity_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_entity_t), suballocation_data->total_entity_count, alignment);
	const u64 transform_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(glm::vec3), suballocation_data->total_entity_count * 3, alignment);
	const u64 mesh_binding_offsets_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_offsets_t), suballocation_data->total_mesh_count, alignment);
	const u64 mesh_binding_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_t), suballocation_data->total_instance_model_count, alignment);
	const u64 mesh_binding_metadata_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_metadata_t), suballocation_data->total_mesh_count, alignment);

	const u64 collision_attribute_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_attribute_t), suballocation_data->collidable_entity_count, alignment);
	const u64 grid_cell_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collidable_entity_count * suballocation_data->grid_cell_index_per_collidable_entity_count, alignment);
	const u64 collision_mesh_batch_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_mesh_batch_t), suballocation_data->collidable_entity_count, alignment);
	const u64 collision_mesh_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_mesh_t), suballocation_data->collidable_entity_count * suballocation_data->collision_mesh_per_batch_count, alignment);
	const u64 has_already_collided_with_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64),  SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->collidable_entity_count, suballocation_data->collidable_entity_count), alignment);
	const u64 collision_layer_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collidable_entity_count * 64, alignment);
	const u64 game_side_entity_kind_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64),  SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->game_side_entity_kind_count, suballocation_data->collidable_entity_count), alignment);

	const u64 raycast_grid_cell_index_subarena_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_raycast_grid_cell_index_subarena_index_count, alignment);
	const u64 has_already_been_traced_bitmask_subarena_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->collidable_entity_count, suballocation_data->total_per_frame_raycast_count), alignment);
	const u64 raycast_pierce_layer_test_data_subarena_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_raycast_pierce_layer_test_data_t), suballocation_data->total_raycast_pierce_layer_test_data_count, alignment);

	const u64 quat_rotation_buffer = sunder_compute_aligned_allocation_size(sizeof(sunder_quat_t), suballocation_data->total_entity_count, alignment);

	return  sunder_align64(entity_buffer_suballocation_size + transform_buffer_suballocation_size + mesh_binding_offsets_suballocation_size + mesh_binding_buffer_suballocation_size + mesh_binding_metadata_buffer_suballocation_size + grid_cell_index_buffer_suballocation_size + collision_attribute_buffer_suballocation_size + collision_mesh_batch_buffer_suballocation_size + collision_mesh_buffer_suballocation_size + has_already_collided_with_bitmask_buffer_suballocation_size + collision_layer_index_buffer_suballocation_size + game_side_entity_kind_bitmask_buffer_suballocation_size + raycast_grid_cell_index_subarena_suballocation_size + has_already_been_traced_bitmask_subarena_suballocation_size + raycast_pierce_layer_test_data_subarena_suballocation_size + quat_rotation_buffer, alignment);
}

lightray_entity_creation_result_t lightray_create_entity(lightray_scene_t* scene, lightray_entity_kind kind, u32 flags)
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
	scene->entity_buffer[scene->current_entity_count].flags = flags;
	scene->current_entity_count++;

	return res;
}

lightray_mesh_binding_result_t lightray_bind_mesh(lightray_scene_t* scene, u32 entity_index, u32 mesh_index, lightray_render_target_kind kind)
{
	lightray_mesh_binding_result_t res{};
	res.mesh_binding_index = UINT32_MAX;
	res.result = LIGHTRAY_RESULT_INVALID_ENTITY_INDEX;

	if (entity_index > scene->total_entity_count - 1)
	{
		return res;
	}

	res.result = LIGHTRAY_RESULT_INVALID_MESH_INDEX;

	if (mesh_index > scene->total_mesh_count - 1)
	{
		return res;
	}

	res.result = LIGHTRAY_RESULT_MESH_BINDING_OVERFLOW;

	if (scene->mesh_binding_count == scene->total_instance_model_count)
	{
		return res;
	}

	res.result = LIGHTRAY_RESULT_OPAQUE_MESH_BINDING_COUNT_HAS_BEEN_EXCEEDED;

	if (kind == LIGHTRAY_RENDER_TARGET_KIND_OPAQUE_MESH)
	{
		if (scene->mesh_binding_metadata_buffer[mesh_index].current_opaque_instance_binding_count == scene->mesh_binding_metadata_buffer[mesh_index].opaque_instance_count)
		{
			return res;
		}
	}

	res.result = LIGHTRAY_RESULT_WIREFRAME_MESH_BINDING_COUNT_HAS_BEEN_EXCEEDED;

	if (kind == LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH)
	{
		if (scene->mesh_binding_metadata_buffer[mesh_index].current_wireframe_instance_binding_count == scene->mesh_binding_metadata_buffer[mesh_index].wireframe_instance_count)
		{
			return res;
		}
	}

	scene->entity_buffer[entity_index].mesh_binding_index = mesh_index;
	scene->mesh_binding_buffer[scene->mesh_binding_count].transform_index = entity_index;

	if (kind == LIGHTRAY_RENDER_TARGET_KIND_OPAQUE_MESH)
	{
		scene->mesh_binding_buffer[scene->mesh_binding_count].instance_model_index = scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index;
		scene->entity_buffer[entity_index].instance_model_binding_index = scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index;
		scene->mesh_binding_metadata_buffer[mesh_index].current_opaque_instance_binding_count++;
	}

	else if (kind == LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH)
	{
		scene->mesh_binding_buffer[scene->mesh_binding_count].instance_model_index = scene->mesh_binding_offsets[mesh_index].current_wireframe_instance_model_index;
		scene->entity_buffer[entity_index].instance_model_binding_index = scene->mesh_binding_offsets[mesh_index].current_wireframe_instance_model_index;
		scene->mesh_binding_metadata_buffer[mesh_index].current_wireframe_instance_binding_count++;
	}

	res.mesh_binding_index = scene->mesh_binding_count;
	scene->mesh_binding_count++;

	if (kind == LIGHTRAY_RENDER_TARGET_KIND_OPAQUE_MESH)
	{
		if (scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index != scene->mesh_binding_offsets[mesh_index].last_opaque_instance_model_index)
		{
			scene->mesh_binding_offsets[mesh_index].current_opaque_instance_model_index++;
		}
	}

	else if (kind == LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH)
	{
		if (scene->mesh_binding_offsets[mesh_index].current_wireframe_instance_model_index != scene->mesh_binding_offsets[mesh_index].last_wireframe_instance_model_index)
		{
			scene->mesh_binding_offsets[mesh_index].current_wireframe_instance_model_index++;
		}
	}

	res.result = LIGHTRAY_RESULT_SUCCESS;

	return res;
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
	if (!sunder_valid_index(entity_to_bind_index, scene->total_entity_count) || !sunder_valid_index(entity_to_bind_to_index, scene->total_entity_count))
	{
		return;
	}

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

void lightray_compute_interpolated_skeleton_transform(lightray_animation_core_t* animation_core, u32 animation_index, u32 skeleton_index, u32 instance_index, u32 animation_playback_index)
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

			const glm::vec3 scale_key = lightray_compute_interpolated_animation_channel_scale_key(current_animation_channel, animation_core->animation_scale_key_buffer, animation_core->playback_command_buffer[animation_playback_index].ticks);
			const glm::quat rotation_key = lightray_compute_interpolated_animation_channel_rotation_key(current_animation_channel, animation_core->animation_rotation_key_buffer, animation_core->playback_command_buffer[animation_playback_index].ticks);
			const glm::vec3 position_key = lightray_compute_interpolated_animation_channel_position_key(current_animation_channel, animation_core->animation_position_key_buffer, animation_core->playback_command_buffer[animation_playback_index].ticks);

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

void lightray_assimp_execute_second_node_buffer_population_pass(const aiNode* node, lightray_node_t* node_buffer, u32 node_count, const sunder_string_t* names, u32* current_index, u32 node_buffer_offset)
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
			const bool parent_node_names_match = sunder_compare_strings(names[i + node_buffer_offset].data, names[i + node_buffer_offset].length, parent_node_name, parent_node_name_length);

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
		lightray_assimp_execute_second_node_buffer_population_pass(node->mChildren[i], node_buffer, node_count, names, current_index, node_buffer_offset);
	}
}

void lightray_log_mesh_binding_offset_buffer(const lightray_scene_t* scene)
{
	SUNDER_LOG("\n\nmesh binding offsets\n");
	for (u32 i = 0; i < scene->total_mesh_count; i++)
	{
		SUNDER_LOG("\nopaque: ");
		SUNDER_LOG(scene->mesh_binding_offsets[i].current_opaque_instance_model_index);
		SUNDER_LOG(" ");
		SUNDER_LOG(scene->mesh_binding_offsets[i].last_opaque_instance_model_index);
		SUNDER_LOG("\nwireframe: ");
		SUNDER_LOG(scene->mesh_binding_offsets[i].current_wireframe_instance_model_index);
		SUNDER_LOG(" ");
		SUNDER_LOG(scene->mesh_binding_offsets[i].last_wireframe_instance_model_index);
		SUNDER_LOG("\n\n");
	}
}

void lightray_log_mesh_binding_metadata_buffer(const lightray_scene_t* scene)
{
	SUNDER_LOG("\n\nmesh binding metadata buffer\n");
	for (u32 i = 0; i < scene->total_mesh_count; i++)
	{
		SUNDER_LOG("current_opaque_instance_binding_count/opaque_instance_count: ");
		SUNDER_LOG(scene->mesh_binding_metadata_buffer[i].current_opaque_instance_binding_count);
		SUNDER_LOG("/");
		SUNDER_LOG(scene->mesh_binding_metadata_buffer[i].opaque_instance_count);

		SUNDER_LOG("\ncurrent_wireframe_instance_binding_count/wireframe_instance_count: ");
		SUNDER_LOG(scene->mesh_binding_metadata_buffer[i].current_wireframe_instance_binding_count);
		SUNDER_LOG("/");
		SUNDER_LOG(scene->mesh_binding_metadata_buffer[i].wireframe_instance_count);

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

void lightray_load_json_garbage()
{
	FILE* file = nullptr;
	fopen_s(&file, "resources/ShareTech-Regular_layout.json", "rb");

	if (file != nullptr)
	{
		fseek(file, 0, SEEK_END);

		const u64 file_size = ftell(file);
		rewind(file);

		i8* buffer = (i8*)sunder_aligned_halloc(file_size + 1, 8);

		const u64 bytes_read = fread(buffer, 1, file_size, file);
		buffer[file_size] = 0;

		fclose(file);

		cJSON* root = cJSON_Parse(buffer);

		const cJSON* atlas = cJSON_GetObjectItem(root, "atlas");
		const cJSON* metrics = cJSON_GetObjectItem(root, "metrics");
		const cJSON* glyphs = cJSON_GetObjectItem(root, "glyphs");

		const i32 size = cJSON_GetObjectItem(atlas, "size")->valueint;
		const i32 width = cJSON_GetObjectItem(atlas, "width")->valueint;
		const i32 height = cJSON_GetObjectItem(atlas, "height")->valueint;

		const i32 em_size = cJSON_GetObjectItem(metrics, "emSize")->valueint;
		const f64 base_line_height = cJSON_GetObjectItem(metrics, "lineHeight")->valuedouble;
		const f64 ascender = cJSON_GetObjectItem(metrics, "ascender")->valuedouble;
		const f64 descender = cJSON_GetObjectItem(metrics, "descender")->valuedouble;

		const i32 glyph_count = cJSON_GetArraySize(glyphs);

		SUNDER_LOG("\n=======================================");

		for (i32 i = 0; i < glyph_count; i++)
		{
			const cJSON* glyph = cJSON_GetArrayItem(glyphs, i);
			const i32 index = cJSON_GetObjectItem(glyph, "index")->valueint;
			const f64 advance = cJSON_GetObjectItem(glyph, "advance")->valuedouble;

			const cJSON* plane_bounds = cJSON_GetObjectItem(glyph, "planeBounds");
			const cJSON* atlas_bounds = cJSON_GetObjectItem(glyph, "atlasBounds");

			SUNDER_LOG("\nindex: ");
			SUNDER_LOG(index);
			SUNDER_LOG("\nadvance: ");
			SUNDER_LOG(advance);

			if (plane_bounds != nullptr)
			{
				const f64 plane_bounds_left = cJSON_GetObjectItem(plane_bounds, "left")->valuedouble;
				const f64 plane_bounds_right = cJSON_GetObjectItem(plane_bounds, "right")->valuedouble;
				const f64 plane_bounds_top = cJSON_GetObjectItem(plane_bounds, "top")->valuedouble;
				const f64 plane_bounds_bottom = cJSON_GetObjectItem(plane_bounds, "bottom")->valuedouble;

				SUNDER_LOG("\n\nplane_bounds ");
				SUNDER_LOG("\nleft: ");
				SUNDER_LOG(plane_bounds_left);
				SUNDER_LOG("\nright: ");
				SUNDER_LOG(plane_bounds_right);
				SUNDER_LOG("\ntop: ");
				SUNDER_LOG(plane_bounds_top);
				SUNDER_LOG("\nbottom: ");
				SUNDER_LOG(plane_bounds_bottom);
			}

			if (atlas_bounds != nullptr)
			{
				const f64 atlas_bounds_left = cJSON_GetObjectItem(atlas_bounds, "left")->valuedouble;
				const f64 atlas_bounds_right = cJSON_GetObjectItem(atlas_bounds, "right")->valuedouble;
				const f64 atlas_bounds_top = cJSON_GetObjectItem(atlas_bounds, "top")->valuedouble;
				const f64 atlas_bounds_bottom = cJSON_GetObjectItem(atlas_bounds, "bottom")->valuedouble;

				SUNDER_LOG("\n\natlas_bounds ");
				SUNDER_LOG("\nleft: ");
				SUNDER_LOG(atlas_bounds_left);
				SUNDER_LOG("\nright: ");
				SUNDER_LOG(atlas_bounds_right);
				SUNDER_LOG("\ntop: ");
				SUNDER_LOG(atlas_bounds_top);
				SUNDER_LOG("\nbottom: ");
				SUNDER_LOG(atlas_bounds_bottom);
			}

			SUNDER_LOG("\n=======================================\n");
		}

		cJSON_Delete(root);
		sunder_aligned_free(SUNDER_PRE_FREE_CAST(buffer));
	}
}

i32 lightray_get_file_size(cstring_literal* path)
{
	struct _stat st {};
	const i32 st_file_result = _stat(path, &st);
	return st.st_size;
}

f32 lightray_get_default_cube_grid_scale_x(const lightray_grid_t* grid)
{
	return grid->cell_width / LIGHTRAY_DEFAULT_CUBE_SCALE_FACTOR;
}

f32 lightray_get_default_cube_grid_scale_y(const lightray_grid_t* grid)
{
	return grid->cell_height / LIGHTRAY_DEFAULT_CUBE_SCALE_FACTOR;
}

void lightray_create_grid(lightray_grid_t* grid, const lightray_grid_creation_data_t* creation_data)
{
	grid->column_count = creation_data->column_count;
	grid->row_count = creation_data->row_count;
	grid->cell_count = creation_data->column_count * creation_data->row_count;
	grid->cell_width = creation_data->cell_width;
	grid->cell_height = creation_data->cell_height;
	grid->origin = creation_data->origin;
	grid->collision_attribute_index_count = grid->cell_count * creation_data->collidable_entity_per_cell_count;
	grid->collision_attribute_index_count_per_cell = creation_data->collidable_entity_per_cell_count;

	sunder_arena_suballocation_result_t cell_buffer_suballocation_result = sunder_suballocate_from_arena_debug(creation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_grid_cell_t), grid->cell_count), alignof(lightray_grid_cell_t));
	grid->cell_buffer = SUNDER_CAST(lightray_grid_cell_t*, cell_buffer_suballocation_result.data);

	sunder_arena_suballocation_result_t collision_attribute_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(creation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), grid->collision_attribute_index_count), alignof(u32));
	grid->collision_attribute_index_buffer = SUNDER_CAST(u32*, collision_attribute_index_buffer_suballocation_result.data);

	u32 grid_column_index = creation_data->first_column_entity_index;
	u32 grid_row_index = creation_data->first_row_entity_index;
	const f32 cube_length = LIGHTRAY_DEFAULT_CUBE_SCALE_FACTOR;
	const f32 scale_fix_for_columns = (grid->row_count * grid->cell_height) / cube_length;
	const f32 scale_fix_for_rows = (grid->column_count * grid->cell_width) / cube_length;
	const f32 default_cube_scale_fix_for_columns = grid->cell_height / cube_length;
	const f32 default_cube_scale_fix_for_rows = grid->cell_width / cube_length;

	for (u32 col = 0; col < grid->column_count + 1; col++)
	{
		sunder_v3_t pos = grid->origin + sunder_v3(
			col * grid->cell_width,           // X position along columns
			(grid->row_count * grid->cell_height) / 2.0f, // Y centered
			0.0f                       // Z at floor level
		);

		sunder_v3_t scale = sunder_v3(
			0.05f,                 // thin in X (width)
			scale_fix_for_columns,        // stretched along Y (length)
			0.01f                  // thin in Z (height)
		);

		creation_data->scene->position_buffer[grid_column_index] = pos;
		creation_data->scene->scale_buffer[grid_column_index] = scale;
		grid_column_index++;
	}

	for (u32 row = 0; row < grid->row_count + 1; row++)
	{
		sunder_v3_t pos = grid->origin + sunder_v3(
			(grid->column_count * grid->cell_width) / 2.0f, // X centered
			row * grid->cell_height,          // Y varies per row
			0.0f                       // Z at floor level
		);

		sunder_v3_t scale = sunder_v3(
			scale_fix_for_rows,         // stretched along X (length)
			0.05f,                 // thin in Y (width)
			0.01f                  // thin in Z (height)
		);

		creation_data->scene->position_buffer[grid_row_index] = pos;
		creation_data->scene->scale_buffer[grid_row_index] = scale;
		grid_row_index++;
	}

	for (u32 i = 0; i < grid->cell_count; i++)
	{
		grid->cell_buffer[i].collision_attribute_index_buffer_offset = i * creation_data->collidable_entity_per_cell_count;
	}


	// old way in case i fuck up something

	//const f32 thickness = 0.05f;
//const f32 scale_fix = 5.015f;

//const u32 rows = 4u;
//const u32 cols = 4u;
//const f32 cell_width = 1.0f;
//const f32 cell_height = 1.0f;
//const glm::vec3 grid_origin = glm::vec3(0.0f, 0.0f, -0.2f);
//	
//const f32 scale_fix_for_columns = (rows * cell_height) / 0.1996f;
//const f32 scale_fix_for_rows = (cols * cell_width) / 0.2f;

//for (u32 col = 0; col < cols + 1; col++)
//{
//	glm::vec3 pos = grid_origin + glm::vec3(
//		col * cell_width,           // X position along columns
//		(rows * cell_height) / 2.0f, // Y centered
//		0.0f                       // Z at floor level
//	);

//	//glm::vec3 scale = glm::vec3(
//	//	0.05f,                 // thin in X (width)
//	//	rows * cell_height * scale_fix,        // stretched along Y (length)
//	//	0.01f                  // thin in Z (height)
//	//);

//	glm::vec3 scale = glm::vec3(
//		0.05f,                 // thin in X (width)
//		rows * cell_height / 0.1996f * 0.1996f,        // stretched along Y (length)
//		0.01f                  // thin in Z (height)
//	);

//	scale.y = scale_fix_for_columns;

//	scene.position_buffer[grid_col_index] = pos;
//	scene.scale_buffer[grid_col_index] = scale;
//	grid_col_index++;
//}

//for (u32 row = 0; row < rows + 1; row++)
//{
//	glm::vec3 pos = grid_origin + glm::vec3(
//	 (cols * cell_width) / 2.0f, // X centered
//		row * cell_height,          // Y varies per row
//		0.0f                       // Z at floor level
//	);

//	glm::vec3 scale = glm::vec3(
//		cols * cell_width * scale_fix,         // stretched along X (length)
//		0.05f,                 // thin in Y (width)
//		0.01f                  // thin in Z (height)
//	);

//	scene.position_buffer[grid_row_index] = pos;
//	scene.scale_buffer[grid_row_index] = scale;
//	grid_row_index++;
//}
}

b32 lightray_get_grid_cell_coordinates(const lightray_grid_t* grid, const sunder_v3_t* world_position, u32* row_index, u32* column_index)
{
	const sunder_v3_t local_pos = (*world_position) - grid->origin;

	const i32 col = grid->column_count - 1 - (i32)floor(local_pos.x / grid->cell_width); // inverting the thing so that columns are counter from the opposite corner (top left, instead of top right)
	const i32 row = (i32)floor(local_pos.y / grid->cell_height);

	if (col < 0 || col >= (i32)grid->column_count || row < 0 || row >= (i32)grid->row_count)
	{
		return SUNDER_FALSE;
	}

	*column_index = col;
	*row_index = row;

	return SUNDER_TRUE;
}

void lightray_get_grid_cell_coordinates_aabb(const lightray_grid_t* grid, const sunder_v3_t* aabb_position, const sunder_v3_t* aabb_scale, lightray_grid_cell_aabb_coordinates_t* row_coordinates, lightray_grid_cell_aabb_coordinates_t* column_coordinates)
{
	sunder_v3_t aabb_min{};
	sunder_v3_t aabb_max{};
	
	lightray_compute_aabb_min_max(*aabb_position, *aabb_scale, &aabb_min, &aabb_max);

	const sunder_v3_t local_aabb_min = aabb_min - grid->origin;
	const sunder_v3_t local_aabb_max = aabb_max - grid->origin;

	i32 min_column = grid->column_count - 1 - (i32)floor(local_aabb_min.x / grid->cell_width);
	i32 max_column = grid->column_count - 1 - (i32)floor(local_aabb_max.x / grid->cell_width);

	//min_column = grid->column_count - 1 - min_column;
	//max_column = grid->column_count - 1 - max_column;

	if (min_column > max_column)
	{
		const i32 column_temp = min_column;
		min_column = max_column;
		max_column = column_temp;
	}

	const i32 min_row = (i32)floor(local_aabb_min.y / grid->cell_height);
	const i32 max_row = (i32)floor(local_aabb_max.y / grid->cell_height);

	row_coordinates->min = min_row;
	row_coordinates->max = max_row;
	column_coordinates->min = min_column;
	column_coordinates->max = max_column;
}

u32 lightray_push_collision_attribute(lightray_scene_t* scene)
{
	u32 attribute_index = UINT32_MAX;

	if (sunder_valid_index(scene->collision_attribute_current_count, scene->total_collidable_entity_count))
	{
		attribute_index = scene->collision_attribute_current_count;
		scene->collision_attribute_buffer[attribute_index].game_side_entity_kind_bitmask_buffer_offset = scene->game_side_entity_kind_bitmask_per_collision_attribute_count * attribute_index;
		scene->collision_attribute_buffer[attribute_index].grid_cell_index_buffer_offset = scene->grid_cell_index_per_collidable_entity_count * attribute_index;
		scene->collision_attribute_buffer[attribute_index].has_already_collided_with_bitmask_buffer_offset = scene->has_already_collided_with_bitmask_per_collision_attribute_count * attribute_index;
		scene->collision_attribute_buffer[attribute_index].collision_layer_index_buffer_offset = 64 * attribute_index;
		scene->collision_attribute_current_count++;
	}

	return attribute_index;
}

void lightray_bind_aabb(lightray_scene_t* scene, u32 collidable_entity_index, u32 aabb_index, u32 collision_attribute_index)
{
	if (!sunder_valid_index(collision_attribute_index, scene->total_collidable_entity_count))
	{
		return;
	}

	const lightray_entity_kind collidable_entity_kind = scene->entity_buffer[collidable_entity_index].kind;
	const lightray_entity_kind aabb_entity_kind = scene->entity_buffer[aabb_index].kind;

	if (collidable_entity_kind == LIGHTRAY_ENTITY_KIND_AABB || collidable_entity_kind == LIGHTRAY_ENTITY_KIND_COLLISION_MESH)
	{
		return;
	}

	if (aabb_entity_kind != LIGHTRAY_ENTITY_KIND_AABB)
	{
		return;
	}

	scene->collision_attribute_buffer[collision_attribute_index].aabb_index = aabb_index;
	lightray_bind_entity(scene, aabb_index, collidable_entity_index);
}

u32 lightray_get_grid_cell_index(const lightray_grid_t* grid, u32 row_index, u32 column_index)
{
	// cell_index = (row_index * column_count) + column_index

	return (row_index * grid->column_count) + column_index;
}

void lightray_log_glm_matrix(const glm::mat4& m)
{
	SUNDER_LOG("\n");
	SUNDER_LOG(m[0][0]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[1][0]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[2][0]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[3][0]);

	SUNDER_LOG("\n");
	SUNDER_LOG(m[0][1]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[1][1]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[2][1]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[3][1]);

	SUNDER_LOG("\n");
	SUNDER_LOG(m[0][2]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[1][2]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[2][2]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[3][2]);

	SUNDER_LOG("\n");
	SUNDER_LOG(m[0][3]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[1][3]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[2][3]);
	SUNDER_LOG(" | ");
	SUNDER_LOG(m[3][3]);
}

glm::mat4 lightray_copy_sunder_m4_to_glm(const sunder_m4_t& m)
{
	glm::mat4 cm = glm::mat4(glm::vec4(m.x0, m.y0, m.z0, m.w0),
											glm::vec4(m.x1, m.y1, m.z1, m.w1),
											glm::vec4(m.x2, m.y2, m.z2, m.w2),
											glm::vec4(m.x3, m.y3, m.z3, m.w3));

	return cm;
}

sunder_m4_t lightray_copy_glm_mat4_to_sunder(const glm::mat4& m)
{
	sunder_m4_t result{};
	f32* sunder_mat = &result.x0;
	const f32* glm_mat  = &m[0][0];
	
	for (u32 i = 0; i < 16; i++)
	{
		sunder_mat[i] = glm_mat[i];
	}

	return result;
}

void lightray_bind_position(lightray_scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index, const sunder_v3_t& relative_offset)
{

}

u32 lightray_push_collision_mesh_batch(lightray_scene_t* scene)
{
	u32 batch_index = UINT32_MAX;

	if (sunder_valid_index(scene->current_collision_mesh_batch_count, scene->total_collision_mesh_batch_count))
	{
		batch_index = scene->current_collision_mesh_batch_count;
		scene->collision_mesh_batch_buffer[batch_index].collision_mesh_buffer_offset = batch_index * scene->collision_mesh_per_batch_count;
		scene->current_collision_mesh_batch_count++;
	}

	return batch_index;
}

u32 lightray_push_collision_mesh(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_mesh_index, u32 vertex_count, u32 index_buffer_offset, const lightray_model_t* model)
{
	const u32 collision_mesh_count = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count;

	u32 return_collision_mesh_index = UINT32_MAX;

	if (sunder_valid_index(collision_mesh_count, scene->collision_mesh_per_batch_count))
	{
		const u32 collision_mesh_buffer_index = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset + collision_mesh_count;
		scene->collision_mesh_buffer[collision_mesh_buffer_index].vertex_count = vertex_count;
		scene->collision_mesh_buffer[collision_mesh_buffer_index].index_buffer_offset = index_buffer_offset;
		scene->collision_mesh_buffer[collision_mesh_buffer_index].model = model;
		scene->collision_mesh_buffer[collision_mesh_buffer_index].entity_index = collision_mesh_index;
		scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count++;
		return_collision_mesh_index = collision_mesh_count;
	}

	return return_collision_mesh_index;
}

void lightray_bind_collision_mesh_batch(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_attribute_index)
{
	scene->collision_attribute_buffer[collision_attribute_index].collision_mesh_batch_index = collision_mesh_batch_index;
}

void lightray_allocate_collision_mesh_vertex_buffers(lightray_scene_t* scene)
{
	const u32 total_collision_mesh_batch_count = scene->total_collision_mesh_batch_count;
	u64 total_vertex_buffers_allocation_size = 0;

	for (u32 i = 0; i < total_collision_mesh_batch_count; i++)
	{
		const u32 collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[i].collision_mesh_buffer_offset;
		const u32 total_collision_mesh_count_in_a_batch = scene->collision_mesh_batch_buffer[i].collision_mesh_count;

		for (u32 j = 0; j < total_collision_mesh_count_in_a_batch; j++)
		{
			total_vertex_buffers_allocation_size += sunder_compute_aligned_allocation_size(sizeof(sunder_v3_t), scene->collision_mesh_buffer[collision_mesh_buffer_offset + j].vertex_count * 2, alignof(u64));
			total_vertex_buffers_allocation_size = sunder_align64(total_vertex_buffers_allocation_size, alignof(u64));
		}
	}

	const sunder_arena_result arena_allocation_result = sunder_allocate_arena(&scene->collision_mesh_vertex_buffers_arena, total_vertex_buffers_allocation_size, alignof(u64));

	for (u32 i = 0; i < total_collision_mesh_batch_count; i++)
	{
		const u32 collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[i].collision_mesh_buffer_offset;
		const u32 total_collision_mesh_count_in_a_batch = scene->collision_mesh_batch_buffer[i].collision_mesh_count;

		for (u32 j = 0; j < total_collision_mesh_count_in_a_batch; j++)
		{
			const u64 vertex_buffer_suballocation_size = sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), scene->collision_mesh_buffer[collision_mesh_buffer_offset + j].vertex_count);

			const sunder_arena_suballocation_result_t raw_vertex_position_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, vertex_buffer_suballocation_size, alignof(sunder_v3_t));
			scene->collision_mesh_buffer[collision_mesh_buffer_offset + j].raw_vertex_position_buffer = SUNDER_CAST(sunder_v3_t*, raw_vertex_position_buffer_suballocation_result.data);

			const sunder_arena_suballocation_result_t projected_vertex_position_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, vertex_buffer_suballocation_size, alignof(sunder_v3_t));
			scene->collision_mesh_buffer[collision_mesh_buffer_offset + j].projected_vertex_position_buffer = SUNDER_CAST(sunder_v3_t*, projected_vertex_position_buffer_suballocation_result.data);
		}
	}
}

void lightray_free_collision_mesh_vertex_buffers(lightray_scene_t* scene)
{
	sunder_free_arena(&scene->collision_mesh_vertex_buffers_arena);
}

void lightray_initialize_collision_mesh_raw_vertex_buffer(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_mesh_index)
{
	const u32 collision_mesh_buffer_index = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset + collision_mesh_index;

	lightray_get_raw_vertex_positions(scene->collision_mesh_buffer[collision_mesh_buffer_index].index_buffer_offset, scene->collision_mesh_buffer[collision_mesh_buffer_index].vertex_count, scene->collision_mesh_buffer[collision_mesh_buffer_index].raw_vertex_position_buffer, scene->vertex_buffer, scene->index_buffer);
}

void lightray_initialize_collision_mesh_projected_vertex_buffer(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_mesh_index)
{
	const u32 collision_mesh_buffer_index = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset + collision_mesh_index;

	lightray_compute_projected_vertex_positions(scene->collision_mesh_buffer[collision_mesh_buffer_index].raw_vertex_position_buffer, scene->collision_mesh_buffer[collision_mesh_buffer_index].projected_vertex_position_buffer, scene->collision_mesh_buffer[collision_mesh_buffer_index].vertex_count, scene->collision_mesh_buffer[collision_mesh_buffer_index].model);
}

void lightray_add_collision_layer(lightray_scene_t* scene, u32 collision_attribute_index, u64 collision_layer)
{
	SUNDER_SET_BIT(scene->collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask, collision_layer, 1ull);
	const u32 index_buffer_offset = scene->collision_attribute_buffer[collision_attribute_index].collision_layer_index_buffer_offset;
	scene->collision_layer_index_buffer[index_buffer_offset + scene->collision_attribute_buffer[collision_attribute_index].collision_layer_count] = SUNDER_CAST2(u32)collision_layer;
	scene->collision_attribute_buffer[collision_attribute_index].collision_layer_count++;
}

void lightray_remove_collision_layer(lightray_scene_t* scene, u32 collision_attribute_index, u64 collision_layer)
{
	SUNDER_ZERO_BIT(scene->collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask, collision_layer, 1ull);
	const u32 index_buffer_offset = scene->collision_attribute_buffer[collision_attribute_index].collision_layer_index_buffer_offset;
	const u32 last_index = scene->collision_attribute_buffer[collision_attribute_index].collision_layer_count - 1;
	u32 swap_index = 0;

	for (u32 i = 0; i < last_index + 1; i++)
	{
		if (scene->collision_layer_index_buffer[index_buffer_offset + i] == SUNDER_CAST2(u32)collision_layer)
		{
			swap_index = i;
			break;
		}
	}

	scene->collision_layer_index_buffer[index_buffer_offset + swap_index] = scene->collision_layer_index_buffer[index_buffer_offset + last_index];
	scene->collision_attribute_buffer[collision_attribute_index].collision_layer_count--;
}

void lightray_add_collision_layer_exception(lightray_scene_t* scene, u32 collision_attribute_index, u64 exception_layer)
{
	//scene->collision_attribute_buffer[collision_attribute_index].exception_collision_layer_bitmask |= exception_layer;
}

void lightray_remove_collision_layer_exception(lightray_scene_t* scene, u32 collision_attribute_index, u64 exception_layer)
{
	//scene->collision_attribute_buffer[collision_attribute_index].exception_collision_layer_bitmask &= (~exception_layer);
}

void lightray_enable_collision(lightray_scene_t* scene, u32 collision_attribute_index)
{
	SUNDER_SET_BIT(scene->collision_attribute_buffer[collision_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_COLLIDED_WITH_BIT, 1u);
}

void lightray_disable_collision(lightray_scene_t* scene, u32 collision_attribute_index)
{
	SUNDER_ZERO_BIT(scene->collision_attribute_buffer[collision_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_COLLIDED_WITH_BIT, 1u);
}

void lightray_enable_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 collision_attribute_index, u32 collision_mesh_index)
{
	SUNDER_SET_BIT(scene->collision_attribute_buffer[collision_attribute_index].should_reproject_vertices_bitmask, collision_mesh_index, 1u);
}

void lightray_disable_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 collision_attribute_index, u32 collision_mesh_index)
{
	SUNDER_ZERO_BIT(scene->collision_attribute_buffer[collision_attribute_index].should_reproject_vertices_bitmask, collision_mesh_index, 1u);
}

void lightray_add_collision_attribute_flags(lightray_scene_t* scene, u32 collision_attribute_index, u32 flags)
{
	scene->collision_attribute_buffer[collision_attribute_index].flags |= flags;
}

void lightray_set_pierce_layer(lightray_scene_t* scene, u32 collision_attribute_index, u16 layer)
{
	scene->collision_attribute_buffer[collision_attribute_index].pierce_layer = layer;
}

b32 lightray_collision_attribute_collides(const lightray_scene_t* scene, u32 collision_mesh_attribute_index)
{
	return SUNDER_IS_ANY_BIT_SET(scene->collision_attribute_buffer[collision_mesh_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_COLLIDES_BIT, 1u);
}

b32 lightray_collision_attribute_aabb_collides(const lightray_scene_t* scene, u32 collision_mesh_attribute_index)
{
	return SUNDER_IS_ANY_BIT_SET(scene->collision_attribute_buffer[collision_mesh_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_AABB_COLLIDES_BIT, 1u);
}

b32 lightray_collision_attribute_collision_meshes_collide(const lightray_scene_t* scene, u32 collision_mesh_attribute_index)
{
	return SUNDER_IS_ANY_BIT_SET(scene->collision_attribute_buffer[collision_mesh_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_COLLISION_MESHES_COLLIDE_BIT, 1u);
}

b32 lightray_collision_attribute_collides_with_entity_of_kind(const lightray_scene_t* scene, u32 collision_mesh_attribute_index, u32 game_side_entity_kind)
{
	const u32 game_side_entity_kind_bitmask_buffer_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(scene->collision_attribute_buffer[collision_mesh_attribute_index].game_side_entity_kind_bitmask_buffer_offset);

	return SUNDER_IS_BUFFERED_BIT_SET(scene->game_side_entity_kind_bitmask_buffer, game_side_entity_kind_bitmask_buffer_offset + game_side_entity_kind);
}

void lightray_set_collision_attribute_entity_kind(lightray_scene_t* scene, u32 collision_mesh_attribute_index, u32 game_side_entity_kind)
{
	scene->collision_attribute_buffer[collision_mesh_attribute_index].game_side_entity_kind = game_side_entity_kind;
}

b32 lightray_ray_aabb_intersect(const lightray_ray_t* ray, const sunder_v3_t& aabb_position, const sunder_v3_t& aabb_scale, f32* t_hit)
{
	f32 t_min = -FLT_MAX;
	f32 t_max = ray->distance;

	const sunder_v3_t ray_origin = sunder_v3(ray->origin.x, ray->origin.y, ray->origin.z);
	const sunder_v3_t ray_direction = sunder_v3(ray->direction.x, ray->direction.y, ray->direction.z);
	
	sunder_v3_t aabb_min = sunder_v3_scalar(0.0f);
	sunder_v3_t aabb_max= sunder_v3_scalar(0.0f);

	lightray_compute_aabb_min_max(aabb_position, aabb_scale, &aabb_min, &aabb_max);

	for (u32 axis = 0; axis < 3; axis++)
	{
		const f32 o = axis == 0 ? ray_origin.x : (axis == 1 ? ray_origin.y : ray_origin.z);
		const f32 d = axis == 0 ? ray_direction.x : (axis == 1 ? ray_direction.y : ray_direction.z);
		const f32 bmin = axis == 0 ? aabb_min.x : (axis == 1 ? aabb_min.y : aabb_min.z);
		const f32 bmax = axis == 0 ? aabb_max.x : (axis == 1 ? aabb_max.y : aabb_max.z);

		if (d != 0.0f)
		{
			f32 t1 = (bmin - o) / d;
			f32 t2 = (bmax - o) / d;

			// Swap if direction is negative
			if (t1 > t2)
			{
				f32 temp = t1;
				t1 = t2;
				t2 = temp;
			}

			if (t1 > t_min) t_min = t1;
			if (t2 < t_max) t_max = t2;

			if (t_min > t_max) return SUNDER_FALSE; // No intersection
			if (t_max < 0) return SUNDER_FALSE;     // Box is behind ray
		}

		else
		{
			// Ray parallel to this axis
			if (o < bmin || o > bmax) return SUNDER_FALSE; // Outside slab
		}
	}

	if (t_hit)
	{
		if (t_min >= 0.0f)
		{
			*t_hit = t_min;        // first intersection in front of ray
		}

		else if (t_max >= 0.0f)
		{
			*t_hit = 0.0f;
		}
		//*t_hit = t_min >= 0 ? t_min : t_max;
	}

	return SUNDER_TRUE;
}

sunder_v3_t lightray_get_grid_cell_center(const lightray_grid_t* grid, u32 row_index, u32 column_index)
{
	return grid->origin + sunder_v3((grid->column_count - 1 - column_index + 0.5f) * grid->cell_width, (row_index + 0.5f) * grid->cell_height, 0.0f);
}

void lightray_reserve_raycast_memory(lightray_scene_t* scene, u32 raycast_grid_cell_index_per_ray, u32 raycast_pierce_layer_test_data_count_per_ray, u32* raycast_grid_cell_index_subarena_offset, u32* has_already_been_traced_bitmask_subarena, u32* raycast_pierce_layer_test_data_subarena_offset)
{
	if (!(sunder_valid_offset(scene->current_raycast_grid_cell_index_subarena_offset, scene->total_raycast_grid_cell_subarena_index_count)))
	{
		return;
	}

	if (!(sunder_valid_offset(scene->current_has_already_traced_bitmask_arena_offset, scene->total_has_already_been_traced_bitmask_count)))
	{
		return;
	}

	if (!(sunder_valid_offset(scene->current_pierce_layer_test_data_subarena_offset, scene->total_pierce_layer_test_data_count)))
	{
		return;
	}

	const u32 current_raycast_grid_cell_index_subarena_offset = scene->current_raycast_grid_cell_index_subarena_offset;
	const u32 current_has_already_traced_bitmask_arena_offset = scene->current_has_already_traced_bitmask_arena_offset;
	const u32 current_pierce_layer_test_data_subarena_offset = scene->current_pierce_layer_test_data_subarena_offset;

	scene->current_raycast_grid_cell_index_subarena_offset += raycast_grid_cell_index_per_ray;
	scene->current_has_already_traced_bitmask_arena_offset += scene->has_already_traced_bitmask_per_raycast_count;
	scene->current_pierce_layer_test_data_subarena_offset += raycast_pierce_layer_test_data_count_per_ray;

	*raycast_grid_cell_index_subarena_offset = current_raycast_grid_cell_index_subarena_offset;
	*has_already_been_traced_bitmask_subarena = current_has_already_traced_bitmask_arena_offset;
	*raycast_pierce_layer_test_data_subarena_offset = current_pierce_layer_test_data_subarena_offset;
}

void lightray_reclaim_raycast_memory(lightray_scene_t* scene)
{
	scene->current_raycast_grid_cell_index_subarena_offset = 0;
	scene->current_has_already_traced_bitmask_arena_offset = 0;
	scene->current_pierce_layer_test_data_subarena_offset = 0;
}

u32 lightray_traverse_grid(u32 index, i32 direction)
{
	u32 val = index;
	val += direction;

	return val;
}

u32 lightray_handle_grid_traversal_aftermath(const lightray_grid_t* grid, sunder_v3_t* position_buffer, sunder_v3_t* scale_buffer, const lightray_ray_t* aabb_intersection_ray, u32 cube_entity_index, u32 row_index, u32 column_index, u32 previous_grid_cell_index, u32* next_row_index, u32* next_column_index)
{
	const u32 grid_cell_index = lightray_get_grid_cell_index(grid, row_index, column_index);

	if (previous_grid_cell_index != grid_cell_index)
	{
		const sunder_v3_t grid_cell_center = lightray_get_grid_cell_center(grid, row_index, column_index);

		position_buffer[cube_entity_index] = grid_cell_center;
		scale_buffer[cube_entity_index] = sunder_v3(lightray_get_default_cube_grid_scale_x(grid), lightray_get_default_cube_grid_scale_y(grid), 90.0f);
		f32 t_hit = 0.0f;

		const b32 cell_on_ray_path = lightray_ray_aabb_intersect(aabb_intersection_ray, position_buffer[cube_entity_index], scale_buffer[cube_entity_index], &t_hit);

		if (cell_on_ray_path)
		{
			//SUNDER_LOG("\nRAY HIT GRID CELL INDEX: ");
			//SUNDER_LOG(grid_cell_index);

			*next_row_index = row_index;
			*next_column_index = column_index;

			//SUNDER_LOG("\nUPDATED ROW INDEX: ");
			//SUNDER_LOG(row_index);
			//SUNDER_LOG("\nUPDATED COLUMN_INDEX: ");
			//SUNDER_LOG(column_index);

			return grid_cell_index;
		}
	}

	return UINT32_MAX;
}

bool lightray_quick_sort_compare_raycast_pierce_layer_test_data_squared_distance_less(const lightray_raycast_pierce_layer_test_data_t* i, const lightray_raycast_pierce_layer_test_data_t* j)
{
	return i->squared_distance < j->squared_distance;
}

SUNDER_IMPLEMENT_QUICK_SORT_PARTITION_FUNCTION(lightray_raycast_pierce_layer_test_data_t, raycast_pierce_layer_test_data, lightray)
SUNDER_IMPLEMENT_QUICK_SORT_FUNCTION(lightray_raycast_pierce_layer_test_data_t, raycast_pierce_layer_test_data, lightray)

b32 lightray_cast_ray(lightray_scene_t* scene, const lightray_raycast_data_t* data, u32* out_raycast_pierce_layer_test_data_count)
{
	u32 written_grid_cell_index_count = 0;
	u32 written_collision_attribute_index_count = 0;

	const sunder_v3_t ray_origin = sunder_v3(data->ray.origin.x, data->ray.origin.y, data->ray.origin.z);
	const sunder_v3_t ray_direction = sunder_v3(data->ray.direction.x, data->ray.direction.y, data->ray.direction.z);
	const f32 ray_distance = data->ray.distance;
	const sunder_v3_t ray_vec = sunder_ray_v3(ray_origin, ray_direction, ray_distance);
	
	u32 ray_vec_row_index = 0;
	u32 ray_vec_column_index = 0;

	const b32 ray_vec_in_bounds = lightray_get_grid_cell_coordinates(data->grid, &ray_vec,  &ray_vec_row_index, &ray_vec_column_index);

	if (!ray_vec_in_bounds)
	{
		return SUNDER_FALSE;
	}

	const u32 ray_vec_grid_cell_index = lightray_get_grid_cell_index(data->grid, ray_vec_row_index, ray_vec_column_index);

	u32 ray_origin_row_index = 0;
	u32 ray_origin_column_index = 0;

	const b32 ray_origin_in_bounds = lightray_get_grid_cell_coordinates(data->grid, &ray_origin, &ray_origin_row_index, &ray_origin_column_index);

	if (!ray_origin_in_bounds)
	{
		return SUNDER_FALSE;
	}

	const u32 ray_origin_grid_cell_index = lightray_get_grid_cell_index(data->grid, ray_origin_row_index, ray_origin_column_index);

	u32 current_grid_cell_index = ray_origin_grid_cell_index;
	u32 current_row_index = ray_origin_row_index;
	u32 current_column_index = ray_origin_column_index;

	//u32 previous_grid_cell_index = current_grid_cell_index;
	u32 previous_grid_cell_index = UINT32_MAX;

	const lightray_ray_t aabb_intersection_ray = data->ray;

	scene->raycast_grid_cell_index_subarena[data->raycast_grid_cell_index_subarena_offset] = current_grid_cell_index;
	written_grid_cell_index_count++;

	while(current_grid_cell_index != ray_vec_grid_cell_index)
	{
		u32 next_row_index = UINT32_MAX;
		u32 next_column_index = UINT32_MAX;

		const u32 next_up_direction = lightray_traverse_grid(current_row_index, -1);
		const bool valid_up_direction = sunder_valid_index(next_up_direction, data->grid->row_count);

		const u32 next_down_direction = lightray_traverse_grid(current_row_index, 1);
		const bool valid_down_direction = sunder_valid_index(next_down_direction, data->grid->row_count);

		const u32 next_left_direction = lightray_traverse_grid(current_column_index, -1);
		const bool valid_left_direction = sunder_valid_index(next_left_direction, data->grid->column_count);

		const u32 next_right_direction = lightray_traverse_grid(current_column_index, 1);
		const bool valid_right_direction = sunder_valid_index(next_right_direction, data->grid->column_count);

		if (valid_up_direction)
		{
			const u32 hit_grid_cell_index = lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, next_up_direction, current_column_index, previous_grid_cell_index, &next_row_index, &next_column_index);

			if (hit_grid_cell_index != UINT32_MAX)
			{
				if (sunder_valid_index(written_grid_cell_index_count, data->raycast_grid_cell_index_count))
				{
					scene->raycast_grid_cell_index_subarena[data->raycast_grid_cell_index_subarena_offset + written_grid_cell_index_count] = hit_grid_cell_index;
					written_grid_cell_index_count++;
				}
			}
		}

		if (valid_down_direction)
		{
			const u32 hit_grid_cell_index = lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, next_down_direction, current_column_index, previous_grid_cell_index, &next_row_index, &next_column_index);

			if (hit_grid_cell_index != UINT32_MAX)
			{
				if (sunder_valid_index(written_grid_cell_index_count, data->raycast_grid_cell_index_count))
				{
					scene->raycast_grid_cell_index_subarena[data->raycast_grid_cell_index_subarena_offset + written_grid_cell_index_count] = hit_grid_cell_index;
					written_grid_cell_index_count++;
				}
			}
		}

		if (valid_left_direction)
		{
			const u32 hit_grid_cell_index = lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, current_row_index, next_left_direction, previous_grid_cell_index, &next_row_index, &next_column_index);

			if (hit_grid_cell_index != UINT32_MAX)
			{
				if (sunder_valid_index(written_grid_cell_index_count, data->raycast_grid_cell_index_count))
				{
					scene->raycast_grid_cell_index_subarena[data->raycast_grid_cell_index_subarena_offset + written_grid_cell_index_count] = hit_grid_cell_index;
					written_grid_cell_index_count++;
				}
			}
		}

		if (valid_right_direction)
		{
			const u32 hit_grid_cell_index = lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, current_row_index, next_right_direction, previous_grid_cell_index, &next_row_index, &next_column_index);

			if (hit_grid_cell_index != UINT32_MAX)
			{
				if (sunder_valid_index(written_grid_cell_index_count, data->raycast_grid_cell_index_count))
				{
					scene->raycast_grid_cell_index_subarena[data->raycast_grid_cell_index_subarena_offset + written_grid_cell_index_count] = hit_grid_cell_index;
					written_grid_cell_index_count++;
				}
			}
		}

		if (next_row_index == UINT32_MAX || next_column_index == UINT32_MAX)
		{
			SUNDER_LOG("\n\nTERMINATED");

			break;
		}

		previous_grid_cell_index = current_grid_cell_index;

		current_row_index = next_row_index;
		current_column_index = next_column_index;

		current_grid_cell_index = lightray_get_grid_cell_index(data->grid, current_row_index, current_column_index);
	}

	const sunder_v3_t destination_cell_center = lightray_get_grid_cell_center(data->grid, current_row_index, current_column_index);
	scene->position_buffer[data->cube_entity_index] = destination_cell_center;
	scene->scale_buffer[data->cube_entity_index] = sunder_v3(lightray_get_default_cube_grid_scale_x(data->grid), lightray_get_default_cube_grid_scale_y(data->grid), 90.0f);

	b32 ray_collision_attribute_intersect = SUNDER_FALSE;
	u32 written_pierce_layer_test_data_count = 0;

	u32 closest_collision_attribute_index = UINT32_MAX;
	f32 closest_squared_distance = FLT_MAX;
	sunder_v3_t closest_intersection_point{};

	for (u32 i = 0; i < written_grid_cell_index_count; i++)
	{
		const u32 cgci = scene->raycast_grid_cell_index_subarena[data->raycast_grid_cell_index_subarena_offset + i];
		const u32 cai_buffer_offset = data->grid->cell_buffer[cgci].collision_attribute_index_buffer_offset;
		const u32 cai_count = data->grid->cell_buffer[cgci].collision_attribute_index_count;

		for (u32 cai = 0; cai < cai_count; cai++)
		{
			const u32 current_cai = data->grid->collision_attribute_index_buffer[cai_buffer_offset + cai];
			const u32 collision_attribute_flags = scene->collision_attribute_buffer[current_cai].flags;

			const u32 ray_collision_layer_index_count = data->collision_layer_index_count;

			if (!(SUNDER_IS_ANY_BIT_SET(collision_attribute_flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT, 1u)))
			{
				continue;
			}

			const u32 buffered_bit_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->has_already_been_traced_bitmask_subarena_offset);

			if (SUNDER_IS_BUFFERED_BIT_SET(scene->has_already_been_traced_bitmask_subarena, buffered_bit_offset + current_cai))
			{
				continue;
			}

			const u32 collision_attribute_aabb_entity_index = scene->collision_attribute_buffer[current_cai].aabb_index;

			if (!(SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_HIT_ANY_BIT, 1u)))
			{
				const u64 collision_attribute_collision_layer_bitmask = scene->collision_attribute_buffer[current_cai].collision_layer_bitmask;
				u64 common_collision_layer_bitmask = 0;

				for (u32 collision_layer = 0; collision_layer < ray_collision_layer_index_count; collision_layer++)
				{
					const u32 current_collision_layer_bit = data->collision_layer_index_buffer[collision_layer];

					const b32 collision_attribute_collision_layer_check = SUNDER_IS_ANY_BIT_SET(collision_attribute_collision_layer_bitmask, current_collision_layer_bit, 1ull);
					const b32 ray_collision_layer_check = SUNDER_IS_ANY_BIT_SET(data->collision_layer_bitmask, current_collision_layer_bit, 1ull);

					if (collision_attribute_collision_layer_check && ray_collision_layer_check)
					{
						SUNDER_SET_BIT(common_collision_layer_bitmask, current_collision_layer_bit, 1ull);
					}
				}

				if (common_collision_layer_bitmask == 0)
				{
					continue;
				}
			}

			const sunder_v3_t collision_attribute_aabb_position = scene->position_buffer[collision_attribute_aabb_entity_index];
			const sunder_v3_t collision_attribute_aabb_scale = scene->scale_buffer[collision_attribute_aabb_entity_index];
			f32 t_hit = 0.0f;

			const b32 collision_attribute_aabb_ray_intersect = lightray_ray_aabb_intersect(&aabb_intersection_ray, collision_attribute_aabb_position, collision_attribute_aabb_scale, &t_hit);

			if (collision_attribute_aabb_ray_intersect)
			{
				const u32 collision_mesh_batch_index = scene->collision_attribute_buffer[current_cai].collision_mesh_batch_index;
				const u32 collision_mesh_count = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count;
				const u32 collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset;

				f32 out_t = 0.0f;

				for (u32 cm = 0; cm < collision_mesh_count; cm++)
				{
					sunder_v3_t* collision_mesh_projected_vertex_positions = scene->collision_mesh_buffer[collision_mesh_buffer_offset + cm].projected_vertex_position_buffer;
					const u32 collision_mesh_vertex_count = scene->collision_mesh_buffer[collision_mesh_buffer_offset + cm].vertex_count;
					
					f32 out_u = 0.0f;
					f32 out_v = 0.0f;

					b32 collision_mesh_ray_intersect = SUNDER_FALSE;

					for (u32 v = 0; v < collision_mesh_vertex_count; v += 3)
					{
						const b32 ray_triangle_intersect = lightray_ray_triangle_intersect(&aabb_intersection_ray, &collision_mesh_projected_vertex_positions[v], &out_t, &out_u, &out_v);

						if (ray_triangle_intersect)
						{
							collision_mesh_ray_intersect = SUNDER_TRUE;
							break;
						}
					}

					if (collision_mesh_ray_intersect)
					{
						SUNDER_SET_BUFFERED_BIT(scene->has_already_been_traced_bitmask_subarena, buffered_bit_offset + current_cai);

						if (sunder_valid_index(written_pierce_layer_test_data_count, data->raycast_pierce_layer_test_data_count))
						{
							const sunder_v3_t intersection_point = ray_origin + ray_direction * out_t;
							const f32 intersection_ray_squared_distance = sunder_squared_distance_v3(intersection_point, ray_origin);

							if (intersection_ray_squared_distance < closest_squared_distance)
							{
								closest_squared_distance = intersection_ray_squared_distance;
								closest_collision_attribute_index = current_cai;
								closest_intersection_point = intersection_point;
							}

							scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + written_pierce_layer_test_data_count].intersection_point = intersection_point;
							scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + written_pierce_layer_test_data_count].squared_distance = intersection_ray_squared_distance;
							scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + written_pierce_layer_test_data_count].collision_attribute_index = current_cai;
								
							written_pierce_layer_test_data_count++;
						}

						ray_collision_attribute_intersect = SUNDER_TRUE;
					}
				}
			}
		}
	}

	u32 filtered_pierce_layer_count = 0;

	if (written_pierce_layer_test_data_count > 0)
	{
		if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_PIERCE_BIT, 1u))
		{
			if (written_pierce_layer_test_data_count > 1)
			{
				lightray_quick_sort_raycast_pierce_layer_test_data(&scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset], 0, written_pierce_layer_test_data_count - 1, lightray_quick_sort_compare_raycast_pierce_layer_test_data_squared_distance_less);
			}

			lightray_raycast_pierce_layer_t previous_collision_attribute_pierce_layer{};
			previous_collision_attribute_pierce_layer.layer = UINT16_MAX;
			previous_collision_attribute_pierce_layer.threshold = UINT16_MAX;

			for (u32 pltd = 0; pltd < written_pierce_layer_test_data_count; pltd++)
			{
				const u32 current_collision_attribute_index = scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + pltd].collision_attribute_index;
				const u16 current_collision_attribute_pierce_layer = scene->collision_attribute_buffer[current_collision_attribute_index].pierce_layer;

				b32 ray_able_to_pierce_through = SUNDER_FALSE;

				for (u32 rpl = 0; rpl < data->pierce_layer_count; rpl++)
				{
					const u16 current_ray_pierce_layer = data->pierce_layer_buffer[rpl].layer;
					const u16 current_ray_pierce_layer_threshold = data->pierce_layer_buffer[rpl].threshold;

					filtered_pierce_layer_count++;
					scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + pltd].previous_hit_pierce_layer = previous_collision_attribute_pierce_layer.layer;
					scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + pltd].previous_hit_applied_threshold = previous_collision_attribute_pierce_layer.threshold;

					if (current_ray_pierce_layer == current_collision_attribute_pierce_layer)
					{
						if (current_ray_pierce_layer_threshold > 0 || SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_MAX_PIERCE_THRESHOLD_BIT, 1u))
						{
							previous_collision_attribute_pierce_layer.layer = current_collision_attribute_pierce_layer;
							previous_collision_attribute_pierce_layer.threshold = sunder_clamp_u16(0, current_ray_pierce_layer_threshold, current_ray_pierce_layer_threshold - 1);
							ray_able_to_pierce_through = SUNDER_TRUE;

							break;
						}
					}
				}

				if (!ray_able_to_pierce_through)
				{
					break;
				}
			}
		}

		else
		{
			scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].intersection_point = closest_intersection_point;
			scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].squared_distance = closest_squared_distance;
			scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].collision_attribute_index = closest_collision_attribute_index;
			scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].previous_hit_pierce_layer = UINT16_MAX;
			scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].previous_hit_applied_threshold = UINT16_MAX;

			filtered_pierce_layer_count = 1;
		}

		if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_MARK_INTERSECTION_POINT_BIT, 1u) && filtered_pierce_layer_count > 0)
		{
			scene->position_buffer[data->cube_entity_index] = scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].intersection_point;
			scene->scale_buffer[data->cube_entity_index] = sunder_v3(0.05f, 0.05f, 0.05f);
		}
	}

	if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_TRACE_LINE_BIT, 1u))
	{
		const sunder_quat_t line_trace_rotation = sunder_rotate_from_to_v3(ray_direction, sunder_v3(0.0f, 1.0f, 0.0f));

		const f32 ray_offset = 0.60f * ray_distance;

		scene->position_buffer[data->line_trace_entity_index] = ray_origin + ray_direction * ray_offset;
		scene->quat_rotation_buffer[data->line_trace_entity_index] = line_trace_rotation;
		scene->scale_buffer[data->line_trace_entity_index].y = ray_distance * 1.225f;
		scene->scale_buffer[data->line_trace_entity_index].z = 0.01f;
		scene->scale_buffer[data->line_trace_entity_index].x = 0.01f;
	}

	*out_raycast_pierce_layer_test_data_count = filtered_pierce_layer_count;

	return ray_collision_attribute_intersect;
}

lightray_ray_t lightray_ray(const sunder_v3_t& origin, const sunder_v3_t& direction, f32 distance)
{
	lightray_ray_t result{};

	const glm::vec3 converted_origin = glm::vec3(origin.x, origin.y, origin.z);
	const glm::vec3 converted_direction = glm::vec3(direction.x, direction.y, direction.z);
	
	result.origin = converted_origin;
	result.direction = converted_direction;
	result.distance = distance;

	return result;
}

u16 lightray_get_pierce_layer_hit_count(u16 initial_threshold, u16 applied_at_hit)
{
	if (applied_at_hit == UINT16_MAX)
	{
		return UINT16_MAX;
	}

	return initial_threshold - sunder_clamp_u16(0, initial_threshold, applied_at_hit);
}

u32 lightray_get_bone_computed_transform_matrix_buffer_index(const lightray_animation_core_t* animation_core, cstring_literal* bone_name, u32 skeleton_index, u32 instance_index)
{
	const u32 bone_name_length = SUNDER_CAST2(u32)strlen(bone_name);
	u32 relative_bone_index = UINT32_MAX;

	const u32 bone_buffer_offset = animation_core->skeleton_buffer[skeleton_index].bone_buffer_offset;
	const u32 bone_count = animation_core->skeleton_buffer[skeleton_index].bone_count;
	const u32 computed_bone_transform_matrix_buffer_offset_base = animation_core->skeleton_buffer[skeleton_index].computed_bone_transform_matrix_buffer_offset;

	for (u32 i = 0; i < bone_count; i++)
	{
		const bool bone_found = sunder_compare_strings(animation_core->bone_names[bone_buffer_offset + i].data, animation_core->bone_names[bone_buffer_offset + i].length, bone_name, bone_name_length);
		
		if (bone_found)
		{
			relative_bone_index = i;
			break;
		}
	}

	const u32 computed_bone_transform_matrix_buffer_offset = lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(instance_index, bone_count, computed_bone_transform_matrix_buffer_offset_base);

	if (relative_bone_index == UINT32_MAX)
	{
		return UINT32_MAX;
	}

	return relative_bone_index + computed_bone_transform_matrix_buffer_offset;
}

sunder_m4_t lightray_get_bone_m4(const lightray_animation_core_t* animation_core, u32 bone_computed_transform_matrix_buffer_index)
{
	if (!(sunder_valid_index(bone_computed_transform_matrix_buffer_index, animation_core->total_computed_bone_transform_matrix_buffer_bone_count)))
	{
		return sunder_m4_identity();
	}

	return lightray_copy_glm_mat4_to_sunder(animation_core->computed_bone_matrix_buffer[bone_computed_transform_matrix_buffer_index]);
}