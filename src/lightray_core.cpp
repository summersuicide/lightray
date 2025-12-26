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

b32 lightray_ray_triangle_intersect(const lightray_ray_t* ray, const sunder_v3_t* triangle_vertices, f32* out_t, f32* out_u, f32* out_v, lightray_ray_triangle_face_culling_mode_e culling_mode, sunder_v3_t* hit_v0, sunder_v3_t* hit_v1, sunder_v3_t* hit_v2)
{
	const sunder_v3_t ray_origin = sunder_v3(ray->origin.x, ray->origin.y, ray->origin.z);
	const sunder_v3_t ray_direction = sunder_v3(ray->direction.x, ray->direction.y, ray->direction.z);

	const f32 EPSILON = 1e-8f;

	const sunder_v3_t AB = triangle_vertices[1] - triangle_vertices[0];
	const sunder_v3_t AC = triangle_vertices[2] - triangle_vertices[0];
	const sunder_v3_t P = sunder_cross_v3(ray_direction, AC);
	const sunder_v3_t N = sunder_cross_v3(AB, AC);

	const f32 det = sunder_dot_v3(AB, P);

	if (fabs(det) < EPSILON)
	{
		return SUNDER_FALSE;
	}

	const f32 invDet = 1.0f / det;

	const sunder_v3_t T = ray_origin - triangle_vertices[0];

	const f32 u = sunder_dot_v3(T, P) * invDet;
	if (u < 0.0f || u > 1.0f) return SUNDER_FALSE;

	sunder_v3_t Q = sunder_cross_v3(T, AB);
	const f32 v = sunder_dot_v3(ray_direction, Q) * invDet;
	if (v < 0.0f || u + v > 1.0f) return SUNDER_FALSE;

	const f32 t = sunder_dot_v3(AC, Q) * invDet;

	if (t < EPSILON) return SUNDER_FALSE;

	const f32 normal_dot_ray = sunder_dot_v3(N, ray_direction);

	if (culling_mode == LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_BACKFACE)
	{
		if (normal_dot_ray > 0)
		{
			return SUNDER_FALSE;
		}
	}

	else if (culling_mode == LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_FRONTFACE)
	{
		if (normal_dot_ray < 0)
		{
			return SUNDER_FALSE;
		}
	}

	*hit_v0 = triangle_vertices[0];
	*hit_v1 = triangle_vertices[1];
	*hit_v2 = triangle_vertices[2];

	*out_t = (f32)t;
	*out_u = (f32)u;
	*out_v = (f32)v;

	return SUNDER_TRUE;
}

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

void lightray_compute_sentinel_world_space_vertex_positions(const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_m4_t& m)
{
	for (u32 i = 0; i < sentinel_vertex_count; i++)
	{
		sentinel_world_space_vertex_position_buffer[i] = sunder_v3_v4(m * sunder_v4_v3(sentinel_local_space_vertex_position_buffer[i], 1.0f));

		//const glm::vec3 v = model->model * glm::vec4(glm::vec3(raw_vertex_positions[i].x, raw_vertex_positions[i].y, raw_vertex_positions[i].z), 1.0f);
		//projected_vertex_positions[i] = sunder_v3(v.x, v.y, v.z);
	}
}

void lightray_compute_sentinel_world_space_vertex_positions_via_index_buffer(const u32* sentinel_index_buffer, const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_m4_t& m)
{
	for (u32 v = 0; v < sentinel_vertex_count; v++)
	{
		const u32 vertex_index = sentinel_index_buffer[v];
		const sunder_v3_t vertex_position = sentinel_local_space_vertex_position_buffer[vertex_index];

		sentinel_world_space_vertex_position_buffer[v] = sunder_v3_v4(m * sunder_v4_v3(vertex_position, 1.0f));
	}
}

void lightray_compute_sentinel_world_space_vertex_positions_ts(const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_v3_t& t, const sunder_v3_t& s)
{
	for (u32 i = 0; i < sentinel_vertex_count; i++)
	{
		sentinel_world_space_vertex_position_buffer[i] = sunder_v3_world_space_ts(sentinel_local_space_vertex_position_buffer[i], t, s);
	}
}

void lightray_compute_sentinel_world_space_vertex_positions_t(const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_v3_t& t)
{
	for (u32 i = 0; i < sentinel_vertex_count; i++)
	{
		sentinel_world_space_vertex_position_buffer[i] = sentinel_local_space_vertex_position_buffer[i] + t;
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

lightray_gjk_support_point_t lightray_gjk_support(const sunder_v3_t* vertex_positions1, u32 vertex_count1, const sunder_v3_t* vertex_positions2, u32 vertex_count2, const sunder_v3_t& direction)
{
	const sunder_v3_t a = lightray_gjk_find_furthest_point(vertex_positions1, vertex_count1, direction);
	const sunder_v3_t b = lightray_gjk_find_furthest_point(vertex_positions2, vertex_count2, -direction);

	lightray_gjk_support_point_t support_point{};
	support_point.a = a;
	support_point.b = b;
	support_point.p = a - b;

	return support_point;
}

b32 lightray_gjk_same_direction(const sunder_v3_t& direction, sunder_v3_t& ao)
{
	return sunder_dot_v3(direction, ao) > 0;
}

b32 lightray_gjk_line(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction)
{
	const f32 EPSILON = 1e-12f;

	sunder_v3_t a = simplex[0].p;
	sunder_v3_t b = simplex[1].p;
	sunder_v3_t ab = b - a;
	sunder_v3_t ao = -a;

	// Check degenerate
	if (sunder_squared_magnitude_v3(ab) < EPSILON)
	{
		*direction = ao;
		return SUNDER_FALSE;
	}

	if (lightray_gjk_same_direction(ab, ao))
	{
		*direction = sunder_cross_v3(sunder_cross_v3(ab, ao), ab);
		if (sunder_squared_magnitude_v3(*direction) < EPSILON) *direction = ao;
	}
	else
	{
		// Keep only A
		simplex[0] = simplex[0];
		*simplex_size = 1;
		*direction = ao;
	}

	return SUNDER_FALSE;
}

b32 lightray_gjk_triangle(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction)
{


	const f32 EPSILON = 1e-12f;

	sunder_v3_t a = simplex[0].p;
	sunder_v3_t b = simplex[1].p;
	sunder_v3_t c = simplex[2].p;

	sunder_v3_t ab = b - a;
	sunder_v3_t ac = c - a;
	sunder_v3_t ao = -a;

	sunder_v3_t abc = sunder_cross_v3(ab, ac);

	// Check if origin is in AC region
	sunder_v3_t ac_perp = sunder_cross_v3(abc, ac);
	if (lightray_gjk_same_direction(ac_perp, ao))
	{
		if (lightray_gjk_same_direction(ac, ao))
		{
			// Keep A and C
			simplex[1] = simplex[2];
			*simplex_size = 2;
			*direction = sunder_cross_v3(sunder_cross_v3(ac, ao), ac);
			if (sunder_squared_magnitude_v3(*direction) < EPSILON) *direction = ao;
		}
		else
		{
			// Reduce to line AB
			simplex[1] = simplex[1];
			*simplex_size = 2;
			return lightray_gjk_line(simplex, simplex_size, direction);
		}
		return SUNDER_FALSE;
	}

	// Check if origin is in AB region
	sunder_v3_t ab_perp = sunder_cross_v3(ab, abc);
	if (lightray_gjk_same_direction(ab_perp, ao))
	{
		// Reduce to line AB
		*simplex_size = 2;
		return lightray_gjk_line(simplex, simplex_size, direction);
	}

	// Origin is above or below triangle
	if (lightray_gjk_same_direction(abc, ao))
	{
		*direction = abc;
	}
	else
	{
		*direction = -abc;
		// Swap B and C
		lightray_gjk_support_point_t temp = simplex[1];
		simplex[1] = simplex[2];
		simplex[2] = temp;
	}

	if (sunder_squared_magnitude_v3(*direction) < EPSILON) *direction = ao;

	return SUNDER_FALSE;
}

b32 lightray_gjk_tetrahedron(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction)
{


	sunder_v3_t a = simplex[0].p;
	sunder_v3_t b = simplex[1].p;
	sunder_v3_t c = simplex[2].p;
	sunder_v3_t d = simplex[3].p;

	sunder_v3_t ao = -a;

	sunder_v3_t ab = b - a;
	sunder_v3_t ac = c - a;
	sunder_v3_t ad = d - a;

	sunder_v3_t abc = sunder_cross_v3(ab, ac);
	sunder_v3_t acd = sunder_cross_v3(ac, ad);
	sunder_v3_t adb = sunder_cross_v3(ad, ab);

	// Check each face
	if (lightray_gjk_same_direction(abc, ao))
	{
		// Reduce to triangle ABC
		simplex[3] = simplex[2]; // discard D
		*simplex_size = 3;
		return lightray_gjk_triangle(simplex, simplex_size, direction);
	}
	if (lightray_gjk_same_direction(acd, ao))
	{
		// Reduce to triangle ACD
		simplex[1] = simplex[2];
		simplex[2] = simplex[3];
		*simplex_size = 3;
		return lightray_gjk_triangle(simplex, simplex_size, direction);
	}
	if (lightray_gjk_same_direction(adb, ao))
	{
		// Reduce to triangle ADB
		simplex[2] = simplex[1];
		simplex[1] = simplex[3];
		*simplex_size = 3;
		return lightray_gjk_triangle(simplex, simplex_size, direction);
	}

	// Origin is inside tetrahedron
	return SUNDER_TRUE;
}

b32 lightray_gjk_next_simplex(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction)
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

b32 lightray_gjk_intersect(const sunder_v3_t* vertex_positions1, u32 vertex_count1, const sunder_v3_t* vertex_positions2, u32 vertex_count2, lightray_gjk_support_point_t* out_simplex, u32* out_simplex_size)
{
	u32 iteration_count = 0;
	lightray_gjk_support_point_t support = lightray_gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, sunder_v3(1.0f, 0.0f, 0.0f));

	lightray_gjk_support_point_t simplex[4]{};
	u32 simplex_size = 1;
	simplex[0] = support;

	sunder_v3_t direction = -support.p;

	while (SUNDER_TRUE)
	{
		iteration_count++;

		if (iteration_count > 50)
		{
			SUNDER_LOG("\n\nITERATION COUNT EXCEEDED, INFINITE LOOP TERMINATED");
			return SUNDER_FALSE;
		}

		support = lightray_gjk_support(vertex_positions1, vertex_count1, vertex_positions2, vertex_count2, direction);

		if (sunder_dot_v3(support.p, direction) <= 0)
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
			*out_simplex_size = simplex_size;

			for (u32 i = 0; i < simplex_size; i++)
			{
				out_simplex[i] = simplex[i];
			}

			return SUNDER_TRUE;
		}


		// Prevent degenerate direction
		if (sunder_squared_magnitude_v3(direction) < 1e-12f)
			return SUNDER_TRUE;
	};
}

void lightray_gjk_compute_closest_point(lightray_gjk_support_point_t* simplex, u32 simplex_size, sunder_v3_t* closest, f32* bary)
{
	if (simplex_size == 1)
	{
		*closest = simplex[0].p;
		bary[0] = 1.0f;
		return;
	}

	if (simplex_size == 2)
	{
		// Segment AB
		sunder_v3_t A = simplex[0].p;
		sunder_v3_t B = simplex[1].p;

		sunder_v3_t AB = B - A;
		sunder_v3_t AO = -A;

		float t = sunder_dot_v3(AO, AB);

		if (t <= 0)
		{
			*closest = A;
			bary[0] = 1.0f;
			bary[1] = 0.0f;
			return;
		}

		float denom = sunder_dot_v3(AB, AB);

		if (t >= denom)
		{
			*closest = B;
			bary[0] = 0.0f;
			bary[1] = 1.0f;
			return;
		}

		t /= denom;

		*closest = A + AB * t;
		bary[0] = 1.0f - t;
		bary[1] = t;
		return;
	}

	if (simplex_size == 3)
	{
		// Triangle ABC
		sunder_v3_t A = simplex[0].p;
		sunder_v3_t B = simplex[1].p;
		sunder_v3_t C = simplex[2].p;

		sunder_v3_t AB = B - A;
		sunder_v3_t AC = C - A;
		sunder_v3_t AO = -A;

		// Normal of triangle
		sunder_v3_t ABCn = sunder_cross_v3(AB, AC);

		// Check vertex A region
		sunder_v3_t ABn = sunder_cross_v3(AB, ABCn);
		if (sunder_dot_v3(ABn, AO) > 0)
		{
			// Remove C -> segment AB
			simplex[2] = simplex[1];
			simplex[1] = simplex[0];
			return lightray_gjk_compute_closest_point(simplex, 2, closest, bary);
		}

		sunder_v3_t ACn = sunder_cross_v3(ABCn, AC);
		if (sunder_dot_v3(ACn, AO) > 0)
		{
			// Remove B -> segment AC
			simplex[1] = simplex[2];
			return lightray_gjk_compute_closest_point(simplex, 2, closest, bary);
		}

		// Inside triangle; compute barycentric normally
		float d00 = sunder_dot_v3(AB, AB);
		float d01 = sunder_dot_v3(AB, AC);
		float d11 = sunder_dot_v3(AC, AC);
		float d20 = sunder_dot_v3(AO, AB);
		float d21 = sunder_dot_v3(AO, AC);

		float denom = d00 * d11 - d01 * d01;
		float v = (d11 * d20 - d01 * d21) / denom;
		float w = (d00 * d21 - d01 * d20) / denom;
		float u = 1.0f - v - w;

		*closest = A * u + B * v + C * w;

		bary[0] = u;
		bary[1] = v;
		bary[2] = w;
		return;
	}
}

lightray_gjk_closest_hit_t lightray_gjk_distance(const sunder_v3_t* vertices_a, u32 vertex_count_a, const sunder_v3_t* vertices_b, u32 vertex_count_b, const sunder_v3_t& search_direction)
{
	/*
	lightray_gjk_closest_hit_t result{};
	result.valid = false;

	lightray_gjk_support_point_t simplex[4];
	u32 simplex_size = 0;

	sunder_v3_t direction = sunder_v3(1.0f, 0.0f, 0.0f); // any non-zero

	// First support
	simplex[0] = lightray_gjk_support(vertices_a, vertex_count_a, vertices_b, vertex_count_b, direction);
	simplex_size = 1;

	direction = -simplex[0].p;

	const u32 max_iter = 32;
	for (u32 iter = 0; iter < max_iter; iter++)
	{
		// New support point in current direction
		lightray_gjk_support_point_t newP =
			lightray_gjk_support(vertices_a, vertex_count_a, vertices_b, vertex_count_b, direction);

		// If no progress → we are at closest features
		f32 proj = sunder_dot_v3(newP.p, direction);
		f32 dist2 = sunder_dot_v3(direction, direction);

		if (proj - dist2 < 1e-6f)
			break;

		// Add to simplex
		simplex[simplex_size++] = newP;

		// Compute closest point to origin inside this simplex
		sunder_v3_t closest;
		f32 bary[4] = { 0 };

		lightray_gjk_compute_closest_point(simplex, simplex_size, &closest, bary);

		// Set new direction toward origin
		direction = -closest;

		// If origin reached → distance = 0
		if (sunder_dot_v3(direction, direction) < 1e-12f)
		{
			// Reconstruct closestA and closestB at origin
			sunder_v3_t cA = sunder_v3(0, 0, 0);
			sunder_v3_t cB = sunder_v3(0, 0, 0);

			for (u32 i = 0; i < simplex_size; i++)
			{
				cA += simplex[i].a * bary[i];
				cB += simplex[i].b * bary[i];
			}

			result.closest_a = cA;
			result.closest_b = cB;
			result.distance = 0.0f;
			result.valid = SUNDER_TRUE;
			return result;
		}
	}

	// Compute final closest point
	sunder_v3_t closest;
	f32 bary[4] = { 0 };
	lightray_gjk_compute_closest_point(simplex, simplex_size, &closest, bary);

	sunder_v3_t cA = sunder_v3(0, 0, 0);
	sunder_v3_t cB = sunder_v3(0, 0, 0);

	for (u32 i = 0; i < simplex_size; i++)
	{
		cA += simplex[i].a * bary[i];
		cB += simplex[i].b * bary[i];
	}

	result.closest_a = cA;
	result.closest_b = cB;
	result.distance = sunder_magnitude_v3(cB - cA);
	result.valid = SUNDER_TRUE;

	sunder_v3_t world_sep = result.closest_b - result.closest_a;

	if (sunder_dot_v3(world_sep, world_sep) > 1e-12f)
	{
		result.normal = sunder_normalize_v3(world_sep);
	}
	else
	{
		// intersection case: fallback to last GJK direction
		result.normal = sunder_normalize_v3(direction);
	}

	return result;

	*/



	/*

	lightray_gjk_closest_hit_t result{};
	result.valid = false;

	lightray_gjk_support_point_t simplex[4];
	u32 simplex_size = 0;

	sunder_v3_t direction = search_direction; // any non-zero

	// First support
	simplex[0] = lightray_gjk_support(vertices_a, vertex_count_a, vertices_b, vertex_count_b, direction);
	simplex_size = 1;

	direction = -simplex[0].p;

	const u32 max_iter = 32;
	for (u32 iter = 0; iter < max_iter; iter++)
	{
		// New support point in current direction
		lightray_gjk_support_point_t newP =
			lightray_gjk_support(vertices_a, vertex_count_a, vertices_b, vertex_count_b, direction);

		// If no progress → we are at closest features
		f32 proj = sunder_dot_v3(newP.p, direction);
		f32 dist2 = sunder_dot_v3(direction, direction);

		if (proj - dist2 < 1e-6f)
			break;

		// Add to simplex
		simplex[simplex_size++] = newP;

		// Compute closest point to origin inside this simplex
		sunder_v3_t closest;
		f32 bary[4] = { 0 };

		lightray_gjk_compute_closest_point(simplex, simplex_size, &closest, bary);

		// Set new direction toward origin
		direction = -closest;

		// If origin reached → distance = 0
		if (sunder_dot_v3(direction, direction) < 1e-12f)
		{
			// Reconstruct closest points at origin (intersection detected)
			sunder_v3_t cA = sunder_v3(0, 0, 0);
			sunder_v3_t cB = sunder_v3(0, 0, 0);
			for (u32 i = 0; i < simplex_size; i++)
			{
				cA += simplex[i].a * bary[i];
				cB += simplex[i].b * bary[i];
			}

			result.closest_a = cA;
			result.closest_b = cB;
			result.distance = 0.0f;  // intersection detected
			result.valid = SUNDER_TRUE;
			return result;  // Return immediately as the shapes are intersecting
		}
	}

	// Compute final closest point (if no intersection was found)
	sunder_v3_t closest;
	f32 bary[4] = { 0 };
	lightray_gjk_compute_closest_point(simplex, simplex_size, &closest, bary);

	sunder_v3_t cA = sunder_v3(0, 0, 0);
	sunder_v3_t cB = sunder_v3(0, 0, 0);

	for (u32 i = 0; i < simplex_size; i++)
	{
		cA += simplex[i].a * bary[i];
		cB += simplex[i].b * bary[i];
	}

	result.closest_a = cA;
	result.closest_b = cB;
	result.distance = sunder_magnitude_v3(cB - cA);
	result.valid = SUNDER_TRUE;

	sunder_v3_t world_sep = result.closest_b - result.closest_a;

	if (sunder_dot_v3(world_sep, world_sep) > 1e-12f)
	{
		result.normal = sunder_normalize_v3(world_sep);
	}
	else
	{
		// Intersection case: fallback to last GJK direction
		result.normal = sunder_normalize_v3(direction);
	}

	return result;
	*/


lightray_gjk_closest_hit_t result{};
result.valid = false;

lightray_gjk_support_point_t simplex[4];
u32 simplex_size = 0;

sunder_v3_t direction = sunder_v3(1.0f, 0.0f, 0.0f);  // any non-zero direction

// First support point
simplex[0] = lightray_gjk_support(vertices_a, vertex_count_a, vertices_b, vertex_count_b, direction);
simplex_size = 1;

direction = -simplex[0].p;  // Refine the search direction

const u32 max_iter = 32;
for (u32 iter = 0; iter < max_iter; iter++)
{
	// Get new support point in the current direction
	lightray_gjk_support_point_t newP =
		lightray_gjk_support(vertices_a, vertex_count_a, vertices_b, vertex_count_b, direction);

	// Check for minimal progress
	f32 proj = sunder_dot_v3(newP.p, direction);
	f32 dist2 = sunder_dot_v3(direction, direction);

	// If no progress in direction, break (converged)
	if (proj - dist2 < 1e-6f)
		break;

	// Add the new point to the simplex
	simplex[simplex_size++] = newP;

	// Compute the closest point on the simplex
	sunder_v3_t closest;
	f32 bary[4] = { 0 };
	lightray_gjk_compute_closest_point(simplex, simplex_size, &closest, bary);

	// Update the direction
	direction = -closest;

	// If direction is very small, we've found the closest points
	if (sunder_dot_v3(direction, direction) < 1e-12f)
	{
		// Reconstruct the closest points on both objects
		sunder_v3_t cA = sunder_v3(0, 0, 0);
		sunder_v3_t cB = sunder_v3(0, 0, 0);

		for (u32 i = 0; i < simplex_size; i++)
		{
			cA += simplex[i].a * bary[i];
			cB += simplex[i].b * bary[i];
		}

		result.closest_a = cA;
		result.closest_b = cB;
		result.distance = sunder_magnitude_v3(cB - cA);  // Minimal distance
		result.valid = SUNDER_TRUE;
		return result;
	}
}

// Final closest point if no intersection detected
sunder_v3_t closest;
f32 bary[4] = { 0 };
lightray_gjk_compute_closest_point(simplex, simplex_size, &closest, bary);

sunder_v3_t cA = sunder_v3(0, 0, 0);
sunder_v3_t cB = sunder_v3(0, 0, 0);

for (u32 i = 0; i < simplex_size; i++)
{
	cA += simplex[i].a * bary[i];
	cB += simplex[i].b * bary[i];
}

result.closest_a = cA;
result.closest_b = cB;
result.distance = sunder_magnitude_v3(cB - cA);  // Final distance
result.valid = SUNDER_TRUE;

return result;

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
	scene->index_buffer = suballocation_data->index_buffer;
	scene->vertex_buffer = suballocation_data->vertex_buffer;

	scene->total_entity_count = suballocation_data->total_entity_count;
	scene->total_mesh_count = suballocation_data->total_mesh_count;
	scene->total_instance_model_count = suballocation_data->total_instance_model_count;


	////////////////////////////////////////// collision_attribute core //////////////////////////////////////////
	scene->collision_attribute_core.collision_attribute_buffer.capacity = suballocation_data->collision_attribute_count;
	 
	scene->collision_attribute_core.collision_mesh_batch_buffer.capacity = suballocation_data->collision_attribute_count;
	scene->collision_attribute_core.collision_mesh_batch_buffer.count_per = suballocation_data->collision_mesh_per_batch_count;

	scene->collision_attribute_core.collision_mesh_buffer.capacity = suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count;
	 
	scene->collision_attribute_core.collision_layer_index_buffer.size = suballocation_data->collision_attribute_count * 64;

	scene->collision_attribute_core.collision_grid_cell_row_index_buffer.capacity = suballocation_data->collision_attribute_count * suballocation_data->grid_cell_index_per_collision_attribute_count;
	scene->collision_attribute_core.collision_grid_cell_row_index_buffer.count_per = suballocation_data->grid_cell_index_per_collision_attribute_count;

	scene->collision_attribute_core.has_already_collided_with_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count, suballocation_data->collision_attribute_count);
	scene->collision_attribute_core.has_already_collided_with_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->collision_attribute_count);
		
	scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count, suballocation_data->collision_attribute_count);
	scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->collision_attribute_count);

	scene->collision_attribute_core.filtered_collision_mesh_index_buffer.capacity = suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count;

	scene->collision_attribute_core.trace_step_buffer.capacity = suballocation_data->collision_attribute_count * LIGHTRAY_MAX_TRACE_STEP_COUNT;
	scene->collision_attribute_core.trace_step_buffer.count_per = LIGHTRAY_MAX_TRACE_STEP_COUNT;

	scene->collision_attribute_core.trace_step_grid_cell_index_buffer.capacity = scene->collision_attribute_core.trace_step_buffer.capacity * suballocation_data->collision_attribute_per_cell_count;
	scene->collision_attribute_core.trace_step_grid_cell_index_buffer.count_per = suballocation_data->collision_attribute_per_cell_count;

	scene->collision_attribute_core.trace_data_buffer.capacity = suballocation_data->collision_attribute_count;

	////////////////////////////////////////// collision_attribute core //////////////////////////////////////////


	////////////////////////////////////////// raycast core //////////////////////////////////////////
	scene->raycast_core.max_raycast_per_frame_count = suballocation_data->total_per_frame_raycast_count;

	scene->raycast_core.raycast_grid_cell_index_subarena.capacity = suballocation_data->total_raycast_grid_cell_index_count;

	scene->raycast_core.pierce_layer_test_data_subarena.capacity = suballocation_data->total_raycast_pierce_layer_test_data_count;

	scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->total_per_frame_raycast_count);
	scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->overlap_attribute_count);

	scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count, suballocation_data->total_per_frame_raycast_count);
	scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->collision_attribute_count);

	scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->total_per_frame_raycast_count);
	scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->overlap_attribute_count);

	scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count, suballocation_data->total_per_frame_raycast_count);
	scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->collision_attribute_count);

	scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, 1);
	scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count);

	scene->raycast_core.has_already_cached_inverse_collision_mesh_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count, 1);
	scene->raycast_core.has_already_cached_inverse_collision_mesh_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count);

	scene->raycast_core.m4_inverse_cache_overlap_mesh_buffer.size = suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count;
	scene->raycast_core.m4_inverse_cache_collision_mesh_buffer.size = suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count;
	////////////////////////////////////////// raycast core //////////////////////////////////////////


	scene->total_chained_entity_count = suballocation_data->total_entity_count;
	scene->total_entity_binding_chain_count = suballocation_data->total_entity_count / 2;
	scene->total_binding_chain_index_count = suballocation_data->total_entity_count;
	scene->total_has_already_assembled_transform_matrix_bitmask_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count);
	scene->has_already_assembled_transform_matrix_bitmask_per_entity_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count);
	scene->total_chained_entity_transform_matrix_count = suballocation_data->total_entity_count;
	scene->total_already_part_of_other_binding_chain_bitmask_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count);
	scene->already_part_of_other_binding_chain_bitmask_count_for_total_entity_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count);
	scene->total_entity_children_index_count = suballocation_data->total_entity_count * suballocation_data->entity_children_index_per_entity_count;
	scene->entity_children_index_per_entity_count = suballocation_data->entity_children_index_per_entity_count;
	scene->total_binding_chain_depth_count = suballocation_data->total_entity_count;
	scene->total_has_already_written_into_binding_chain_bitmask_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count);
	scene->has_already_written_into_binding_chain_bitmask_count_for_total_entity_count = SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count);
	

	////////////////////////////////////////// overlap_attribute core //////////////////////////////////////////
	scene->overlap_attribute_core.overlap_attribute_buffer.capacity = suballocation_data->overlap_attribute_count;

	scene->overlap_attribute_core.overlap_mesh_batch_buffer.capacity = suballocation_data->overlap_attribute_count;
	scene->overlap_attribute_core.overlap_mesh_batch_buffer.count_per = suballocation_data->overlap_mesh_per_batch_count;

	scene->overlap_attribute_core.overlap_mesh_buffer.capacity = suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count;

	scene->overlap_attribute_core.overlap_grid_cell_row_index_buffer.capacity = suballocation_data->overlap_attribute_count * suballocation_data->overlap_attribute_grid_cell_row_index_per_attribute_count;
	scene->overlap_attribute_core.overlap_grid_cell_row_index_buffer.count_per = suballocation_data->overlap_attribute_grid_cell_row_index_per_attribute_count;

	scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->overlap_attribute_count);
	scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->overlap_attribute_count);

	scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->overlap_attribute_count);
	scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(suballocation_data->overlap_attribute_count);

	scene->overlap_attribute_core.overlap_layer_index_buffer.size = suballocation_data->overlap_attribute_count * 64;
	
	scene->overlap_attribute_core.overlap_attribute_index_buffer.size = suballocation_data->overlap_attribute_count * suballocation_data->overlapped_attribute_index_per_game_side_entity_count;
	scene->overlap_attribute_core.overlap_attribute_index_buffer.count_per = suballocation_data->overlapped_attribute_index_per_game_side_entity_count;

	scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.capacity = suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count;
	////////////////////////////////////////// overlap_attribute core //////////////////////////////////////////


	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_entity_t, scene->total_entity_count, scene->entity_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, sunder_v3_t, scene->total_entity_count, scene->position_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, sunder_v3_t, scene->total_entity_count, scene->scale_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, sunder_quat_t, scene->total_entity_count, scene->quat_rotation_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_mesh_binding_offsets_t, scene->total_mesh_count, scene->mesh_binding_offsets);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_mesh_binding_t, scene->total_instance_model_count, scene->mesh_binding_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_mesh_binding_metadata_t, scene->total_mesh_count, scene->mesh_binding_metadata_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->total_chained_entity_count, scene->chained_entity_index_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_entity_binding_chain_t, scene->total_entity_binding_chain_count, scene->entity_binding_chain_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->total_binding_chain_index_count, scene->entity_binding_chain_index_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->total_has_already_assembled_transform_matrix_bitmask_count, scene->has_already_assembled_transform_matrix_bitmask_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, sunder_m4_t, scene->total_chained_entity_transform_matrix_count, scene->chained_entity_transform_matrix_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->total_already_part_of_other_binding_chain_bitmask_count, scene->already_part_of_other_binding_chain_bitmask_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->total_entity_children_index_count, scene->entity_children_index_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->total_binding_chain_depth_count, scene->binding_chain_depth_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->total_has_already_written_into_binding_chain_bitmask_count, scene->has_already_written_into_binding_chain_bitmask_buffer);


	////////////////////////////////////////// raycast core //////////////////////////////////////////
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->raycast_core.raycast_grid_cell_index_subarena.capacity, scene->raycast_core.raycast_grid_cell_index_subarena.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_raycast_pierce_layer_test_data_t, scene->raycast_core.pierce_layer_test_data_subarena.capacity, scene->raycast_core.pierce_layer_test_data_subarena.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.capacity, scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.capacity, scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.capacity, scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.capacity, scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer.capacity, scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->raycast_core.has_already_cached_inverse_collision_mesh_bit_buffer.capacity, scene->raycast_core.has_already_cached_inverse_collision_mesh_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_m4_cache_t, scene->raycast_core.m4_inverse_cache_overlap_mesh_buffer.size, scene->raycast_core.m4_inverse_cache_overlap_mesh_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_m4_cache_t, scene->raycast_core.m4_inverse_cache_collision_mesh_buffer.size, scene->raycast_core.m4_inverse_cache_collision_mesh_buffer.buffer);
	////////////////////////////////////////// raycast core //////////////////////////////////////////


	////////////////////////////////////////// overlap_attribute core //////////////////////////////////////////
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_overlap_attribute_t, scene->overlap_attribute_core.overlap_attribute_buffer.capacity, scene->overlap_attribute_core.overlap_attribute_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_overlap_mesh_batch_t, scene->overlap_attribute_core.overlap_mesh_batch_buffer.capacity, scene->overlap_attribute_core.overlap_mesh_batch_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_overlap_mesh_t, scene->overlap_attribute_core.overlap_mesh_buffer.capacity, scene->overlap_attribute_core.overlap_mesh_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->overlap_attribute_core.overlap_grid_cell_row_index_buffer.capacity, scene->overlap_attribute_core.overlap_grid_cell_row_index_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer.capacity, scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer.capacity, scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->overlap_attribute_core.overlap_layer_index_buffer.size, scene->overlap_attribute_core.overlap_layer_index_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->overlap_attribute_core.overlap_attribute_index_buffer.size, scene->overlap_attribute_core.overlap_attribute_index_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.capacity, scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.buffer);
	////////////////////////////////////////// overlap_attribute core //////////////////////////////////////////


	////////////////////////////////////////// collision_attribute core //////////////////////////////////////////
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_collision_attribute_t, scene->collision_attribute_core.collision_attribute_buffer.capacity, scene->collision_attribute_core.collision_attribute_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_collision_mesh_batch_t, scene->collision_attribute_core.collision_mesh_batch_buffer.capacity, scene->collision_attribute_core.collision_mesh_batch_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_collision_mesh_t, scene->collision_attribute_core.collision_mesh_buffer.capacity, scene->collision_attribute_core.collision_mesh_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->collision_attribute_core.collision_layer_index_buffer.size, scene->collision_attribute_core.collision_layer_index_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->collision_attribute_core.collision_grid_cell_row_index_buffer.capacity, scene->collision_attribute_core.collision_grid_cell_row_index_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->collision_attribute_core.has_already_collided_with_bit_buffer.capacity, scene->collision_attribute_core.has_already_collided_with_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u64, scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.capacity, scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->collision_attribute_core.filtered_collision_mesh_index_buffer.capacity, scene->collision_attribute_core.filtered_collision_mesh_index_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_trace_step_t, scene->collision_attribute_core.trace_step_buffer.capacity, scene->collision_attribute_core.trace_step_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, u32, scene->collision_attribute_core.trace_step_grid_cell_index_buffer.capacity, scene->collision_attribute_core.trace_step_grid_cell_index_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_trace_data_t, scene->collision_attribute_core.trace_data_buffer.capacity, scene->collision_attribute_core.trace_data_buffer.buffer);
	////////////////////////////////////////// collision_attribute core //////////////////////////////////////////


	////////////////////////////////////////// whatever this shit belonges to core xddd //////////////////////////////////////////
	SUNDER_ARENA_SUBALLOCATE_DEBUG(suballocation_data->arena, lightray_collision_vertex_buffers_population_filter_data_t, scene->collision_attribute_core.collision_mesh_buffer.capacity + scene->overlap_attribute_core.overlap_mesh_buffer.capacity, scene->collision_vertex_buffers_population_filter_data_buffer);
	////////////////////////////////////////// whatever this shit belonges to core xddd //////////////////////////////////////////


	/*
	const sunder_arena_suballocation_result_t entity_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_entity_t), scene->total_entity_count), alignof(lightray_entity_t));
	scene->entity_buffer = (lightray_entity_t*)entity_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t position_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), scene->total_entity_count), alignof(sunder_v3_t));
	scene->position_buffer = (sunder_v3_t*)position_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t rotation_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), scene->total_entity_count), alignof(sunder_v3_t));
	scene->rotation_buffer = (sunder_v3_t*)rotation_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t scale_buffer_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), scene->total_entity_count), alignof(sunder_v3_t));
	scene->scale_buffer = (sunder_v3_t*)scale_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_binding_offsets_suballocation_result = sunder_suballocate_from_arena(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_binding_offsets_t), scene->total_mesh_count), alignof(lightray_mesh_binding_offsets_t));
	scene->mesh_binding_offsets = (lightray_mesh_binding_offsets_t*)mesh_binding_offsets_suballocation_result.data;

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

	//const sunder_arena_suballocation_result_t raycast_grid_cell_index_subarena_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_raycast_grid_cell_subarena_index_count), alignof(u32));
	//scene->raycast_grid_cell_index_subarena = SUNDER_CAST(u32*, raycast_grid_cell_index_subarena_suballocation_result.data);

	//const sunder_arena_suballocation_result_t has_already_been_traced_bitmask_subarena_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_has_already_been_traced_bitmask_count), alignof(u64));
	//scene->has_already_been_traced_bitmask_subarena = SUNDER_CAST(u64*, has_already_been_traced_bitmask_subarena_suballocation_result.data);

	//const sunder_arena_suballocation_result_t raycast_pierce_layer_test_data_subarena_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_raycast_pierce_layer_test_data_t), scene->total_pierce_layer_test_data_count), alignof(lightray_raycast_pierce_layer_test_data_t));
	//scene->pierce_layer_test_data_subarena = SUNDER_CAST(lightray_raycast_pierce_layer_test_data_t*, raycast_pierce_layer_test_data_subarena_suballocation_result.data);

	const sunder_arena_suballocation_result_t quat_rotation_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_quat_t), scene->total_entity_count), alignof(sunder_quat_t));
	scene->quat_rotation_buffer = SUNDER_CAST(sunder_quat_t*, quat_rotation_buffer_suballocation_result.data);


	const sunder_arena_suballocation_result_t chained_entity_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_chained_entity_count), alignof(u32));
	scene->chained_entity_index_buffer = SUNDER_CAST(u32*, chained_entity_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t entity_binding_chain_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_entity_binding_chain_t), scene->total_entity_binding_chain_count), alignof(lightray_entity_binding_chain_t));
	scene->entity_binding_chain_buffer = SUNDER_CAST(lightray_entity_binding_chain_t*, entity_binding_chain_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t entity_binding_chain_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_binding_chain_index_count), alignof(u32));
	scene->entity_binding_chain_index_buffer = SUNDER_CAST(u32*, entity_binding_chain_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t has_already_assembled_transform_matrix_bitmask_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_has_already_assembled_transform_matrix_bitmask_count), alignof(u64));
	scene->has_already_assembled_transform_matrix_bitmask_buffer = SUNDER_CAST(u64*, has_already_assembled_transform_matrix_bitmask_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t chained_entity_transform_matrix_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(sunder_m4_t), scene->total_chained_entity_transform_matrix_count), alignof(sunder_m4_t));
	scene->chained_entity_transform_matrix_buffer = SUNDER_CAST(sunder_m4_t*, chained_entity_transform_matrix_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t already_part_of_other_binding_chain_bitmask_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_already_part_of_other_binding_chain_bitmask_count), alignof(u64));
	scene->already_part_of_other_binding_chain_bitmask_buffer = SUNDER_CAST(u64*, already_part_of_other_binding_chain_bitmask_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t entity_children_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_entity_children_index_count), alignof(u32));
	scene->entity_children_index_buffer= SUNDER_CAST(u32*, entity_children_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t binding_chain_depth_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_binding_chain_depth_count), alignof(u32));
	scene->binding_chain_depth_buffer = SUNDER_CAST(u32*, binding_chain_depth_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t has_already_written_into_binding_chain_bitmask_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_has_already_written_into_binding_chain_bitmask_count), alignof(u64));
	scene->has_already_written_into_binding_chain_bitmask_buffer = SUNDER_CAST(u64*, has_already_written_into_binding_chain_bitmask_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t closest_epa_result_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_epa_result_t), scene->total_collidable_entity_count), alignof(lightray_epa_result_t));
	scene->closest_epa_result_buffer = SUNDER_CAST(lightray_epa_result_t*, closest_epa_result_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t temp_squared_distance_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(f32), scene->total_collidable_entity_count), alignof(f32));
	scene->temp_contact_point_squared_distance_buffer = SUNDER_CAST(f32*, temp_squared_distance_buffer_suballocation_result.data);


	const sunder_arena_suballocation_result_t overlap_attribute_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_overlap_attribute_t), scene->total_overlap_attribute_count), alignof(lightray_overlap_attribute_t));
	scene->overlap_attribute_buffer = SUNDER_CAST(lightray_overlap_attribute_t*, overlap_attribute_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t overlap_mesh_batch_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_overlap_mesh_batch_t), scene->total_overlap_mesh_batch_count), alignof(lightray_overlap_mesh_batch_t));
	scene->overlap_mesh_batch_buffer = SUNDER_CAST(lightray_overlap_mesh_batch_t*, overlap_mesh_batch_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t overlap_mesh_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_overlap_mesh_t), scene->total_overlap_mesh_count), alignof(lightray_overlap_mesh_t));
	scene->overlap_mesh_buffer = SUNDER_CAST(lightray_overlap_mesh_t*, overlap_mesh_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t filtered_overlap_mesh_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_overlap_mesh_count), alignof(u32));
	scene->filtered_overlap_mesh_index_buffer = SUNDER_CAST(u32*, filtered_overlap_mesh_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t filtered_collision_mesh_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_collision_mesh_count), alignof(u32));
	scene->filtered_collision_mesh_index_buffer = SUNDER_CAST(u32*, filtered_collision_mesh_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t collision_vertex_buffers_population_filter_data_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_collision_vertex_buffers_population_filter_data_t), scene->total_collision_mesh_count + scene->total_overlap_mesh_count), alignof(lightray_collision_vertex_buffers_population_filter_data_t));
	scene->collision_vertex_buffers_population_filter_data_buffer = SUNDER_CAST(lightray_collision_vertex_buffers_population_filter_data_t*, collision_vertex_buffers_population_filter_data_buffer_suballocation_result.data);


	const sunder_arena_suballocation_result_t overlap_attribute_grid_cell_row_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_overlap_attribute_grid_cell_row_index_count), alignof(u32));
	scene->overlap_attribute_grid_cell_row_index_buffer = SUNDER_CAST(u32*, overlap_attribute_grid_cell_row_index_buffer_suballocation_result.data);


	const sunder_arena_suballocation_result_t overlap_layer_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_overlap_layer_index_count), alignof(u32));
	scene->overlap_layer_index_buffer = SUNDER_CAST(u32*, overlap_layer_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t has_already_overlapped_with_bitmask_per_overlap_attribute_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_has_already_overlapped_with_bitmask_per_overlap_attribute_count), alignof(u64));
	scene->has_already_overlapped_with_bitmask_per_overlap_attribute_buffer = SUNDER_CAST(u64*, has_already_overlapped_with_bitmask_per_overlap_attribute_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t overlapped_attribute_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), scene->total_overlapped_attribute_index_count), alignof(u32));
	scene->overlapped_attribute_index_buffer = SUNDER_CAST(u32*, overlapped_attribute_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t has_already_failed_to_overlap_bitmask_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->total_has_already_failed_to_overlap_bitmask_count), alignof(u64));
	scene->has_already_failed_to_overlap_bitmask_buffer = SUNDER_CAST(u64*, has_already_failed_to_overlap_bitmask_buffer_suballocation_result.data);


	//const sunder_arena_suballocation_result_t has_already_failed_to_trace_overlap_attribute_bit_buffer_suballocation_result = sunder_suballocate_from_arena_debug(suballocation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), scene->has_already_failed_to_trace_overlap_attribute_bit_buffer.capacity), alignof(u64));
	//scene->has_already_failed_to_trace_overlap_attribute_bit_buffer.buffer = SUNDER_CAST(u64*, has_already_failed_to_trace_overlap_attribute_bit_buffer_suballocation_result.data);
	*/


	for (u32 i = 0; i < scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.capacity; i++)
	{
		scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer[i] = UINT32_MAX;
	}

	for (u32 i = 0; i < scene->collision_attribute_core.collision_mesh_buffer.capacity; i++)
	{
		scene->collision_attribute_core.filtered_collision_mesh_index_buffer[i] = UINT32_MAX;
	}

	for (u32 i = 0; i < scene->collision_attribute_core.collision_mesh_buffer.capacity + scene->overlap_attribute_core.overlap_mesh_buffer.capacity; i++)
	{
		scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index = UINT32_MAX;
	}


	for (u32 i = 0; i < scene->total_entity_count; i++)
	{
		scene->entity_buffer[i].global_mesh_index = UINT16_MAX;
		scene->entity_buffer[i].light_attribute_index = UINT16_MAX;
		scene->entity_buffer[i].camera_attribute_index = UINT16_MAX;
		scene->entity_buffer[i].parent_index = UINT32_MAX;
		scene->entity_buffer[i].instance_model_binding_index = UINT32_MAX;
		scene->entity_buffer[i].collision_attribute_index = UINT32_MAX;
		scene->entity_buffer[i].bone_binding_index = UINT32_MAX;
		scene->entity_buffer[i].overlap_attribute_index = UINT32_MAX;
		scene->scale_buffer[i] = sunder_v3_scalar(1.0f);

		lightray_add_entity_flags(scene, i, (1 << LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_TRANSLATION_BIT) | (1 << LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_ROTATION_BIT) | (1 << LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_SCALE_BIT)); //  remove scaling shit // booooo
	}

	// remove this shit aswell

	for (u32 i = 0; i < scene->total_entity_count; i++)
	{
		SUNDER_SET_BIT(scene->visibility_flags, i, 1ull);
	}
}

u64 lightray_compute_scene_suballocation_size(const lightray_scene_suballocation_data_t* suballocation_data, u64 alignment)
{
	//const u64 entity_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_entity_t), suballocation_data->total_entity_count, alignment);
	//const u64 transform_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(glm::vec3), suballocation_data->total_entity_count * 2, alignment);
	//const u64 mesh_binding_offsets_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_offsets_t), suballocation_data->total_mesh_count, alignment);
	//const u64 mesh_binding_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_t), suballocation_data->total_instance_model_count, alignment);
	//const u64 mesh_binding_metadata_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_metadata_t), suballocation_data->total_mesh_count, alignment);

	//const u64 collision_attribute_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_attribute_t), suballocation_data->collidable_entity_count, alignment);
	//const u64 grid_cell_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collidable_entity_count * suballocation_data->grid_cell_index_per_collidable_entity_count, alignment);
	//const u64 collision_mesh_batch_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_mesh_batch_t), suballocation_data->collidable_entity_count, alignment);
	//const u64 collision_mesh_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_mesh_t), suballocation_data->collidable_entity_count * suballocation_data->collision_mesh_per_batch_count, alignment);
	//const u64 has_already_collided_with_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64),  SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->collidable_entity_count, suballocation_data->collidable_entity_count), alignment);
	//const u64 collision_layer_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collidable_entity_count * 64, alignment);
	//const u64 game_side_entity_kind_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64),  SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->game_side_entity_kind_count, suballocation_data->collidable_entity_count), alignment);


	/////////////////////////////////////////////////// raycast_core ///////////////////////////////////////////////////
	//const u64 raycast_grid_cell_index_subarena_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_raycast_grid_cell_index_count, alignment);
	//const u64 pierce_layer_test_data_subarena_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_raycast_pierce_layer_test_data_t), suballocation_data->total_raycast_pierce_layer_test_data_count, alignment);
	//const u64 has_already_been_traced_overlap_attribute_bit_buffer_suballocation_size = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->total_per_frame_raycast_count), alignment);
	//const u64 has_already_been_traced_collision_attribute_bit_buffer_suballocation_size = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->collidable_entity_count, suballocation_data->total_per_frame_raycast_count), alignment);
	//const u64 has_already_failed_to_trace_overlap_attribute_bit_buffer_suballocation_size = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->total_per_frame_raycast_count), alignment);
	//const u64 has_already_failed_to_trace_collision_attribute_bit_buffer_suballocation_size = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->collidable_entity_count, suballocation_data->total_per_frame_raycast_count), alignment);
	//const u64 overlap_mesh_index_buffer_sublallocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, alignment);
	//const u64 collision_mesh_index_buffer_sublallocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collidable_entity_count * suballocation_data->collision_mesh_per_batch_count, alignment);
	//const u64 overlap_mesh_m4_inverse_cache_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_m4_cache_t), suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, alignment);
	//const u64 collision_mesh_m4_inverse_cache_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_m4_cache_t), suballocation_data->collidable_entity_count * suballocation_data->collision_mesh_per_batch_count, alignment);
	//const u64 has_already_cached_inverse_overlap_attribute_bit_buffer_suballocation_size = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->overlap_attribute_count), alignment);
	//const u64 has_already_cached_inverse_collision_attribute_bit_buffer_suballocation_size = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->collidable_entity_count, suballocation_data->collidable_entity_count), alignment);
	/////////////////////////////////////////////////// raycast_core ///////////////////////////////////////////////////


	//const u64 quat_rotation_buffer = sunder_compute_aligned_allocation_size(sizeof(sunder_quat_t), suballocation_data->total_entity_count, alignment);

	//const u64 chained_entity_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count, alignment);
	//const u64 entity_binding_chain_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_entity_binding_chain_t), suballocation_data->total_entity_count / 2, alignment);
	//const u64 entity_binding_chain_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count, alignment);
	//const u64 has_already_assembled_transform_matrix_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count), alignment);
	//const u64 chained_entity_transform_matrix_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(sunder_m4_t), suballocation_data->total_entity_count, alignment);
	//const u64 already_part_of_other_binding_chain_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count), alignment);
	//const u64 entity_children_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count * suballocation_data->entity_children_index_per_entity_count, alignment);
	//const u64 binding_chain_depth_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count, alignment);
	//const u64 has_already_written_into_binding_chain_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count), alignment);
	//const u64 closest_epa_result_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_epa_result_t), suballocation_data->collidable_entity_count, alignment);
	//const u64 temp_squared_distance_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(f32), suballocation_data->collidable_entity_count, alignment);


	//const u64 overlap_attribute_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_overlap_attribute_t), suballocation_data->overlap_attribute_count, alignment);
	//const u64 overlap_mesh_batch_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_overlap_mesh_batch_t), suballocation_data->overlap_attribute_count, alignment);
	//const u64 overlap_mesh_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_overlap_mesh_t), suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, alignment);

	//const u64 filtered_overlap_mesh_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, alignment);
	//const u64 filtered_collision_mesh_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collidable_entity_count * suballocation_data->collision_mesh_per_batch_count, alignment);
	//const u64 collision_vertex_buffers_population_filter_data_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), (suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count) + (suballocation_data->collidable_entity_count * suballocation_data->collision_mesh_per_batch_count), alignment);

	//const u64 overlap_attribute_grid_row_cell_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * suballocation_data->overlap_attribute_grid_cell_row_index_per_attribute_count, alignment);
	//const u64 grid_overlap_attribute_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_per_cell_count * suballocation_data->grid_cell_count, alignment);

	//const u64 grid_collision_attribute_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collision_attribute_per_cell_count * suballocation_data->grid_cell_count, alignment);

	//const u64 has_already_overlapped_with_bitmask_per_overlap_attribute_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->overlap_attribute_count) * suballocation_data->overlap_attribute_count, alignment);
	//const u64 overlap_layer_index_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * 64, alignment);
	//const u64 overlapped_attribute_per_attribute_index_count = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * suballocation_data->overlapped_attribute_index_per_game_side_entity_count, alignment);

	//const u64 grid_cell_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_grid_cell_t), suballocation_data->grid_cell_count, alignment);

	//const u64 has_already_failed_to_overlap_bitmask_buffer_suballocation_size = sunder_compute_aligned_allocation_size(sizeof(u64),SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->overlap_attribute_count, suballocation_data->overlap_attribute_count), alignment);

	//const u64 has_already_failed_to_trace_overlap_attribute_bit_buffer_suballocation_size = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->total_per_frame_raycast_count), alignment);



	const u64 wacky_whatever_the_fuck_the_buffer_size_is = 80u;
	u64 aligned_allocation_size_buffer[wacky_whatever_the_fuck_the_buffer_size_is]{};
	/////////////////////////////////////////////////////// entity_core ///////////////////////////////////////////////////////
	aligned_allocation_size_buffer[0] = sunder_compute_aligned_allocation_size(sizeof(lightray_entity_t), suballocation_data->total_entity_count, alignment); // entity_buffer
	aligned_allocation_size_buffer[1] = sunder_compute_aligned_allocation_size(sizeof(glm::vec3), suballocation_data->total_entity_count * 2, alignment); // transform_buffer // position, scale
	aligned_allocation_size_buffer[2] = sunder_compute_aligned_allocation_size(sizeof(sunder_quat_t), suballocation_data->total_entity_count, alignment); // quat_rotation_buffer 
	aligned_allocation_size_buffer[3] = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_offsets_t), suballocation_data->total_mesh_count, alignment); // mesh_binding_offsets;
	aligned_allocation_size_buffer[4] = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_t), suballocation_data->total_instance_model_count, alignment); // mesh_binding_buffer;
	aligned_allocation_size_buffer[5] = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_binding_metadata_t), suballocation_data->total_mesh_count, alignment); // mesh_binding_metadata_buffer
	aligned_allocation_size_buffer[6] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count, alignment); // chained_entity_index_buffer
	aligned_allocation_size_buffer[7] = sunder_compute_aligned_allocation_size(sizeof(lightray_entity_binding_chain_t), suballocation_data->total_entity_count / 2, alignment); // entity_binding_chain_buffer
	aligned_allocation_size_buffer[8] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count, alignment); // entity_binding_chain_index_buffer
	aligned_allocation_size_buffer[9] = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count), alignment); // has_already_assembled_transform_matrix_bitmask_buffer
	aligned_allocation_size_buffer[10] = sunder_compute_aligned_allocation_size(sizeof(sunder_m4_t), suballocation_data->total_entity_count, alignment); // chained_entity_transform_matrix_buffer
	aligned_allocation_size_buffer[11] = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count), alignment); // already_part_of_other_binding_chain_bitmask_buffer
	aligned_allocation_size_buffer[12] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count * suballocation_data->entity_children_index_per_entity_count, alignment); // entity_children_index_buffer
	aligned_allocation_size_buffer[13] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_entity_count, alignment); // binding_chain_depth_buffer
	aligned_allocation_size_buffer[14] = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->total_entity_count), alignment); // has_already_written_into_binding_chain_bitmask_buffer
	/////////////////////////////////////////////////////// entity_core ///////////////////////////////////////////////////////

	/////////////////////////////////////////////////////// collision_attribute_core ///////////////////////////////////////////////////////
	aligned_allocation_size_buffer[15] = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_attribute_t), suballocation_data->collision_attribute_count, alignment); // collision_attribute_buffer;
	aligned_allocation_size_buffer[16] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collision_attribute_count * suballocation_data->grid_cell_index_per_collision_attribute_count, alignment); // collision_attribute_grid_cell_row_index_buffer
	aligned_allocation_size_buffer[17] = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_mesh_batch_t), suballocation_data->collision_attribute_count, alignment); // collision_mesh_batch_buffer
	aligned_allocation_size_buffer[18] = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_mesh_t), suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count, alignment); // collision_mesh_buffer;
	aligned_allocation_size_buffer[19] = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->collision_attribute_count, suballocation_data->collision_attribute_count), alignment); // has_already_collided_with_bitmask_buffer;
	aligned_allocation_size_buffer[20] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collision_attribute_count * 64, alignment); // collision_layer_index_buffer;
	aligned_allocation_size_buffer[21] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collision_attribute_per_cell_count * suballocation_data->grid_cell_count, alignment); // grid_collision_attribute_index_buffer
	aligned_allocation_size_buffer[22] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count, alignment); // filtered_collision_mesh_index_buffer
	aligned_allocation_size_buffer[23] = sunder_compute_aligned_allocation_size(sizeof(lightray_trace_step_t), suballocation_data->collision_attribute_count * LIGHTRAY_MAX_TRACE_STEP_COUNT, alignment); // trace_step_buffer
	aligned_allocation_size_buffer[24] = sunder_compute_aligned_allocation_size(sizeof(u32), (suballocation_data->collision_attribute_count * LIGHTRAY_MAX_TRACE_STEP_COUNT) * suballocation_data->collision_attribute_per_cell_count, alignment); // trace_step_grid_cell_index_buffer
	aligned_allocation_size_buffer[25] = sunder_compute_aligned_allocation_size(sizeof(lightray_trace_instance_t), suballocation_data->grid_cell_count * suballocation_data->collision_attribute_per_cell_count, alignment); // grid_trace_instance_buffer
	aligned_allocation_size_buffer[26] = sunder_compute_aligned_allocation_size(sizeof(lightray_trace_instance_t), suballocation_data->grid_cell_count * suballocation_data->collision_attribute_per_cell_count, alignment); // grid_trace_instance_deferred_buffer
	aligned_allocation_size_buffer[27] = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count, suballocation_data->grid_cell_count), alignment); // grid_written_into_bit_buffer
	aligned_allocation_size_buffer[28] = sunder_compute_aligned_allocation_size(sizeof(lightray_trace_data_t), suballocation_data->collision_attribute_count, alignment); // trace_data_buffer
	/////////////////////////////////////////////////////// collision_attribute_core ///////////////////////////////////////////////////////

	/////////////////////////////////////////////////////// raycast_core ///////////////////////////////////////////////////////
	aligned_allocation_size_buffer[29] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->total_raycast_grid_cell_index_count, alignment); // raycast_grid_cell_index_subarena
	aligned_allocation_size_buffer[30] = sunder_compute_aligned_allocation_size(sizeof(lightray_raycast_pierce_layer_test_data_t), suballocation_data->total_raycast_pierce_layer_test_data_count, alignment); // pierce_layer_test_data_subarena;
	aligned_allocation_size_buffer[31] = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->total_per_frame_raycast_count), alignment); // has_already_been_traced_overlap_attribute_bit_buffer
	aligned_allocation_size_buffer[32] = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count, suballocation_data->total_per_frame_raycast_count), alignment); // has_already_been_traced_collision_attribute_bit_buffer
	aligned_allocation_size_buffer[33] = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count, suballocation_data->total_per_frame_raycast_count), alignment); // // has_already_failed_to_trace_overlap_attribute_bit_buffer
	aligned_allocation_size_buffer[34] = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count, suballocation_data->total_per_frame_raycast_count), alignment); // has_already_failed_to_trace_collision_attribute_bit_buffer
	aligned_allocation_size_buffer[35] = sunder_compute_aligned_allocation_size(sizeof(lightray_m4_cache_t), suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, alignment); // overlap_mesh_m4_inverse_cache_buffer
	aligned_allocation_size_buffer[36] = sunder_compute_aligned_allocation_size(sizeof(lightray_m4_cache_t), suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count, alignment); // collision_mesh_m4_inverse_cache_buffer
	aligned_allocation_size_buffer[37] = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, 1), alignment); // has_already_cached_inverse_overlap_mesh_bit_buffer
	aligned_allocation_size_buffer[38] = sunder_compute_bit_buffer_allocation_size(sunder_compute_bit_buffer_capacity(suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count, 1), alignment); // has_already_cached_inverse_collision_mesh_bit_buffer
	/////////////////////////////////////////////////////// raycast_core ///////////////////////////////////////////////////////

	/////////////////////////////////////////////////////// overlap_attribute_core /////////////////////////////////////////////////////// 
	aligned_allocation_size_buffer[39] = sunder_compute_aligned_allocation_size(sizeof(lightray_overlap_attribute_t), suballocation_data->overlap_attribute_count, alignment); // overlap_attribute_buffer
	aligned_allocation_size_buffer[40] = sunder_compute_aligned_allocation_size(sizeof(lightray_overlap_mesh_batch_t), suballocation_data->overlap_attribute_count, alignment); // overlap_mesh_batch_buffer
	aligned_allocation_size_buffer[41] = sunder_compute_aligned_allocation_size(sizeof(lightray_overlap_mesh_t), suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, alignment); // overlap_mesh_buffer
	aligned_allocation_size_buffer[42] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count, alignment); // filtered_overlap_mesh_index_buffer
	aligned_allocation_size_buffer[43] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * suballocation_data->overlap_attribute_grid_cell_row_index_per_attribute_count, alignment); // overlap_attribute_grid_row_cell_index_buffer
	aligned_allocation_size_buffer[44] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_per_cell_count * suballocation_data->grid_cell_count, alignment); // grid_overlap_attribute_index_buffer
	aligned_allocation_size_buffer[45] = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_COUNT(suballocation_data->overlap_attribute_count) * suballocation_data->overlap_attribute_count, alignment); // has_already_overlapped_with_bitmask_buffer
	aligned_allocation_size_buffer[46] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * 64, alignment); // overlap_layer_index_buffer
	aligned_allocation_size_buffer[47] = sunder_compute_aligned_allocation_size(sizeof(u32), suballocation_data->overlap_attribute_count * suballocation_data->overlapped_attribute_index_per_game_side_entity_count, alignment); // overlap_attribute_index_buffer (for game side entities)
	aligned_allocation_size_buffer[48] = sunder_compute_aligned_allocation_size(sizeof(u64), SUNDER_COMPUTE_BUFFERED_BIT_TOTAL_COUNT(suballocation_data->overlap_attribute_count, suballocation_data->overlap_attribute_count), alignment); // has_already_failed_to_overlap_bitmask_buffer
	/////////////////////////////////////////////////////// overlap_attribute_core /////////////////////////////////////////////////////// 

	/////////////////////////////////////////////////////// lightray_core /////////////////////////////////////////////////////// 
	aligned_allocation_size_buffer[49] = sunder_compute_aligned_allocation_size(sizeof(lightray_collision_vertex_buffers_population_filter_data_t), (suballocation_data->overlap_attribute_count * suballocation_data->overlap_mesh_per_batch_count) + (suballocation_data->collision_attribute_count * suballocation_data->collision_mesh_per_batch_count), alignment); // collision_vertex_buffers_population_filter_data_buffer
	aligned_allocation_size_buffer[50] = sunder_compute_aligned_allocation_size(sizeof(lightray_grid_cell_t), suballocation_data->grid_cell_count, alignment); // grid_cell_buffer
	/////////////////////////////////////////////////////// lightray_core /////////////////////////////////////////////////////// 

	return sunder_accumulate_aligned_allocation_size(aligned_allocation_size_buffer, wacky_whatever_the_fuck_the_buffer_size_is, SUNDER_CAST(u32, alignment));

	//return  sunder_align64(entity_buffer_suballocation_size + transform_buffer_suballocation_size + mesh_binding_offsets_suballocation_size + mesh_binding_buffer_suballocation_size + mesh_binding_metadata_buffer_suballocation_size + grid_cell_index_buffer_suballocation_size + collision_attribute_buffer_suballocation_size + collision_mesh_batch_buffer_suballocation_size + collision_mesh_buffer_suballocation_size + has_already_collided_with_bitmask_buffer_suballocation_size + collision_layer_index_buffer_suballocation_size + game_side_entity_kind_bitmask_buffer_suballocation_size + quat_rotation_buffer + chained_entity_index_buffer_suballocation_size + entity_binding_chain_buffer_suballocation_size + entity_binding_chain_index_buffer_suballocation_size + has_already_assembled_transform_matrix_bitmask_buffer_suballocation_size + chained_entity_transform_matrix_buffer_suballocation_size + already_part_of_other_binding_chain_bitmask_buffer_suballocation_size + entity_children_index_buffer_suballocation_size + binding_chain_depth_buffer_suballocation_size + has_already_written_into_binding_chain_bitmask_buffer_suballocation_size + closest_epa_result_buffer_suballocation_size + temp_squared_distance_buffer_suballocation_size + overlap_attribute_buffer_suballocation_size + overlap_mesh_batch_buffer_suballocation_size + overlap_mesh_buffer_suballocation_size + filtered_overlap_mesh_index_buffer_suballocation_size + filtered_collision_mesh_index_buffer_suballocation_size + collision_vertex_buffers_population_filter_data_buffer_suballocation_size + overlap_attribute_grid_row_cell_index_buffer_suballocation_size + grid_overlap_attribute_index_buffer_suballocation_size + grid_collision_attribute_index_buffer_suballocation_size + has_already_overlapped_with_bitmask_per_overlap_attribute_buffer_suballocation_size + overlap_layer_index_buffer_suballocation_size + overlapped_attribute_per_attribute_index_count + grid_cell_buffer_suballocation_size + has_already_failed_to_overlap_bitmask_buffer_suballocation_size, alignment);
}

lightray_entity_creation_result_t lightray_create_entity(lightray_scene_t* scene, lightray_entity_kind kind, u16 flags)
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
	scene->entity_buffer[scene->current_entity_count].flags |= flags;
	scene->current_entity_count++;

	return res;
}

lightray_mesh_binding_result_t lightray_bind_mesh(lightray_scene_t* scene, u32 entity_index, u16 mesh_index, lightray_render_target_kind kind)
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

	scene->entity_buffer[entity_index].global_mesh_index = mesh_index;
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

u32 lightray_get_entity_children_index_buffer_offset(const lightray_scene_t* scene, u32 entity_index)
{
	return entity_index * scene->entity_children_index_per_entity_count;
}

void lightray_bind_entity(lightray_scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index)
{
	if (!sunder_valid_index(entity_to_bind_index, scene->total_entity_count) || !sunder_valid_index(entity_to_bind_to_index, scene->total_entity_count))
	{
		return;
	}

	const u32 entity_children_count = scene->entity_buffer[entity_to_bind_to_index].children_index_count;

	if (sunder_valid_index(entity_children_count, scene->entity_children_index_per_entity_count))
	{
		if(!(sunder_valid_index(scene->current_chained_entity_count + 1, scene->total_chained_entity_count))) // +1 because it's an actual count, validity of doubly increment index will be checked upon next call
		{
			return;
		}

		scene->chained_entity_index_buffer[scene->current_chained_entity_count] = entity_to_bind_to_index;
		scene->chained_entity_index_buffer[scene->current_chained_entity_count + 1] = entity_to_bind_index;
		scene->current_chained_entity_count += 2;

		scene->entity_buffer[entity_to_bind_index].parent_index = entity_to_bind_to_index;
		scene->entity_children_index_buffer[lightray_get_entity_children_index_buffer_offset(scene, entity_to_bind_to_index) + entity_children_count] = entity_to_bind_index;
		scene->entity_buffer[entity_to_bind_to_index].children_index_count++;
	}
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

void lightray_compute_interpolated_skeleton_transform(lightray_animation_core_t* animation_core, u32 animation_playback_index)
{
	const u16 animation_index = animation_core->playback_command_buffer[animation_playback_index].animation_index;
	const u16 skeleton_index = animation_core->playback_command_buffer[animation_playback_index].skeletal_mesh_index;
	const u16 instance_index = animation_core->playback_command_buffer[animation_playback_index].instance_index;

	const u32 current_node_count = animation_core->skeleton_buffer[skeleton_index].node_count;
	const u32 current_node_buffer_offset = animation_core->skeleton_buffer[skeleton_index].node_buffer_offset;
	const u32 current_skeletal_mesh_bone_buffer_offset = animation_core->skeleton_buffer[skeleton_index].bone_buffer_offset;
	const u32 current_animation_channel_buffer_offset = animation_core->animation_buffer[animation_index].channel_buffer_offset;
	const u32 final_bone_matrices_buffer_offset_with_respect_to_instance = lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(instance_index, animation_core->skeleton_buffer[skeleton_index].bone_count, animation_core->skeleton_buffer[skeleton_index].computed_bone_transform_matrix_buffer_offset);

	const sunder_quat_t no_rotation_quat = sunder_quat_raw(0.0f, 0.0f, 0.0f, 1.0f);

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

			//const glm::mat4 identity = glm::mat4(1.0f);
			//const glm::mat4 scale_matrix = glm::scale(identity, scale_key);
			const glm::mat4 rotation_matrix = glm::toMat4(rotation_key);
			//const glm::mat4 translation_matrix = glm::translate(identity, position_key);

			const sunder_v3_t sv = sunder_v3(scale_key.x, scale_key.y, scale_key.z);
			const sunder_quat_t rq = sunder_quat_raw(rotation_key.x, rotation_key.y, rotation_key.z, rotation_key.w);
			const sunder_v3_t pv = sunder_v3(position_key.x, position_key.y, position_key.z);

			sunder_m4_t m{};

			if (rq == no_rotation_quat)
			{
				m = sunder_m4_ts(pv, sv);
			}

			else
			{
				m = sunder_m4_model(pv, rq, sv);
			}

			//const sunder_m4_t t = sunder_m4_translation(pv);
			//const sunder_m4_t r = sunder_m4_rotation(rq);
			//const sunder_m4_t s = sunder_m4_scale(sv);

			//const sunder_m4_t m = t * r * s;
			//const sunder_m4_t m = sunder_m4_model(pv, rq, sv);
			//const glm::mat4 m_glm = translation_matrix * rotation_matrix * scale_matrix;

			/*
			SUNDER_LOG("\n\nt\n");
			sunder_log_m4(t);
			SUNDER_LOG("\n\nr\n");
			sunder_log_m4(r);
			SUNDER_LOG("\n\ns\n");
			sunder_log_m4(s);

			SUNDER_LOG("\n\nglm_m\n");
			lightray_log_glm_matrix(translation_matrix);
			SUNDER_LOG("\n\nglm_r\n");
			lightray_log_glm_matrix(rotation_matrix);
			SUNDER_LOG("\n\nglm_s\n");
			lightray_log_glm_matrix(scale_matrix);

			SUNDER_LOG("\n\ns");
			sunder_log_quat(rq);
			SUNDER_LOG("\n\ng");
			SUNDER_LOG(rotation_key.x);
			SUNDER_LOG(", ");
			SUNDER_LOG(rotation_key.y);
			SUNDER_LOG(", ");
			SUNDER_LOG(rotation_key.z);
			SUNDER_LOG(", ");
			SUNDER_LOG(rotation_key.w);
			SUNDER_LOG(", ");
			*/

			local_transform_matrix = lightray_copy_sunder_m4_to_glm(m);
			//local_transform_matrix = translation_matrix * rotation_matrix * scale_matrix;
		}

		animation_core->node_buffer[current_node_index].global_transform_matrix = animation_core->node_buffer[current_parent_index].global_transform_matrix * local_transform_matrix;

		if (current_bone_buffer_index != LIGHTRAY_INVALID_NODE_BONE_BUFFER_INDEX)
		{
			animation_core->computed_bone_matrix_buffer[final_bone_matrices_buffer_offset_with_respect_to_instance + current_bone_buffer_index] = lightray_copy_sunder_m4_to_glm(lightray_copy_glm_mat4_to_sunder(animation_core->global_root_inverse_matrix_buffer[skeleton_index]) * lightray_copy_glm_mat4_to_sunder(animation_core->node_buffer[current_node_index].global_transform_matrix) * lightray_copy_glm_mat4_to_sunder(animation_core->bone_buffer[current_skeletal_mesh_bone_buffer_offset + current_bone_buffer_index].inverse_bind_pose_matrix));
		}
	}
}

void lightray_blend_skeleton_transform(lightray_animation_core_t* animation_core, u32 animation_playback_index_a, u32 animation_playback_index_b, f32 blend_factor)
{
	const f32 blend = sunder_clamp_f32(0.0f, 1.0f, blend_factor);

	const u32 skeleton_index = animation_core->playback_command_buffer[animation_playback_index_a].skeletal_mesh_index;
	const u32 instance_index = animation_core->playback_command_buffer[animation_playback_index_a].instance_index;
	const u32 animation_index_a = animation_core->playback_command_buffer[animation_playback_index_a].animation_index;
	const u32 animation_index_b = animation_core->playback_command_buffer[animation_playback_index_b].animation_index;

	const u32 node_count = animation_core->skeleton_buffer[skeleton_index].node_count;
	const u32 node_buffer_offset = animation_core->skeleton_buffer[skeleton_index].node_buffer_offset;
	const u32 bone_buffer_offset = animation_core->skeleton_buffer[skeleton_index].bone_buffer_offset;
	const u32 channel_buffer_offset_a = animation_core->animation_buffer[animation_index_a].channel_buffer_offset;
	const u32 channel_buffer_offset_b = animation_core->animation_buffer[animation_index_b].channel_buffer_offset;
	const u32 final_bone_matrices_buffer_offset_with_respect_to_instance = lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(instance_index, animation_core->skeleton_buffer[skeleton_index].bone_count, animation_core->skeleton_buffer[skeleton_index].computed_bone_transform_matrix_buffer_offset);
	
	const f32 ticks_a = animation_core->playback_command_buffer[animation_playback_index_a].ticks;
	const f32 ticks_b = animation_core->playback_command_buffer[animation_playback_index_b].ticks;

	const sunder_quat_t no_rotation_quat = sunder_quat_raw(0.0f, 0.0f, 0.0f, 1.0f);

	sunder_m4_t local_m{};

	for (u32 i = 1; i < node_count; i++)
	{
		const u32 current_node_index = node_buffer_offset + i;
		const u32 current_parent_index = node_buffer_offset + animation_core->node_buffer[current_node_index].parent_index;
		const u32 current_bone_buffer_index = animation_core->node_buffer[current_node_index].bone_buffer_index;
		const u32 current_animation_channel_buffer_index = animation_core->node_buffer[current_node_index].animation_channel_buffer_index;

		local_m = lightray_copy_glm_mat4_to_sunder(animation_core->node_buffer[current_node_index].local_transform_matrix);

		if (current_animation_channel_buffer_index != LIGHTRAY_INVALID_NODE_ANIMATION_CHANNEL_BUFFER_INDEX)
		{
			const u32 current_animation_channel_index_a = channel_buffer_offset_a + current_animation_channel_buffer_index;
			const u32 current_animation_channel_index_b = channel_buffer_offset_b + current_animation_channel_buffer_index;
			const lightray_animation_channel_t* current_animation_channel_a = &animation_core->animation_channel_buffer[current_animation_channel_index_a];
			const lightray_animation_channel_t* current_animation_channel_b = &animation_core->animation_channel_buffer[current_animation_channel_index_b];

			const glm::vec3 scale_key_a = lightray_compute_interpolated_animation_channel_scale_key(current_animation_channel_a, animation_core->animation_scale_key_buffer, ticks_a);
			const glm::quat rotation_key_a = lightray_compute_interpolated_animation_channel_rotation_key(current_animation_channel_a, animation_core->animation_rotation_key_buffer, ticks_a);
			const glm::vec3 position_key_a = lightray_compute_interpolated_animation_channel_position_key(current_animation_channel_a, animation_core->animation_position_key_buffer, ticks_a);

			const glm::vec3 scale_key_b = lightray_compute_interpolated_animation_channel_scale_key(current_animation_channel_b, animation_core->animation_scale_key_buffer, ticks_b);
			const glm::quat rotation_key_b = lightray_compute_interpolated_animation_channel_rotation_key(current_animation_channel_b, animation_core->animation_rotation_key_buffer, ticks_b);
			const glm::vec3 position_key_b = lightray_compute_interpolated_animation_channel_position_key(current_animation_channel_b, animation_core->animation_position_key_buffer, ticks_b);

			const sunder_v3_t sunder_scale_key_a = lightray_glm_vec3_to_sunder(scale_key_a);
			const sunder_v3_t sunder_scale_key_b = lightray_glm_vec3_to_sunder(scale_key_b);

			const sunder_quat_t sunder_rotation_key_a = sunder_quat_raw(rotation_key_a.x, rotation_key_a.y, rotation_key_a.z, rotation_key_a.w);
			const sunder_quat_t sunder_rotation_key_b = sunder_quat_raw(rotation_key_b.x, rotation_key_b.y, rotation_key_b.z, rotation_key_b.w);

			const sunder_v3_t sunder_position_key_a = lightray_glm_vec3_to_sunder(position_key_a);
			const sunder_v3_t sunder_position_key_b = lightray_glm_vec3_to_sunder(position_key_b);

			const sunder_v3_t scale_blended = sunder_lerp_v3(sunder_scale_key_a, sunder_scale_key_b, blend);
			const sunder_v3_t position_blended = sunder_lerp_v3(sunder_position_key_a, sunder_position_key_b, blend);

			sunder_m4_t m{};

			if (sunder_rotation_key_a == no_rotation_quat && sunder_rotation_key_b == no_rotation_quat)
			{
				m = sunder_m4_ts(position_blended, scale_blended);
			}

			else
			{
				const sunder_quat_t rotation_blended = sunder_quat_slerp(sunder_rotation_key_a, sunder_rotation_key_b, blend);
				m = sunder_m4_model(position_blended, rotation_blended, scale_blended);
			}

			local_m = m;
		}

		animation_core->node_buffer[current_node_index].global_transform_matrix = lightray_copy_sunder_m4_to_glm(lightray_copy_glm_mat4_to_sunder(animation_core->node_buffer[current_parent_index].global_transform_matrix) * local_m);
		
		if (current_bone_buffer_index != LIGHTRAY_INVALID_NODE_BONE_BUFFER_INDEX)
		{
			animation_core->computed_bone_matrix_buffer[final_bone_matrices_buffer_offset_with_respect_to_instance + current_bone_buffer_index] = lightray_copy_sunder_m4_to_glm(lightray_copy_glm_mat4_to_sunder(animation_core->global_root_inverse_matrix_buffer[skeleton_index]) * lightray_copy_glm_mat4_to_sunder(animation_core->node_buffer[current_node_index].global_transform_matrix) * lightray_copy_glm_mat4_to_sunder(animation_core->bone_buffer[bone_buffer_offset + current_bone_buffer_index].inverse_bind_pose_matrix));
		}
	}
}

void lightray_blend_skeleton_transform_additive(lightray_animation_core_t* animation_core, u32 animation_playback_index_a, u32 animation_playback_index_b, f32 blend_factor)
{
	const f32 blend = sunder_clamp_f32(0.0f, 1.0f, blend_factor);

	const u32 skeleton_index = animation_core->playback_command_buffer[animation_playback_index_a].skeletal_mesh_index;
	const u32 instance_index = animation_core->playback_command_buffer[animation_playback_index_a].instance_index;
	const u32 animation_index_a = animation_core->playback_command_buffer[animation_playback_index_a].animation_index;
	const u32 animation_index_b = animation_core->playback_command_buffer[animation_playback_index_b].animation_index;

	const u32 node_count = animation_core->skeleton_buffer[skeleton_index].node_count;
	const u32 node_buffer_offset = animation_core->skeleton_buffer[skeleton_index].node_buffer_offset;
	const u32 bone_buffer_offset = animation_core->skeleton_buffer[skeleton_index].bone_buffer_offset;
	const u32 channel_buffer_offset_a = animation_core->animation_buffer[animation_index_a].channel_buffer_offset;
	const u32 channel_buffer_offset_b = animation_core->animation_buffer[animation_index_b].channel_buffer_offset;
	const u32 final_bone_matrices_buffer_offset_with_respect_to_instance = lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(instance_index, animation_core->skeleton_buffer[skeleton_index].bone_count, animation_core->skeleton_buffer[skeleton_index].computed_bone_transform_matrix_buffer_offset);

	const f32 ticks_a = animation_core->playback_command_buffer[animation_playback_index_a].ticks;
	const f32 ticks_b = animation_core->playback_command_buffer[animation_playback_index_b].ticks;

	const sunder_quat_t no_rotation_quat = sunder_quat_raw(0.0f, 0.0f, 0.0f, 1.0f);

	sunder_m4_t local_m{};

	for (u32 i = 1; i < node_count; i++)
	{
		const u32 current_node_index = node_buffer_offset + i;
		const u32 current_parent_index = node_buffer_offset + animation_core->node_buffer[current_node_index].parent_index;
		const u32 current_bone_buffer_index = animation_core->node_buffer[current_node_index].bone_buffer_index;
		const u32 current_animation_channel_buffer_index = animation_core->node_buffer[current_node_index].animation_channel_buffer_index;

		local_m = lightray_copy_glm_mat4_to_sunder(animation_core->node_buffer[current_node_index].local_transform_matrix);

		if (current_animation_channel_buffer_index != LIGHTRAY_INVALID_NODE_ANIMATION_CHANNEL_BUFFER_INDEX)
		{
			const u32 current_animation_channel_index_a = channel_buffer_offset_a + current_animation_channel_buffer_index;
			const u32 current_animation_channel_index_b = channel_buffer_offset_b + current_animation_channel_buffer_index;
			const lightray_animation_channel_t* current_animation_channel_a = &animation_core->animation_channel_buffer[current_animation_channel_index_a];
			const lightray_animation_channel_t* current_animation_channel_b = &animation_core->animation_channel_buffer[current_animation_channel_index_b];

			const glm::vec3 scale_key_a = lightray_compute_interpolated_animation_channel_scale_key(current_animation_channel_a, animation_core->animation_scale_key_buffer, ticks_a);
			const glm::quat rotation_key_a = lightray_compute_interpolated_animation_channel_rotation_key(current_animation_channel_a, animation_core->animation_rotation_key_buffer, ticks_a);
			const glm::vec3 position_key_a = lightray_compute_interpolated_animation_channel_position_key(current_animation_channel_a, animation_core->animation_position_key_buffer, ticks_a);

			const glm::vec3 scale_key_b = lightray_compute_interpolated_animation_channel_scale_key(current_animation_channel_b, animation_core->animation_scale_key_buffer, ticks_b);
			const glm::quat rotation_key_b = lightray_compute_interpolated_animation_channel_rotation_key(current_animation_channel_b, animation_core->animation_rotation_key_buffer, ticks_b);
			const glm::vec3 position_key_b = lightray_compute_interpolated_animation_channel_position_key(current_animation_channel_b, animation_core->animation_position_key_buffer, ticks_b);

			const sunder_v3_t sunder_scale_key_a = lightray_glm_vec3_to_sunder(scale_key_a);
			const sunder_v3_t sunder_scale_key_b = lightray_glm_vec3_to_sunder(scale_key_b);

			const sunder_quat_t sunder_rotation_key_a = sunder_quat_raw(rotation_key_a.x, rotation_key_a.y, rotation_key_a.z, rotation_key_a.w);
			const sunder_quat_t sunder_rotation_key_b = sunder_quat_raw(rotation_key_b.x, rotation_key_b.y, rotation_key_b.z, rotation_key_b.w);

			const sunder_v3_t sunder_position_key_a = lightray_glm_vec3_to_sunder(position_key_a);
			const sunder_v3_t sunder_position_key_b = lightray_glm_vec3_to_sunder(position_key_b);

			const sunder_v3_t delta_position = sunder_position_key_b - sunder_position_key_a;
			const sunder_v3_t blended_position = sunder_position_key_b + delta_position;

			const sunder_v3_t delta_scale = sunder_scale_key_b - sunder_scale_key_a;
			const sunder_v3_t blended_scale = sunder_scale_key_b + delta_scale;

			const sunder_quat_t delta_rotation = sunder_rotation_key_b - sunder_rotation_key_a;
			const sunder_quat_t blended_rotation = sunder_rotation_key_b + delta_rotation;

			sunder_m4_t m{};

			if (sunder_rotation_key_a == no_rotation_quat && sunder_rotation_key_b == no_rotation_quat)
			{
				m = sunder_m4_ts(blended_position, blended_scale);
			}

			else
			{
				m = sunder_m4_model(blended_position, blended_rotation, blended_scale);
			}

			local_m = m;
		}

		animation_core->node_buffer[current_node_index].global_transform_matrix = lightray_copy_sunder_m4_to_glm(lightray_copy_glm_mat4_to_sunder(animation_core->node_buffer[current_parent_index].global_transform_matrix) * local_m);

		if (current_bone_buffer_index != LIGHTRAY_INVALID_NODE_BONE_BUFFER_INDEX)
		{
			animation_core->computed_bone_matrix_buffer[final_bone_matrices_buffer_offset_with_respect_to_instance + current_bone_buffer_index] = lightray_copy_sunder_m4_to_glm(lightray_copy_glm_mat4_to_sunder(animation_core->global_root_inverse_matrix_buffer[skeleton_index]) * lightray_copy_glm_mat4_to_sunder(animation_core->node_buffer[current_node_index].global_transform_matrix) * lightray_copy_glm_mat4_to_sunder(animation_core->bone_buffer[bone_buffer_offset + current_bone_buffer_index].inverse_bind_pose_matrix));
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
	grid->collision_attribute_index_count = grid->cell_count * creation_data->collision_attribute_per_cell_count;
	grid->collision_attribute_index_count_per_cell = creation_data->collision_attribute_per_cell_count;

	grid->overlap_attribute_index_count_per_cell = creation_data->overlap_attribute_per_cell_count;
	grid->overlap_attribute_index_count = grid->cell_count * grid->overlap_attribute_index_count_per_cell;


	grid->trace_instance_buffer.capacity = grid->cell_count * creation_data->collision_attribute_per_cell_count;
	grid->trace_instance_buffer.count_per = creation_data->collision_attribute_per_cell_count;

	grid->trace_instance_deferred_buffer.capacity = grid->cell_count * creation_data->collision_attribute_per_cell_count;
	grid->trace_instance_deferred_buffer.count_per = creation_data->collision_attribute_per_cell_count;
	
	SUNDER_ARENA_SUBALLOCATE_DEBUG(creation_data->arena, lightray_grid_cell_t, grid->cell_count, grid->cell_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(creation_data->arena, u32, grid->collision_attribute_index_count, grid->collision_attribute_index_buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(creation_data->arena, u32, grid->overlap_attribute_index_count, grid->overlap_attribute_index_buffer);

	SUNDER_ARENA_SUBALLOCATE_DEBUG(creation_data->arena, lightray_trace_instance_t, grid->trace_instance_buffer.capacity, grid->trace_instance_buffer.buffer);
	SUNDER_ARENA_SUBALLOCATE_DEBUG(creation_data->arena, lightray_trace_instance_t, grid->trace_instance_deferred_buffer.capacity, grid->trace_instance_deferred_buffer.buffer);

	for (u32 i = 0; i < grid->cell_count; i++)
	{
		grid->cell_buffer[i].collision_attribute_written_bit_buffer.capacity = sunder_compute_bit_buffer_capacity(creation_data->collision_attribute_count, 1);
		grid->cell_buffer[i].collision_attribute_written_bit_buffer.count_per = sunder_compute_bit_buffer_count_per(creation_data->collision_attribute_count);

		SUNDER_ARENA_SUBALLOCATE_DEBUG(creation_data->arena, u64, grid->cell_buffer[i].collision_attribute_written_bit_buffer.capacity, grid->cell_buffer[i].collision_attribute_written_bit_buffer.buffer);
	}


	//sunder_arena_suballocation_result_t cell_buffer_suballocation_result = sunder_suballocate_from_arena_debug(creation_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_grid_cell_t), grid->cell_count), alignof(lightray_grid_cell_t));
	//grid->cell_buffer = SUNDER_CAST(lightray_grid_cell_t*, cell_buffer_suballocation_result.data);

	//sunder_arena_suballocation_result_t collision_attribute_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(creation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), grid->collision_attribute_index_count), alignof(u32));
	//grid->collision_attribute_index_buffer = SUNDER_CAST(u32*, collision_attribute_index_buffer_suballocation_result.data);


	//sunder_arena_suballocation_result_t overlap_attribute_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(creation_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), grid->overlap_attribute_index_count), alignof(u32));
	//grid->overlap_attribute_index_buffer = SUNDER_CAST(u32*, overlap_attribute_index_buffer_suballocation_result.data);


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
		grid->cell_buffer[i].collision_attribute_index_buffer_offset = i * creation_data->collision_attribute_per_cell_count;
		grid->cell_buffer[i].overlap_attribute_index_buffer_offset = i * creation_data->collision_attribute_per_cell_count;
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

u32 lightray_push_collision_attribute(lightray_scene_t* scene, u32 entity_index, u32 flags, u32 max_trace_step_count)
{
	u32 attribute_index = UINT32_MAX;

	if (sunder_valid_index(scene->collision_attribute_core.collision_attribute_buffer.size, scene->collision_attribute_core.collision_attribute_buffer.capacity))
	{
		attribute_index = scene->collision_attribute_core.collision_attribute_buffer.size;
		scene->collision_attribute_core.collision_attribute_buffer[attribute_index].grid_cell_index_buffer_offset = scene->collision_attribute_core.collision_grid_cell_row_index_buffer.count_per * attribute_index;
		scene->collision_attribute_core.collision_attribute_buffer[attribute_index].has_already_collided_with_bitmask_buffer_offset = scene->collision_attribute_core.has_already_collided_with_bit_buffer.count_per * attribute_index;
		scene->collision_attribute_core.collision_attribute_buffer[attribute_index].has_already_failed_to_collide_bit_buffer_offset = scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.count_per * attribute_index;
		scene->collision_attribute_core.collision_attribute_buffer[attribute_index].collision_layer_index_buffer_offset = 64 * attribute_index;
		scene->collision_attribute_core.collision_attribute_buffer[attribute_index].flags = flags;
		scene->collision_attribute_core.collision_attribute_buffer[attribute_index].max_trace_step_count = sunder_clamp_u32(1, LIGHTRAY_MAX_TRACE_STEP_COUNT, max_trace_step_count);
		scene->collision_attribute_core.collision_attribute_buffer.size++;

		scene->collision_attribute_core.collision_attribute_buffer[attribute_index].entity_index = entity_index;
		scene->entity_buffer[entity_index].collision_attribute_index = attribute_index;

		const u32 trace_data_index = scene->collision_attribute_core.trace_data_buffer.size;
		const b32 is_static = SUNDER_IS_ANY_BIT_SET(flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_STATIC_BIT, 1u);

		if (!is_static && sunder_valid_index(trace_data_index, scene->collision_attribute_core.trace_data_buffer.capacity))
		{
			scene->collision_attribute_core.trace_data_buffer[trace_data_index].collision_attribute_index = attribute_index;
			scene->collision_attribute_core.collision_attribute_buffer[attribute_index].trace_data_buffer_index = trace_data_index;
			scene->collision_attribute_core.trace_data_buffer.size++;
		}
	}

	return attribute_index;
}

void lightray_bind_aabb(lightray_scene_t* scene, u32 entity_index, u32 aabb_index)
{
	const u32 entity_collision_attribute_index = scene->entity_buffer[entity_index].collision_attribute_index;

	if (!sunder_valid_index(entity_collision_attribute_index, scene->collision_attribute_core.collision_attribute_buffer.size))
	{
		return;
	}

	const lightray_entity_kind collidable_entity_kind = scene->entity_buffer[entity_index].kind;
	const lightray_entity_kind aabb_entity_kind = scene->entity_buffer[aabb_index].kind;

	if (collidable_entity_kind == LIGHTRAY_ENTITY_KIND_AABB || collidable_entity_kind == LIGHTRAY_ENTITY_KIND_COLLISION_MESH)
	{
		return;
	}

	if (aabb_entity_kind != LIGHTRAY_ENTITY_KIND_AABB)
	{
		return;
	}

	scene->collision_attribute_core.collision_attribute_buffer[entity_collision_attribute_index].aabb_index = aabb_index;
	lightray_bind_entity(scene, aabb_index, entity_index);
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

	if (sunder_valid_index(scene->collision_attribute_core.collision_mesh_batch_buffer.size, scene->collision_attribute_core.collision_mesh_batch_buffer.capacity))
	{
		batch_index = scene->collision_attribute_core.collision_mesh_batch_buffer.size;
		scene->collision_attribute_core.collision_mesh_batch_buffer[batch_index].collision_mesh_buffer_offset = batch_index * scene->collision_attribute_core.collision_mesh_batch_buffer.count_per;
		scene->collision_attribute_core.collision_mesh_batch_buffer.size++;
	}

	return batch_index;
}

u32 lightray_push_collision_mesh(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_mesh_index, u32 vertex_count, u32 index_buffer_offset, const lightray_model_t* model)
{
	const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count;

	u32 return_collision_mesh_index = UINT32_MAX;

	if (sunder_valid_index(collision_mesh_count, scene->collision_attribute_core.collision_mesh_batch_buffer.count_per))
	{
		const u32 collision_mesh_buffer_index = scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset + collision_mesh_count;
		scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].vertex_count = vertex_count;
		scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].index_buffer_offset = index_buffer_offset;
		scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].model = model;
		scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].entity_index = collision_mesh_index;
		scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count++;
		scene->collision_attribute_core.collision_mesh_buffer.size++;
		return_collision_mesh_index = collision_mesh_count;
	}

	return return_collision_mesh_index;
}

void lightray_bind_collision_mesh_batch(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_attribute_index)
{
	scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_mesh_batch_index = collision_mesh_batch_index;
	const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset;
	const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count;
	const u32 collision_attribute_owner_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].entity_index;

	for (u32 i = 0; i < collision_mesh_count; i++)
	{
		const u32 current_collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + i].entity_index;
		lightray_bind_entity(scene, current_collision_mesh_entity_index, collision_attribute_owner_entity_index);
	}
}

void lightray_allocate_collision_mesh_vertex_buffers(lightray_scene_t* scene, u32 capsule_mesh_batch_index, u32 capsule_mesh_index, b32 capsule_is_collision_mesh)
{
	// overlap meshes
	for (u32 ovmb = 0; ovmb < scene->overlap_attribute_core.overlap_mesh_batch_buffer.size; ovmb++)
	{
		const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_count;
		const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_buffer_offset;

		for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
		{
			const u32 overlap_mesh_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].index_buffer_offset;
			b32 is_duplicate = SUNDER_FALSE;

			for (u32 fi = 0; fi < scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.size; fi++)
			{
				const u32 current_filtred_ovm_index = scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer[fi];
				const u32 current_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[current_filtred_ovm_index].index_buffer_offset;

				if (current_index_buffer_offset == overlap_mesh_index_buffer_offset)
				{
					is_duplicate = SUNDER_TRUE;

					break;
				}
			} // fi

			if (!is_duplicate)
			{
				scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer[scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.size] = overlap_mesh_buffer_offset + ovm;
				scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.size++;
			}

		} // ovm

	} // ovmb

	// collision meshes
	for (u32 cmb = 0; cmb < scene->collision_attribute_core.collision_mesh_batch_buffer.size; cmb++)
	{
		const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < collision_mesh_count; cm++)
		{
			const u32 collision_mesh_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].index_buffer_offset;
			b32 is_duplicate = SUNDER_FALSE;

			for (u32 fi = 0; fi < scene->collision_attribute_core.filtered_collision_mesh_index_buffer.size; fi++)
			{
				const u32 current_filtered_cm_index = scene->collision_attribute_core.filtered_collision_mesh_index_buffer[fi];
				const u32 current_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[current_filtered_cm_index].index_buffer_offset;

				if (current_index_buffer_offset == collision_mesh_index_buffer_offset)
				{
					is_duplicate = SUNDER_TRUE;

					break;
				}

			} // fi

			if (!is_duplicate)
			{
				scene->collision_attribute_core.filtered_collision_mesh_index_buffer[scene->collision_attribute_core.filtered_collision_mesh_index_buffer.size] = cm + collision_mesh_buffer_offset;
				scene->collision_attribute_core.filtered_collision_mesh_index_buffer.size++;
			}

		} // cm

	} // cmb

	// overlap meshes filter pass
	for (u32 i = 0; i < scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer.size; i++)
	{
		scene->collision_vertex_buffers_population_filter_data_buffer[scene->collision_vertex_buffers_population_filter_data_count].is_collision_mesh = SUNDER_FALSE;
		scene->collision_vertex_buffers_population_filter_data_buffer[scene->collision_vertex_buffers_population_filter_data_count].global_mesh_index = scene->overlap_attribute_core.filtered_overlap_mesh_index_buffer[i];
		scene->collision_vertex_buffers_population_filter_data_count++;
	}

	// collision meshes filter pass
	for (u32 i = 0; i < scene->collision_attribute_core.filtered_collision_mesh_index_buffer.size; i++)
	{
		const u32 current_filtered_cm_index = scene->collision_attribute_core.filtered_collision_mesh_index_buffer[i];
		const u32 current_mesh_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[current_filtered_cm_index].index_buffer_offset;
		b32 is_duplicate = SUNDER_FALSE;

		for (u32 j = 0; j < scene->collision_vertex_buffers_population_filter_data_count; j++)
		{
			const u32 current_filtered_mesh_index = scene->collision_vertex_buffers_population_filter_data_buffer[j].global_mesh_index;
			const b32 is_collision_mesh = scene->collision_vertex_buffers_population_filter_data_buffer[j].is_collision_mesh;
			u32 mesh_index_buffer_offset = 0;

			if (is_collision_mesh)
			{
				mesh_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[current_filtered_mesh_index].index_buffer_offset;
			}

			else
			{
				mesh_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[current_filtered_mesh_index].index_buffer_offset;
			}

			if (mesh_index_buffer_offset == current_mesh_index_buffer_offset)
			{
				is_duplicate = SUNDER_TRUE;

				break;
			}
		}

		if (!is_duplicate)
		{
			scene->collision_vertex_buffers_population_filter_data_buffer[scene->collision_vertex_buffers_population_filter_data_count].global_mesh_index = current_filtered_cm_index;
			scene->collision_vertex_buffers_population_filter_data_buffer[scene->collision_vertex_buffers_population_filter_data_count].is_collision_mesh = SUNDER_TRUE;
			scene->collision_vertex_buffers_population_filter_data_count++;
		}
	}

	for (u32 i = 0; i < scene->collision_vertex_buffers_population_filter_data_count; i++)
	{
		SUNDER_LOG("\n\nglobal_mesh_index: ");
		SUNDER_LOG(scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index);
		SUNDER_LOG("\nis_collision_mesh: ");
		SUNDER_LOG(scene->collision_vertex_buffers_population_filter_data_buffer[i].is_collision_mesh);
	}

	// 1008 both

	// all i see is void, mother, oh mother, why, why are the stars so dim this night?

	const u32 total_collision_mesh_batch_count = scene->collision_attribute_core.collision_mesh_batch_buffer.size;
	const u32 total_collision_mesh_count = scene->collision_attribute_core.collision_mesh_buffer.capacity;
	u64 initial_sentinel_index_buffer_suballocation_size_aligned = 0;
	u64 initial_sentinel_index_buffer_suballocation_size = 0;

	u64 cull_bitmask = 0;

	for (u32 i = 0; i < scene->collision_vertex_buffers_population_filter_data_count; i++)
	{
		const b32 is_collision_mesh = scene->collision_vertex_buffers_population_filter_data_buffer[i].is_collision_mesh;
		const u32 global_mesh_index = scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index;
		u32 index_count = 0;

		if (is_collision_mesh)
		{
			index_count = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].vertex_count;
		}

		else
		{
			index_count = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_count;
		}

		initial_sentinel_index_buffer_suballocation_size_aligned += sunder_compute_aligned_allocation_size(sizeof(u32), index_count, alignof(u32));
		initial_sentinel_index_buffer_suballocation_size += sunder_compute_aligned_allocation_size(sizeof(u32), index_count, alignof(u32));
		initial_sentinel_index_buffer_suballocation_size_aligned = sunder_align64(initial_sentinel_index_buffer_suballocation_size_aligned, alignof(u64));
	}

	/*

	// computing intermediate_sentinel_index_buffer allocation size
	for (u32 cmb = 0; cmb < total_collision_mesh_batch_count; cmb++)
	{
		const u32 current_collision_mesh_count = scene->collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 current_collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < current_collision_mesh_count; cm++)
		{
			const u32 current_index_count = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].vertex_count;
			const u32 current_index_buffer_offset = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].index_buffer_offset;
			b32 should_cull = SUNDER_FALSE;

			for (u32 d = 0; d < total_collision_mesh_count; d++)
			{
				const u32 other_index_buffer_offset = scene->collision_mesh_buffer[d].index_buffer_offset;

				if (other_index_buffer_offset == current_index_buffer_offset)
				{
					if (SUNDER_IS_ANY_BIT_SET(cull_bitmask, d, 1ull))
					{
						should_cull = SUNDER_TRUE;
						break;
					}
				}

			} // d

			if (should_cull)
			{
				continue;
			}

			initial_sentinel_index_buffer_suballocation_size_aligned += sunder_compute_aligned_allocation_size(sizeof(u32), current_index_count, alignof(u32));
			initial_sentinel_index_buffer_suballocation_size += sunder_compute_array_size_in_bytes(sizeof(u32), current_index_count);
			initial_sentinel_index_buffer_suballocation_size_aligned = sunder_align64(initial_sentinel_index_buffer_suballocation_size_aligned, alignof(u64));
			SUNDER_SET_BIT(cull_bitmask, current_collision_mesh_buffer_offset + cm, 1ull);
		} // cm

	} // cmb
	*/

	sunder_arena_t intermediate_arena{};
	const sunder_arena_result intermediate_arena_allocation_result = sunder_allocate_arena(&intermediate_arena, initial_sentinel_index_buffer_suballocation_size_aligned, alignof(u64));

	const sunder_arena_suballocation_result_t initial_sentinel_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&intermediate_arena, initial_sentinel_index_buffer_suballocation_size, alignof(u32));
	scene->sentinel_index_buffer = SUNDER_CAST(u32*, initial_sentinel_index_buffer_suballocation_result.data);
	u32 sentinel_index_buffer_offset = 0;
	cull_bitmask = 0;

	for (u32 i = 0; i < scene->collision_vertex_buffers_population_filter_data_count; i++)
	{
		const b32 is_collision_mesh = scene->collision_vertex_buffers_population_filter_data_buffer[i].is_collision_mesh;
		const u32 global_mesh_index = scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index;
		u32 index_count = 0;
		u32 index_buffer_offset = 0;

		if (is_collision_mesh)
		{
			index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].index_buffer_offset;
			index_count = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].vertex_count;
		}

		else
		{
			index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_buffer_offset;
			index_count = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_count;
		}

		for (u32 v = 0; v < index_count; v++)
		{
			u32 current_sentinel_vertex_count = 0;

			if (is_collision_mesh)
			{
				current_sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_vertex_count;
			}

			else
			{
				current_sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_vertex_count;
			}

			const u32 current_vertex_index = scene->index_buffer[index_buffer_offset + v];
			b32 should_be_culled = SUNDER_FALSE;

			for (u32 sv = 0; sv < current_sentinel_vertex_count; sv++)
			{
				const u32 current_sentinel_vertex_index = scene->sentinel_index_buffer[sentinel_index_buffer_offset + sv];
				const sunder_v3_t vertex_position = lightray_glm_vec3_to_sunder(scene->vertex_buffer[current_vertex_index].position);
				const sunder_v3_t sentinel_vertex_position = lightray_glm_vec3_to_sunder(scene->vertex_buffer[current_sentinel_vertex_index].position);

				if (vertex_position == sentinel_vertex_position)
				{
					should_be_culled = SUNDER_TRUE;
					break;
				}
			}

			if (!should_be_culled)
			{
				scene->sentinel_index_buffer[sentinel_index_buffer_offset + current_sentinel_vertex_count] = current_vertex_index;

				if (is_collision_mesh)
				{
					scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_vertex_count++;
				}

				else
				{
					scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_vertex_count++;
				}
			}
		}

		if (is_collision_mesh)
		{
			scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset = sentinel_index_buffer_offset;
			sentinel_index_buffer_offset += scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_vertex_count;
		}

		else
		{
			scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset = sentinel_index_buffer_offset;
			sentinel_index_buffer_offset += scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_vertex_count;
		}
	}

	for (u32 i = 0; i < scene->collision_vertex_buffers_population_filter_data_count; i++)
	{
		const u32 global_mesh_index = scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index;
		const b32 is_collision_mesh = scene->collision_vertex_buffers_population_filter_data_buffer[i].is_collision_mesh;

		SUNDER_LOG("\n\nglobal_mesh_index: ");
		SUNDER_LOG(global_mesh_index);
		SUNDER_LOG("\nis_collision_mesh: ");
		SUNDER_LOG(is_collision_mesh);

		if (is_collision_mesh)
		{
			SUNDER_LOG("\ncollision_mesh: ");
			SUNDER_LOG("\nsentinel_vertex_count: ");
			SUNDER_LOG(scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_vertex_count);
			SUNDER_LOG("\nsentinel_local_space_vertex_position_buffer_offset: ");
			SUNDER_LOG(scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset);
		}

		else
		{
			SUNDER_LOG("\noverlap_mesh: ");
			SUNDER_LOG("\nsentinel_vertex_count: ");
			SUNDER_LOG(scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_vertex_count);
			SUNDER_LOG("\nsentinel_local_space_vertex_position_buffer_offset: ");
			SUNDER_LOG(scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset);

		}
	}

	for (u32 i = 0; i < scene->collision_vertex_buffers_population_filter_data_count; i++)
	{
		const u32 global_mesh_index = scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index;
		const b32 is_collision_mesh = scene->collision_vertex_buffers_population_filter_data_buffer[i].is_collision_mesh;

		u32 index_buffer_offset = 0;
		u32 sentinel_vertex_count = 0;
		u32 sentinel_local_space_vertex_position_buffer_offset = 0;

		if (is_collision_mesh)
		{
			sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_vertex_count;
			sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
			index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].index_buffer_offset;
		}

		else
		{
			sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_vertex_count;
			sentinel_local_space_vertex_position_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
			index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_buffer_offset;
		}

		for (u32 cmb = 0; cmb < scene->collision_attribute_core.collision_mesh_batch_buffer.size; cmb++)
		{
			const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_count;
			const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

			for (u32 cm = 0; cm < collision_mesh_count; cm++)
			{
				const u32 cm_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].index_buffer_offset;
				const u32 cm_global_mesh_index = collision_mesh_buffer_offset + cm;

				if (cm_index_buffer_offset == index_buffer_offset)
				{
					if (global_mesh_index != cm_global_mesh_index || global_mesh_index == cm_global_mesh_index && !is_collision_mesh)
					{
						scene->collision_attribute_core.collision_mesh_buffer[cm_global_mesh_index].sentinel_vertex_count = sentinel_vertex_count;
						scene->collision_attribute_core.collision_mesh_buffer[cm_global_mesh_index].sentinel_local_space_vertex_position_buffer_offset = sentinel_local_space_vertex_position_buffer_offset;
					}
				}
			}
		}

		for (u32 ovmb = 0; ovmb < scene->overlap_attribute_core.overlap_mesh_batch_buffer.size; ovmb++)
		{
			const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_count;
			const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_buffer_offset;

			for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
			{
				const u32 ovm_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].index_buffer_offset;
				const u32 ovm_global_mesh_index = overlap_mesh_buffer_offset + ovm;

				if (index_buffer_offset == ovm_index_buffer_offset)
				{
					if (global_mesh_index != ovm_global_mesh_index || global_mesh_index == ovm_global_mesh_index && is_collision_mesh)
					{
						scene->overlap_attribute_core.overlap_mesh_buffer[ovm_global_mesh_index].sentinel_vertex_count = sentinel_vertex_count;;
						scene->overlap_attribute_core.overlap_mesh_buffer[ovm_global_mesh_index].sentinel_local_space_vertex_position_buffer_offset = sentinel_local_space_vertex_position_buffer_offset;
					}
				}
			}
		}
	}

	for (u32 ovmb = 0; ovmb < scene->overlap_attribute_core.overlap_mesh_batch_buffer.size; ovmb++)
	{
		const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_count;
		const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_buffer_offset;

		for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
		{
			SUNDER_LOG("\n\nsentinel_vertex_count: ");
			SUNDER_LOG(scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_vertex_count);
			SUNDER_LOG("\nsentinel_local_space_vertex_position_buffer_offset: ");
			SUNDER_LOG(scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_local_space_vertex_position_buffer_offset);
		}
	}

	for (u32 cmb = 0; cmb < scene->collision_attribute_core.collision_mesh_batch_buffer.size; cmb++)
	{
		const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < collision_mesh_count; cm++)
		{
			SUNDER_LOG("\n\nsentinel_vertex_count: ");
			SUNDER_LOG(scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_vertex_count);
			SUNDER_LOG("\nsentinel_local_space_vertex_position_buffer_offset: ");
			SUNDER_LOG(scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset);
		}
	}

	u32 bbb = 0;



	/*
	// getting sentinel vertex indices
	for (u32 cmb = 0; cmb < total_collision_mesh_batch_count; cmb++)
	{
		const u32 current_collision_mesh_count = scene->collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 current_collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < current_collision_mesh_count; cm++)
		{
			const u32 current_index_count = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].vertex_count;
			const u32 current_index_buffer_offset = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].index_buffer_offset;
			b32 is_duplicate = SUNDER_FALSE;

			for (u32 d = 0; d < total_collision_mesh_count; d++)
			{
				const u32 other_index_buffer_offset = scene->collision_mesh_buffer[d].index_buffer_offset;

				if (current_index_buffer_offset == other_index_buffer_offset)
				{
					if (SUNDER_IS_ANY_BIT_SET(cull_bitmask, d, 1ull))
					{
						scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset = scene->collision_mesh_buffer[d].sentinel_local_space_vertex_position_buffer_offset;
						scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_vertex_count = scene->collision_mesh_buffer[d].sentinel_vertex_count;

						is_duplicate = SUNDER_TRUE;
						break;
					}
				}
			}

			if (is_duplicate)
			{
				continue;
			}

			for (u32 v = 0; v < current_index_count; v++)
			{
				u32 current_sentinel_index_count = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_vertex_count;
				const u32 current_vertex_index = scene->index_buffer[current_index_buffer_offset + v];
				b32 should_be_culled = SUNDER_FALSE;

				for (u32 i = 0; i < current_sentinel_index_count; i++)
				{
					const u32 current_sentinel_vertex_index = scene->sentinel_index_buffer[sentinel_index_buffer_offset + i];
					const sunder_v3_t vertex = lightray_glm_vec3_to_sunder(scene->vertex_buffer[current_vertex_index].position);
					const sunder_v3_t sentinel_vertex = lightray_glm_vec3_to_sunder(scene->vertex_buffer[current_sentinel_vertex_index].position);

					if (sentinel_vertex == vertex)
					{
						should_be_culled = SUNDER_TRUE;
						break;
					}

				} // i

				if (!should_be_culled)
				{
					scene->sentinel_index_buffer[sentinel_index_buffer_offset + current_sentinel_index_count] = current_vertex_index;
					scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_vertex_count++;
				}

			} // v

			scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset = sentinel_index_buffer_offset;
			sentinel_index_buffer_offset += scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_vertex_count;
			SUNDER_SET_BIT(cull_bitmask, current_collision_mesh_buffer_offset + cm, 1ull);

		} // cm

	} // cmb
	*/

	// computing an allocation size for all world space vertex position buffers
	u64 accumulated_sentinel_world_space_vertex_position_buffer_allocation_size = 0;

	for (u32 ovmb = 0; ovmb < scene->overlap_attribute_core.overlap_mesh_batch_buffer.size; ovmb++)
	{
		const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_count;
		const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_buffer_offset;

		for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
		{
			const u32 sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_vertex_count;
			accumulated_sentinel_world_space_vertex_position_buffer_allocation_size += sunder_compute_aligned_allocation_size(sizeof(sunder_v3_t), sentinel_vertex_count, alignof(u64));
			accumulated_sentinel_world_space_vertex_position_buffer_allocation_size = sunder_align64(accumulated_sentinel_world_space_vertex_position_buffer_allocation_size, alignof(u64));
		}
	}

	for (u32 cmb = 0; cmb < scene->collision_attribute_core.collision_mesh_batch_buffer.size; cmb++)
	{
		const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < collision_mesh_count; cm++)
		{
			const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].entity_index;
			const u32 collision_attribute_entity_index = scene->entity_buffer[collision_mesh_entity_index].parent_index;
			const u32 collision_attribute_entity_attribute_index = scene->entity_buffer[collision_attribute_entity_index].collision_attribute_index;
			const u32 max_trace_step_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_entity_attribute_index].max_trace_step_count;
			const u32 collision_attribute_flags = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_entity_attribute_index].flags;
			const b32 is_static = SUNDER_IS_ANY_BIT_SET(collision_attribute_flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_STATIC_BIT, 1u);

			scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].max_trace_step_count = max_trace_step_count;

			const u32 sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_vertex_count;
			accumulated_sentinel_world_space_vertex_position_buffer_allocation_size += sunder_compute_aligned_allocation_size(sizeof(sunder_v3_t), sentinel_vertex_count, alignof(u64));
			accumulated_sentinel_world_space_vertex_position_buffer_allocation_size = sunder_align64(accumulated_sentinel_world_space_vertex_position_buffer_allocation_size, alignof(u64));

			if (!is_static)
			{
				for (u32 sc = 0; sc < max_trace_step_count; sc++)
				{
					accumulated_sentinel_world_space_vertex_position_buffer_allocation_size += sunder_compute_aligned_allocation_size(sizeof(sunder_v3_t), sentinel_vertex_count, alignof(u64));
					accumulated_sentinel_world_space_vertex_position_buffer_allocation_size = sunder_align64(accumulated_sentinel_world_space_vertex_position_buffer_allocation_size, alignof(u64));
				}
			}
		}
	}

	/*
	for (u32 cmb = 0; cmb < total_collision_mesh_batch_count; cmb++)
	{
		const u32 current_collision_mesh_count = scene->collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 current_collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < current_collision_mesh_count; cm++)
		{
			const u32 current_sentinel_vertex_count = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_vertex_count;
			accumulated_sentinel_world_space_vertex_position_buffer_allocation_size += sunder_compute_aligned_allocation_size(sizeof(sunder_v3_t), current_sentinel_vertex_count, alignof(u64));
			accumulated_sentinel_world_space_vertex_position_buffer_allocation_size = sunder_align64(accumulated_sentinel_world_space_vertex_position_buffer_allocation_size, alignof(u64));
		} // cm
		
	} // cmv
	*/

	u32 chosen_mesh_capsule_index = 0;
	u32 chosen_mesh_capsule_sentinel_vertex_count = 0;
	u32 chosen_mesh_capsule_index_count = 0;

	if (capsule_is_collision_mesh)
	{
		chosen_mesh_capsule_index = scene->collision_attribute_core.collision_mesh_batch_buffer[capsule_mesh_batch_index].collision_mesh_buffer_offset + capsule_mesh_index;
		chosen_mesh_capsule_sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[chosen_mesh_capsule_index].sentinel_vertex_count;
		chosen_mesh_capsule_index_count = scene->collision_attribute_core.collision_mesh_buffer[chosen_mesh_capsule_index].vertex_count;
	}

	else
	{
		chosen_mesh_capsule_index = scene->overlap_attribute_core.overlap_mesh_batch_buffer[capsule_mesh_batch_index].overlap_mesh_buffer_offset + capsule_mesh_index;
		chosen_mesh_capsule_sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[chosen_mesh_capsule_index].sentinel_vertex_count;
		chosen_mesh_capsule_index_count = scene->overlap_attribute_core.overlap_mesh_buffer[chosen_mesh_capsule_index].index_count;
	}

	u64 capsule_beyond_lower_clipping_plane_data_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), chosen_mesh_capsule_sentinel_vertex_count, alignof(u64));
	capsule_beyond_lower_clipping_plane_data_allocation_size = sunder_align64(capsule_beyond_lower_clipping_plane_data_allocation_size, alignof(u64));
	capsule_beyond_lower_clipping_plane_data_allocation_size += sunder_compute_aligned_allocation_size(sizeof(u32), chosen_mesh_capsule_index_count, alignof(u64));
	capsule_beyond_lower_clipping_plane_data_allocation_size = sunder_align64(capsule_beyond_lower_clipping_plane_data_allocation_size, alignof(u64));

	const u64 sentinel_local_space_vertex_position_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(sunder_v3_t), sentinel_index_buffer_offset, alignof(sunder_v3_t));
	const u64 sentinel_local_space_vertex_position_buffer_suballocation_size = sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), sentinel_index_buffer_offset);

	const u64 collision_data_arena_allocation_size = initial_sentinel_index_buffer_suballocation_size + sentinel_local_space_vertex_position_buffer_allocation_size + accumulated_sentinel_world_space_vertex_position_buffer_allocation_size + capsule_beyond_lower_clipping_plane_data_allocation_size;
	const sunder_arena_result collision_data_arena_allocation_result = sunder_allocate_arena(&scene->collision_mesh_vertex_buffers_arena, collision_data_arena_allocation_size, alignof(u64));

	const sunder_arena_suballocation_result_t sentinel_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, initial_sentinel_index_buffer_suballocation_size, alignof(u32));
	u32* temp_sentinel_index_buffer_ptr = SUNDER_CAST(u32*, sentinel_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t sentinel_local_space_vertex_position_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, sentinel_local_space_vertex_position_buffer_suballocation_size, alignof(sunder_v3_t));
	scene->sentinel_local_space_vertex_position_buffer = SUNDER_CAST(sunder_v3_t*, sentinel_local_space_vertex_position_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t capsule_beyond_lower_clipping_plane_sentinel_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, sunder_compute_array_size_in_bytes(sizeof(u32), chosen_mesh_capsule_sentinel_vertex_count), alignof(u32));
	scene->capsule_beyond_lower_clipping_plane_sentinel_index_buffer = SUNDER_CAST(u32*, capsule_beyond_lower_clipping_plane_sentinel_index_buffer_suballocation_result.data);

	const sunder_arena_suballocation_result_t capsule_beyond_lower_clipping_plane_triangle_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, sunder_compute_array_size_in_bytes(sizeof(u32), chosen_mesh_capsule_index_count), alignof(u32));
	scene->capsule_beyond_lower_clipping_plane_triangle_index_buffer = SUNDER_CAST(u32*, capsule_beyond_lower_clipping_plane_triangle_index_buffer_suballocation_result.data);

	sunder_buffer_copy_data_t buffer_copy_data{};
	buffer_copy_data.src_offset = 0;
	buffer_copy_data.dst_offset = 0;
	buffer_copy_data.src_size = initial_sentinel_index_buffer_suballocation_size;
	buffer_copy_data.dst_size = initial_sentinel_index_buffer_suballocation_size;
	buffer_copy_data.bytes_to_write = initial_sentinel_index_buffer_suballocation_size;

	const u64 bytes_written = sunder_copy_buffer(temp_sentinel_index_buffer_ptr, scene->sentinel_index_buffer, &buffer_copy_data);
	scene->sentinel_index_buffer = temp_sentinel_index_buffer_ptr;
	sunder_free_arena(&intermediate_arena);

	// copying intermediate_sentinel_index_buffer to sentinel_local_space_vertex_position_buffer
	for (u32 v = 0; v < sentinel_index_buffer_offset; v++)
	{
		const u32 vertex_index = scene->sentinel_index_buffer[v];
		scene->sentinel_local_space_vertex_position_buffer[v] = lightray_glm_vec3_to_sunder(scene->vertex_buffer[vertex_index].position);
	}

	u32 sentinel_index_buffer_iter = 0;
	cull_bitmask = 0;

	for (u32 i = 0; i < scene->collision_vertex_buffers_population_filter_data_count; i++)
	{
		const u32 global_mesh_index = scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index;
		const b32 is_collision_mesh = scene->collision_vertex_buffers_population_filter_data_buffer[i].is_collision_mesh;
		u32 index_count = 0;
		u32 index_buffer_offset = 0;
		u32 sentinel_vertex_count = 0;
		u32 sentinel_local_space_vertex_position_buffer_offset = 0;
		u32 sentinel_index_count = 0;

		if (is_collision_mesh)
		{
			sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_vertex_count;
			index_count = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].vertex_count;
			index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].index_buffer_offset;
			sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
		}

		else
		{
			sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_vertex_count;
			index_count = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_count;
			index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_buffer_offset;
			sentinel_local_space_vertex_position_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
		}

		for (u32 v = 0; v < index_count; v++)
		{
			const u32 vertex_index = scene->index_buffer[index_buffer_offset + v];
			const sunder_v3_t current_vertex = lightray_glm_vec3_to_sunder(scene->vertex_buffer[vertex_index].position);

			for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
			{
				const sunder_v3_t current_sentinel_vertex = scene->sentinel_local_space_vertex_position_buffer[sentinel_local_space_vertex_position_buffer_offset + sv];

				if (current_sentinel_vertex == current_vertex)
				{
					scene->sentinel_index_buffer[sentinel_index_buffer_iter + sentinel_index_count] = sv;
					sentinel_index_count++;

					break;
				}
			}
		}

		if (is_collision_mesh)
		{
			scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_index_buffer_offset = sentinel_index_buffer_iter;
		}
		
		else
		{
			scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_index_buffer_offset = sentinel_index_buffer_iter;
		}

		sentinel_index_buffer_iter += sentinel_index_count;
	}

	// copy pass

	for (u32 i = 0; i < scene->collision_vertex_buffers_population_filter_data_count; i++)
	{
		const u32 global_mesh_index = scene->collision_vertex_buffers_population_filter_data_buffer[i].global_mesh_index;
		const b32 is_collision_mesh = scene->collision_vertex_buffers_population_filter_data_buffer[i].is_collision_mesh;
		u32 index_count = 0;
		u32 sentinel_index_buffer_offset = 0;
		u32 index_buffer_offset = 0;

		if (is_collision_mesh)
		{
			index_count = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].vertex_count;
			sentinel_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].sentinel_index_buffer_offset;
			index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[global_mesh_index].index_buffer_offset;
		}
		
		else
		{
			index_count = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_count;
			sentinel_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].sentinel_index_buffer_offset;
			index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[global_mesh_index].index_buffer_offset;
		}

		for (u32 ovmb = 0; ovmb < scene->overlap_attribute_core.overlap_mesh_batch_buffer.size; ovmb++)
		{
			const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_count;
			const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_buffer_offset;

			for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
			{
				const u32 ovm_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].index_buffer_offset;
				const u32 ovm_global_mesh_index = overlap_mesh_buffer_offset + ovm;

				if (ovm_index_buffer_offset == index_buffer_offset)
				{
					if (global_mesh_index != ovm_global_mesh_index || global_mesh_index == ovm_global_mesh_index && is_collision_mesh)
					{
						scene->overlap_attribute_core.overlap_mesh_buffer[ovm_global_mesh_index].sentinel_index_buffer_offset = sentinel_index_buffer_offset;
					}
				}
			}
		}

		for (u32 cmb = 0; cmb < scene->collision_attribute_core.collision_mesh_batch_buffer.size; cmb++)
		{
			const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_count;
			const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

			for (u32 cm = 0; cm < collision_mesh_count; cm++)
			{
				const u32 cm_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].index_buffer_offset;
				const u32 cm_global_mesh_index = collision_mesh_buffer_offset + cm;

				if (cm_index_buffer_offset == index_buffer_offset)
				{
					if (cm_global_mesh_index != global_mesh_index || cm_global_mesh_index == global_mesh_index && !is_collision_mesh)
					{
						scene->collision_attribute_core.collision_mesh_buffer[cm_global_mesh_index].sentinel_index_buffer_offset = sentinel_index_buffer_offset;
					}
				}
			}
		}
	}

	for (u32 ovmb = 0; ovmb < scene->overlap_attribute_core.overlap_mesh_batch_buffer.size; ovmb++)
	{
		const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_count;
		const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_buffer_offset;

		for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
		{
			SUNDER_LOG("\nsentinel_index_buffer_offset: ");
			const u32 sentinel_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_index_buffer_offset;
			const u32 index_count = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].index_count;
			SUNDER_LOG(sentinel_index_buffer_offset);
			SUNDER_LOG("\n");

			for (u32 si = 0; si < index_count; si++)
			{
				SUNDER_LOG(scene->sentinel_index_buffer[sentinel_index_buffer_offset + si]);
				SUNDER_LOG(", ");
			}
		}
	}

	SUNDER_LOG("\n\n");

	for (u32 cmb = 0; cmb < scene->collision_attribute_core.collision_mesh_batch_buffer.size; cmb++)
	{
		const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;
		
		for (u32 cm = 0; cm < collision_mesh_count; cm++)
		{
			SUNDER_LOG("\nsentinel_index_buffer_offset: ");
			const u32 sentinel_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_index_buffer_offset;
			const u32 index_count = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].vertex_count;
			SUNDER_LOG(sentinel_index_buffer_offset);
			SUNDER_LOG("\n");

			for (u32 si = 0; si < index_count; si++)
			{
				SUNDER_LOG(scene->sentinel_index_buffer[sentinel_index_buffer_offset + si]);
				SUNDER_LOG(", ");
			}
		}
	}

	/*
	// populating the triangle index buffer out of sentinel vertices
	for (u32 cmb = 0; cmb < total_collision_mesh_batch_count; cmb++)
	{
		const u32 current_collision_mesh_count = scene->collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 current_collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < current_collision_mesh_count; cm++)
		{
			const u32 current_index_count = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].vertex_count;
			const u32 current_index_buffer_offset = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].index_buffer_offset;
			const u32 current_sentinel_vertex_count = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_vertex_count;
			const u32 current_sentinel_local_space_vertex_position_buffer_offset = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset;
			b32 is_duplicate = SUNDER_FALSE;

			for (u32 d = 0; d < total_collision_mesh_count; d++)
			{
				const u32 other_index_buffer_offset = scene->collision_mesh_buffer[d].index_buffer_offset;

				if (current_index_buffer_offset == other_index_buffer_offset)
				{
					if (SUNDER_IS_ANY_BIT_SET(cull_bitmask, d, 1ull))
					{
						scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_index_buffer_offset = scene->collision_mesh_buffer[d].sentinel_index_buffer_offset;
						is_duplicate = SUNDER_TRUE;

						break;
					}
				}

			} // d

			if (is_duplicate)
			{
				continue;
			}

			u32 sentinel_index_count = 0;

			for (u32 v = 0; v < current_index_count; v++)
			{
				const u32 current_vertex_index = scene->index_buffer[current_index_buffer_offset + v];
				const sunder_v3_t current_vertex = lightray_glm_vec3_to_sunder(scene->vertex_buffer[current_vertex_index].position);

				for (u32 sv = 0; sv < current_sentinel_vertex_count; sv++)
				{
					const sunder_v3_t current_sentinel_vertex = scene->sentinel_local_space_vertex_position_buffer[current_sentinel_local_space_vertex_position_buffer_offset + sv];

					if (current_sentinel_vertex == current_vertex)
					{
						scene->sentinel_index_buffer[sentinel_index_buffer_iter + sentinel_index_count] = sv;
						sentinel_index_count++;
						break;
					}

				} // sv

			} // v

			scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_index_buffer_offset = sentinel_index_buffer_iter;
			sentinel_index_buffer_iter += sentinel_index_count;
			SUNDER_SET_BIT(cull_bitmask, current_collision_mesh_buffer_offset + cm, 1ull);

		} // cm

	} // cmb
	*/


	/*
	for (u32 cmb = 0; cmb < total_collision_mesh_batch_count; cmb++)
	{
		const u32 current_collision_mesh_count = scene->collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 current_collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < current_collision_mesh_count; cm++)
		{
			const u32 current_sentinel_local_space_vertex_position_buffer_offset = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset;
			const u32 current_index_count = scene->collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].vertex_count;
			SUNDER_LOG("\n\n==========================================\n");

			for (u32 i = 0; i < current_index_count; i++)
			{
				SUNDER_LOG(scene->sentinel_index_buffer[i]);
				SUNDER_LOG(", ");
			}

		} // cm

	} // cmb
	*/

	// allocating sentinel_world_space_vertex_position_buffer of each collision mesh

	for (u32 cmb = 0; cmb < scene->collision_attribute_core.collision_mesh_batch_buffer.size; cmb++)
	{
		const u32 current_collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_count;
		const u32 current_collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[cmb].collision_mesh_buffer_offset;

		for (u32 cm = 0; cm < current_collision_mesh_count; cm++)
		{
			const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].entity_index;
			const u32 collision_attribute_entity_index = scene->entity_buffer[collision_mesh_entity_index].parent_index;
			const u32 collision_attribute_entity_attribute_index = scene->entity_buffer[collision_attribute_entity_index].collision_attribute_index;
			const u32 collision_attribute_flags = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_entity_attribute_index].flags;
			const b32 is_static = SUNDER_IS_ANY_BIT_SET(collision_attribute_flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_STATIC_BIT, 1u);

			const u32 current_sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_vertex_count;
			const sunder_arena_suballocation_result_t sentinel_world_space_vertex_position_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), current_sentinel_vertex_count), alignof(sunder_v3_t));
			scene->collision_attribute_core.collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].sentinel_world_space_vertex_position_buffer = SUNDER_CAST(sunder_v3_t*, sentinel_world_space_vertex_position_buffer_suballocation_result.data);
			const u32 max_trace_step_count = scene->collision_attribute_core.collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].max_trace_step_count;

			if (!is_static)
			{
				const u64 suballocation_size = sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), current_sentinel_vertex_count * max_trace_step_count);
				const sunder_arena_suballocation_result_t suballoc_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, suballocation_size, alignof(sunder_v3_t));
				scene->collision_attribute_core.collision_mesh_buffer[current_collision_mesh_buffer_offset + cm].trace_step_sentinel_world_space_vertex_position_buffer = SUNDER_CAST(sunder_v3_t*, suballoc_result.data);
			}
		}
	}

	for (u32 ovmb = 0; ovmb < scene->overlap_attribute_core.overlap_mesh_batch_buffer.size; ovmb++)
	{
		const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_count;
		const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[ovmb].overlap_mesh_buffer_offset;

		for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
		{
			const u32 sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_vertex_count;
			const u64 suballocation_size = sunder_compute_array_size_in_bytes(sizeof(sunder_v3_t), sentinel_vertex_count);
			const sunder_arena_suballocation_result_t sentinel_world_space_vertex_position_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&scene->collision_mesh_vertex_buffers_arena, suballocation_size, alignof(sunder_v3_t));
			scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_world_space_vertex_position_buffer = SUNDER_CAST(sunder_v3_t*, sentinel_world_space_vertex_position_buffer_suballocation_result.data);
		}
	}

	u32 chosen_capsule_sentinel_local_space_vertex_position_buffer_offset = 0;
	u32 chosen_capsule_sentinel_index_buffer_offset = 0;

	if (capsule_is_collision_mesh)
	{
		chosen_capsule_sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[chosen_mesh_capsule_index].sentinel_local_space_vertex_position_buffer_offset;
		chosen_capsule_sentinel_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[chosen_mesh_capsule_index].sentinel_index_buffer_offset;
	}

	else
	{
		chosen_capsule_sentinel_local_space_vertex_position_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[chosen_mesh_capsule_index].sentinel_local_space_vertex_position_buffer_offset;
		chosen_capsule_sentinel_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[chosen_mesh_capsule_index].sentinel_index_buffer_offset;
	}

	const sunder_v3_t lower_clipping_plane_pivot_vertex = scene->sentinel_local_space_vertex_position_buffer[chosen_capsule_sentinel_local_space_vertex_position_buffer_offset + 1];
	const f32 lower_clipping_plane_pivot_vertex_z = lower_clipping_plane_pivot_vertex.z;
	u32 capsule_beyond_lower_clipping_plane_sentinel_index_buffer_iter = 0;
	u32 capsule_beyond_lower_clipping_plane_triangle_index_buffer_iter = 0;

	scene->capsule_sentinel_local_space_vertex_position_buffer_offset = chosen_capsule_sentinel_local_space_vertex_position_buffer_offset;
	scene->capsule_sentinel_index_buffer_offset = chosen_capsule_sentinel_index_buffer_offset;

	for (u32 sv = 0; sv < chosen_mesh_capsule_sentinel_vertex_count; sv++)
	{
		const sunder_v3_t current_sentinel_vertex = scene->sentinel_local_space_vertex_position_buffer[chosen_capsule_sentinel_local_space_vertex_position_buffer_offset + sv];
		const f32 current_sentinel_vertex_z = current_sentinel_vertex.z;

		if (current_sentinel_vertex_z <= lower_clipping_plane_pivot_vertex_z)
		{
			scene->capsule_beyond_lower_clipping_plane_sentinel_index_buffer[capsule_beyond_lower_clipping_plane_sentinel_index_buffer_iter] = sv;
			capsule_beyond_lower_clipping_plane_sentinel_index_buffer_iter++;
		} 
	}

	SUNDER_LOG("\n\n");
	for (u32 i = 0; i < capsule_beyond_lower_clipping_plane_sentinel_index_buffer_iter; i++)
	{
		SUNDER_LOG(scene->capsule_beyond_lower_clipping_plane_sentinel_index_buffer[i]);
		SUNDER_LOG(", ");
	} 

	for (u32 i = 0; i < chosen_mesh_capsule_index_count; i += 3)
	{
		const u32 v0_index = scene->sentinel_index_buffer[chosen_capsule_sentinel_index_buffer_offset + i + 0];
		const u32 v1_index = scene->sentinel_index_buffer[chosen_capsule_sentinel_index_buffer_offset + i + 1];
		const u32 v2_index = scene->sentinel_index_buffer[chosen_capsule_sentinel_index_buffer_offset + i + 2];

		const sunder_v3_t v0 = scene->sentinel_local_space_vertex_position_buffer[chosen_capsule_sentinel_local_space_vertex_position_buffer_offset + v0_index];
		const sunder_v3_t v1 = scene->sentinel_local_space_vertex_position_buffer[chosen_capsule_sentinel_local_space_vertex_position_buffer_offset + v1_index];
		const sunder_v3_t v2 = scene->sentinel_local_space_vertex_position_buffer[chosen_capsule_sentinel_local_space_vertex_position_buffer_offset + v2_index];

		const f32 v0_z = v0.z;
		const f32 v1_z = v1.z;
		const f32 v2_z = v2.z;

		if (v0_z > lower_clipping_plane_pivot_vertex_z || v1_z > lower_clipping_plane_pivot_vertex_z || v2_z > lower_clipping_plane_pivot_vertex_z)
		{
			continue;
		}

		scene->capsule_beyond_lower_clipping_plane_triangle_index_buffer[capsule_beyond_lower_clipping_plane_triangle_index_buffer_iter + 0] = v0_index;
		scene->capsule_beyond_lower_clipping_plane_triangle_index_buffer[capsule_beyond_lower_clipping_plane_triangle_index_buffer_iter + 1] = v1_index;
		scene->capsule_beyond_lower_clipping_plane_triangle_index_buffer[capsule_beyond_lower_clipping_plane_triangle_index_buffer_iter + 2] = v2_index;
		capsule_beyond_lower_clipping_plane_triangle_index_buffer_iter += 3;
	}

	scene->capsule_beyond_lower_clipping_plane_sentinel_index_count = capsule_beyond_lower_clipping_plane_sentinel_index_buffer_iter;
	scene->capsule_beyond_lower_clipping_plane_triangle_index_count = capsule_beyond_lower_clipping_plane_triangle_index_buffer_iter;

	SUNDER_LOG("\n\n");
	for (u32 i = 0; i < capsule_beyond_lower_clipping_plane_triangle_index_buffer_iter; i++)
	{
		SUNDER_LOG("\n");
		SUNDER_LOG(scene->capsule_beyond_lower_clipping_plane_triangle_index_buffer[i]);
	}

	u32 temp_break = 0;
}

void lightray_free_collision_mesh_vertex_buffers(lightray_scene_t* scene)
{
	sunder_free_arena(&scene->collision_mesh_vertex_buffers_arena);
}

void lightray_initialize_collision_mesh_sentinel_world_space_vertex_buffer(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_mesh_index)
{
	const u32 collision_mesh_buffer_index = scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset + collision_mesh_index;
	const u32 local_space_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].sentinel_local_space_vertex_position_buffer_offset;
	const u32 sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].sentinel_vertex_count;
	const sunder_m4_t m = lightray_copy_glm_mat4_to_sunder(scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].model->model);
	sunder_v3_t* world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_index].sentinel_world_space_vertex_position_buffer;

	lightray_compute_sentinel_world_space_vertex_positions(scene->sentinel_local_space_vertex_position_buffer + local_space_buffer_offset, world_space_vertex_position_buffer, sentinel_vertex_count, m);

	//lightray_compute_sentinel_world_space_vertex_positions(scene->sentinel_local_space_vertex_position_buffer + local_space_buffer_offset, scene->collision_mesh_buffer[collision_mesh_buffer_index].sentinel_world_space_vertex_position_buffer, scene->collision_mesh_buffer[collision_mesh_buffer_index].sentinel_vertex_count, lightray_copy_glm_mat4_to_sunder(scene->collision_mesh_buffer[collision_mesh_buffer_index].model->model));
}

void lightray_add_collision_layer(lightray_scene_t* scene, u32 collision_attribute_index, u64 collision_layer)
{
	SUNDER_SET_BIT(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask, collision_layer, 1ull);
	const u32 index_buffer_offset = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_index_buffer_offset;
	scene->collision_attribute_core.collision_layer_index_buffer[index_buffer_offset + scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_count] = SUNDER_CAST2(u32)collision_layer;
	scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_count++;
}

void lightray_remove_collision_layer(lightray_scene_t* scene, u32 collision_attribute_index, u64 collision_layer)
{
	SUNDER_ZERO_BIT(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask, collision_layer, 1ull);
	const u32 index_buffer_offset = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_index_buffer_offset;
	const u32 last_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_count - 1;
	u32 swap_index = 0;

	for (u32 i = 0; i < last_index + 1; i++)
	{
		if (scene->collision_attribute_core.collision_layer_index_buffer[index_buffer_offset + i] == SUNDER_CAST2(u32)collision_layer)
		{
			swap_index = i;
			break;
		}
	}

	scene->collision_attribute_core.collision_layer_index_buffer[index_buffer_offset + swap_index] = scene->collision_attribute_core.collision_layer_index_buffer[index_buffer_offset + last_index];
	scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_count--;
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
	SUNDER_SET_BIT(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_COLLIDED_WITH_BIT, 1u);
}

void lightray_disable_collision(lightray_scene_t* scene, u32 collision_attribute_index)
{
	SUNDER_ZERO_BIT(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_COLLIDED_WITH_BIT, 1u);
}

void lightray_enable_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 collision_attribute_index, u32 collision_mesh_index)
{
	SUNDER_SET_BIT(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].should_reproject_vertices_bitmask, collision_mesh_index, 1u);
}

void lightray_disable_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 collision_attribute_index, u32 collision_mesh_index)
{
	SUNDER_ZERO_BIT(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].should_reproject_vertices_bitmask, collision_mesh_index, 1u);
}

void lightray_add_collision_attribute_flags(lightray_scene_t* scene, u32 collision_attribute_index, u32 flags)
{
	scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].flags |= flags;
}

void lightray_set_pierce_layer(lightray_scene_t* scene, u32 collision_attribute_index, u16 layer)
{
	scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].pierce_layer = layer;
}

b32 lightray_collision_attribute_collides(const lightray_scene_t* scene, u32 collision_mesh_attribute_index)
{
	return SUNDER_IS_ANY_BIT_SET(scene->collision_attribute_core.collision_attribute_buffer.buffer[collision_mesh_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_COLLIDES_BIT, 1u);
}

b32 lightray_collision_attribute_aabb_collides(const lightray_scene_t* scene, u32 collision_mesh_attribute_index)
{
	return SUNDER_IS_ANY_BIT_SET(scene->collision_attribute_core.collision_attribute_buffer.buffer[collision_mesh_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_AABB_COLLIDES_BIT, 1u);
}

b32 lightray_collision_attribute_collision_meshes_collide(const lightray_scene_t* scene, u32 collision_mesh_attribute_index)
{
	return SUNDER_IS_ANY_BIT_SET(scene->collision_attribute_core.collision_attribute_buffer.buffer[collision_mesh_attribute_index].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_COLLISION_MESHES_COLLIDE_BIT, 1u);
}

void lightray_set_collision_attribute_entity_kind(lightray_scene_t* scene, u32 collision_mesh_attribute_index, u32 game_side_entity_kind)
{
	scene->collision_attribute_core.collision_attribute_buffer.buffer[collision_mesh_attribute_index].game_side_entity_kind = game_side_entity_kind;
}

b32 lightray_ray_aabb_intersect(const lightray_ray_t* ray, const sunder_v3_t& aabb_position, const sunder_v3_t& aabb_scale, f32* t_hit, sunder_v3_t* normal)
{
	f32 t_min = -FLT_MAX;
	f32 t_max = ray->distance;

	i32 hit_axis = -1;
	i32 hit_sign = 0;

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

			i32 sign_t1 = -1;
			i32 sign_t2 = +1;

			// Swap if direction is negative
			if (t1 > t2)
			{
				f32 temp = t1;
				t1 = t2;
				t2 = temp;

				i32 temp_sign = sign_t1;
				sign_t1 = sign_t2;
				sign_t2 = temp_sign;
			}

			if (t1 > t_min)
			{
				t_min = t1;
				hit_axis = axis;
				hit_sign = sign_t1;
			}	

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

	if (hit_axis == 0)
	{
		normal->x = (f32)hit_sign;
	}

	if (hit_axis == 1)
	{
		normal->y = (f32)hit_sign;
	}

	if (hit_axis == 2)
	{
		normal->z = (f32)hit_sign;
	}

	return SUNDER_TRUE;
}

b32 lightray_ray_aabb_intersect_precomputed(const lightray_ray_t* ray, const sunder_v3_t& aabb_min, const sunder_v3_t& aabb_max, f32* t_hit, sunder_v3_t* normal)
{
	f32 t_min = -FLT_MAX;
	f32 t_max = ray->distance;

	i32 hit_axis = -1;
	i32 hit_sign = 0;

	const sunder_v3_t ray_origin = sunder_v3(ray->origin.x, ray->origin.y, ray->origin.z);
	const sunder_v3_t ray_direction = sunder_v3(ray->direction.x, ray->direction.y, ray->direction.z);

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

			i32 sign_t1 = -1;
			i32 sign_t2 = +1;

			// Swap if direction is negative
			if (t1 > t2)
			{
				f32 temp = t1;
				t1 = t2;
				t2 = temp;

				i32 temp_sign = sign_t1;
				sign_t1 = sign_t2;
				sign_t2 = temp_sign;
			}

			if (t1 > t_min)
			{
				t_min = t1;
				hit_axis = axis;
				hit_sign = sign_t1;
			}

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
	}

	if (hit_axis == 0)
	{
		normal->x = (f32)hit_sign;
	}

	if (hit_axis == 1)
	{
		normal->y = (f32)hit_sign;
	}

	if (hit_axis == 2)
	{
		normal->z = (f32)hit_sign;
	}

	return SUNDER_TRUE;
}

sunder_v3_t lightray_get_grid_cell_center(const lightray_grid_t* grid, u32 row_index, u32 column_index)
{
	return grid->origin + sunder_v3((grid->column_count - 1 - column_index + 0.5f) * grid->cell_width, (row_index + 0.5f) * grid->cell_height, 0.0f);
}

lightray_raycast_memory_reserve_result_t lightray_reserve_raycast_memory(lightray_scene_t* scene, u32 raycast_grid_cell_index_per_ray_count, u32 raycast_pierce_layer_test_data_per_ray_count, lightray_raycast_intersection_mode_e intersection_mode)
{
	lightray_raycast_memory_reserve_result_t result{};
	result.success = SUNDER_FALSE;

	if (!sunder_valid_offset(scene->raycast_core.raycast_grid_cell_index_subarena.offset, scene->raycast_core.raycast_grid_cell_index_subarena.capacity))
	{
		return result;
	}

	if (!sunder_valid_offset(scene->raycast_core.pierce_layer_test_data_subarena.offset, scene->raycast_core.pierce_layer_test_data_subarena.capacity))
	{
		return result;
	}

	u32 has_already_failed_to_trace_attribute_bit_buffer_offset = 0;
	u32 has_already_been_traced_attribute_bit_buffer_offset = 0;

	if (intersection_mode == LIGHTRAY_RAYCAST_INTERSECTION_MODE_OVERLAP_MESH)
	{
		if (!sunder_valid_offset(scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.offset, scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.capacity))
		{
			return result;
		}

		if (!sunder_valid_offset(scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.offset, scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.capacity))
		{
			return result;
		}

		has_already_failed_to_trace_attribute_bit_buffer_offset = scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.offset;
		has_already_been_traced_attribute_bit_buffer_offset = scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.offset;

		scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.offset += scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer.count_per;
		scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.offset += scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer.count_per;
	}
	
	else if (intersection_mode == LIGHTRAY_RAYCAST_INTERSECTION_MODE_COLLISION_MESH)
	{
		if (!sunder_valid_offset(scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.offset, scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.capacity))
		{
			return result;
		}

		if (!sunder_valid_offset(scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.offset, scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.capacity))
		{
			return result;
		}

		has_already_failed_to_trace_attribute_bit_buffer_offset = scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.offset;
		has_already_been_traced_attribute_bit_buffer_offset = scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.offset;

		scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.offset += scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer.count_per;
		scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.offset += scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer.count_per;
	}

	const u32 raycast_grid_cell_index_subarena_offset = scene->raycast_core.raycast_grid_cell_index_subarena.offset;
	const u32 pierce_layer_test_data_subarena_offset = scene->raycast_core.pierce_layer_test_data_subarena.offset;

	scene->raycast_core.raycast_grid_cell_index_subarena.offset += raycast_grid_cell_index_per_ray_count;
	scene->raycast_core.pierce_layer_test_data_subarena.offset += raycast_pierce_layer_test_data_per_ray_count;

	result.grid_cell_index_subarena_offset = raycast_grid_cell_index_subarena_offset;
	result.pierce_layer_test_data_subarena_offset = pierce_layer_test_data_subarena_offset;

	result.has_already_been_traced_attribute_bit_buffer_offset = has_already_been_traced_attribute_bit_buffer_offset;
	result.has_already_failed_to_trace_attribute_bit_buffer_offset = has_already_failed_to_trace_attribute_bit_buffer_offset;

	result.grid_cell_index_per_ray_count = raycast_grid_cell_index_per_ray_count;
	result.pierce_layer_test_data_per_ray_count = raycast_pierce_layer_test_data_per_ray_count;

	result.success = SUNDER_TRUE;

	return result;
}

void lightray_reclaim_raycast_memory(lightray_scene_t* scene)
{
	scene->raycast_core.raycast_grid_cell_index_subarena.offset = 0;
	scene->raycast_core.pierce_layer_test_data_subarena.offset = 0;

	sunder_flush_bit_buffer(&scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer);
	sunder_flush_bit_buffer(&scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer);
	sunder_flush_bit_buffer(&scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer);
	sunder_flush_bit_buffer(&scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer);

	const u32 collision_attribute_count = scene->collision_attribute_core.collision_attribute_buffer.size;

	for (u32 i = 0; i < collision_attribute_count; i++)
	{
		const u32 collision_attribute_flags = scene->collision_attribute_core.collision_attribute_buffer[i].flags;
		const b32 is_static = SUNDER_IS_ANY_BIT_SET(collision_attribute_flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_STATIC_BIT, 1u);

		if (!is_static)
		{
			const u32 bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[i].collision_mesh_batch_index;
			const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_count;
			const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_buffer_offset;

			for (u32 cm = 0; cm < collision_mesh_count; cm++)
			{
				sunder_zero_bit_buffer_bit(&scene->raycast_core.has_already_cached_inverse_collision_mesh_bit_buffer, collision_mesh_buffer_offset + cm);
			}
		}
	}


	const u32 overlap_attribute_count = scene->overlap_attribute_core.overlap_attribute_buffer.size;

	for (u32 i = 0; i < overlap_attribute_count; i++)
	{
		const u32 overlap_attribute_flags = scene->overlap_attribute_core.overlap_attribute_buffer[i].flags;
		const b32 is_static = SUNDER_IS_ANY_BIT_SET(overlap_attribute_flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_STATIC_BIT, 1u);

		if (!is_static)
		{
			const u32 bound_overlap_mesh_batch_index = scene->overlap_attribute_core.overlap_attribute_buffer[i].overlap_mesh_batch_index;
			const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[bound_overlap_mesh_batch_index].overlap_mesh_count;
			const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[bound_overlap_mesh_batch_index].overlap_mesh_buffer_offset;

			for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
			{
				sunder_zero_bit_buffer_bit(&scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer, overlap_mesh_buffer_offset + ovm);
			}
		}
	}
}

u32 lightray_traverse_grid(u32 index, i32 direction)
{
	u32 val = index;
	val += direction;

	return val;
}

void lightray_handle_grid_traversal_aftermath(lightray_scene_t* scene, u32 cube_entity_index, const lightray_grid_t* grid, const lightray_ray_t* aabb_intersection_ray, u32 row_index, u32 column_index, u32 previous_grid_cell_index, u32* next_row_index, u32* next_column_index, u32* written_grid_cell_index_count, u32 max_grid_cell_index_count, u32 raycast_grid_cell_index_subarena_offset)
{
	const u32 grid_cell_index = lightray_get_grid_cell_index(grid, row_index, column_index);

	if (previous_grid_cell_index != grid_cell_index)
	{
		const sunder_v3_t grid_cell_center = lightray_get_grid_cell_center(grid, row_index, column_index);

		scene->position_buffer[cube_entity_index] = grid_cell_center;
		scene->scale_buffer[cube_entity_index] = sunder_v3(lightray_get_default_cube_grid_scale_x(grid), lightray_get_default_cube_grid_scale_y(grid), 90.0f);
		f32 t_hit = 0.0f;

		sunder_v3_t normal{};
		const b32 cell_on_ray_path = lightray_ray_aabb_intersect(aabb_intersection_ray, scene->position_buffer[cube_entity_index], scene->scale_buffer[cube_entity_index], &t_hit, &normal);

		if (cell_on_ray_path)
		{
			*next_row_index = row_index;
			*next_column_index = column_index;

			if (sunder_valid_index(*written_grid_cell_index_count, max_grid_cell_index_count))
			{
				scene->raycast_core.raycast_grid_cell_index_subarena[raycast_grid_cell_index_subarena_offset + (*written_grid_cell_index_count)] = grid_cell_index;
				(*written_grid_cell_index_count)++;
			}
		}
	}
}

bool lightray_quick_sort_compare_raycast_pierce_layer_test_data_squared_distance_less(const lightray_raycast_pierce_layer_test_data_t* i, const lightray_raycast_pierce_layer_test_data_t* j)
{
	return i->squared_distance < j->squared_distance;
}

SUNDER_IMPLEMENT_QUICK_SORT_PARTITION_FUNCTION(lightray_raycast_pierce_layer_test_data_t, raycast_pierce_layer_test_data, lightray)
SUNDER_IMPLEMENT_QUICK_SORT_FUNCTION(lightray_raycast_pierce_layer_test_data_t, raycast_pierce_layer_test_data, lightray)

lightray_raycast_result_t lightray_cast_ray(lightray_scene_t* scene, const lightray_raycast_data_t* data, const lightray_render_instance_t* render_instance_buffer)
{
	lightray_raycast_result_t result{};
	result.hit_count = 0;
	result.hit_any = SUNDER_FALSE;

	u32 written_grid_cell_index_count = 0;
	u32 written_attribute_count = 0;
	u32 filtered_attribute_count = 0;

	const sunder_v3_t ray_origin = data->ray.origin;
	const sunder_v3_t ray_direction = data->ray.direction;
	const f32 ray_distance = data->ray.distance;
	const sunder_v3_t ray_vec = sunder_ray_v3(ray_origin, ray_direction, ray_distance);

	u32 ray_vec_row_index = 0;
	u32 ray_vec_column_index = 0;

	const b32 ray_vec_in_bounds = lightray_get_grid_cell_coordinates(data->grid, &ray_vec, &ray_vec_row_index, &ray_vec_column_index);

	if (!ray_vec_in_bounds)
	{
		return result;
	}

	const u32 ray_vec_grid_cell_index = lightray_get_grid_cell_index(data->grid, ray_vec_row_index, ray_vec_column_index);

	u32 ray_origin_row_index = 0;
	u32 ray_origin_column_index = 0;

	const b32 ray_origin_in_bounds = lightray_get_grid_cell_coordinates(data->grid, &ray_origin, &ray_origin_row_index, &ray_origin_column_index);

	if (!ray_origin_in_bounds)
	{
		return result;
	}

	const u32 ray_origin_grid_cell_index = lightray_get_grid_cell_index(data->grid, ray_origin_row_index, ray_origin_column_index);

	u32 current_grid_cell_index = ray_origin_grid_cell_index;
	u32 current_row_index = ray_origin_row_index;
	u32 current_column_index = ray_origin_column_index;

	u32 previous_grid_cell_index = UINT32_MAX;

	const lightray_ray_t aabb_intersection_ray = data->ray;

	scene->raycast_core.raycast_grid_cell_index_subarena[data->memory_reserve_result.grid_cell_index_subarena_offset] = current_grid_cell_index;
	written_grid_cell_index_count++;

	while (current_grid_cell_index != ray_vec_grid_cell_index)
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
			lightray_handle_grid_traversal_aftermath(scene, data->cube_entity_index, data->grid, &aabb_intersection_ray, next_up_direction, current_column_index, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->memory_reserve_result.grid_cell_index_per_ray_count, data->memory_reserve_result.grid_cell_index_subarena_offset);
		}

		if (valid_down_direction)
		{
			lightray_handle_grid_traversal_aftermath(scene, data->cube_entity_index, data->grid, &aabb_intersection_ray, next_down_direction, current_column_index, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->memory_reserve_result.grid_cell_index_per_ray_count, data->memory_reserve_result.grid_cell_index_subarena_offset);
		}

		if (valid_left_direction)
		{
			lightray_handle_grid_traversal_aftermath(scene, data->cube_entity_index, data->grid, &aabb_intersection_ray, current_row_index, next_left_direction, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->memory_reserve_result.grid_cell_index_per_ray_count, data->memory_reserve_result.grid_cell_index_subarena_offset);
		}

		if (valid_right_direction)
		{
			lightray_handle_grid_traversal_aftermath(scene, data->cube_entity_index, data->grid, &aabb_intersection_ray, current_row_index, next_right_direction, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->memory_reserve_result.grid_cell_index_per_ray_count, data->memory_reserve_result.grid_cell_index_subarena_offset);
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

	sunder_v3_t closest_intersection_point{};
	sunder_v3_t closest_triangle_normal{};
	f32 closest_squared_distance = FLT_MAX;
	u32 closest_attribute_index = 0;

	if (data->intersection_mode == LIGHTRAY_RAYCAST_INTERSECTION_MODE_OVERLAP_MESH)
	{
		for (u32 i = 0; i < written_grid_cell_index_count; i++)
		{
			const u32 grid_cell_index = scene->raycast_core.raycast_grid_cell_index_subarena[data->memory_reserve_result.grid_cell_index_subarena_offset + i];
			const u32 overlap_attribute_index_count = data->grid->cell_buffer[grid_cell_index].overlap_attribute_index_count;
			const u32 overlap_attribute_index_buffer_offset = data->grid->cell_buffer[grid_cell_index].overlap_attribute_index_buffer_offset;

			for (u32 ovai = 0; ovai < overlap_attribute_index_count; ovai++)
			{
				const u32 overlap_attribute_index = data->grid->overlap_attribute_index_buffer[overlap_attribute_index_buffer_offset + ovai];
				const u32 overlap_attribute_flags = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].flags;

				if (!(SUNDER_IS_ANY_BIT_SET(overlap_attribute_flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT, 1u)))
				{
					continue;
				}

				if (scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer == (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_been_traced_attribute_bit_buffer_offset) + overlap_attribute_index))
				{
					continue;
				}

				if (scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer == (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + overlap_attribute_index))
				{
					continue;
				}

				const u64 ray_overlap_layer_bitmask = data->layer_bitmask;
				const u64 ray_cull_overlap_layer_bitmask = data->cull_layer_bitmask;
				const u64 overlap_layer_bitmask = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_bitmask;
				const u32 ray_overlap_layer_count = data->layer_index_buffer.size;
				u64 common_overlap_layer_bitmask = 0;

				for (u32 li = 0; li < ray_overlap_layer_count; li++)
				{
					const u32 current_bit = data->layer_index_buffer.buffer[li];

					const b32 ray_layer_check = SUNDER_IS_ANY_BIT_SET(ray_overlap_layer_bitmask, current_bit, 1ull);
					const b32 overlap_attribute_check = SUNDER_IS_ANY_BIT_SET(overlap_layer_bitmask, current_bit, 1ull);
					const b32 ray_cull_layer_check = SUNDER_IS_ANY_BIT_SET(ray_cull_overlap_layer_bitmask, current_bit, 1ull);

					if (ray_layer_check && overlap_attribute_check && !ray_cull_layer_check)
					{
						SUNDER_SET_BIT(common_overlap_layer_bitmask, current_bit, 1ull);
					}
				}

				if (common_overlap_layer_bitmask == 0)
				{
					continue;
				}

				const u32 overlap_attribute_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].entity_index;
				const u32 overlap_attribute_entity_flags = scene->entity_buffer[overlap_attribute_entity_index].flags;
				const u32 aabb_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].aabb_index;
				const u32 aabb_instance_model_binding_index = scene->entity_buffer[aabb_entity_index].instance_model_binding_index;
				const sunder_v3_t aabb_position = sunder_extract_translation_m4(lightray_copy_glm_mat4_to_sunder(render_instance_buffer[aabb_instance_model_binding_index].model.model));
				const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];

				f32 t_hit = 0.0f;
				sunder_v3_t aabb_hit_normal{};

				const b32 aabb_hit = lightray_ray_aabb_intersect(&aabb_intersection_ray, aabb_position, aabb_scale, &t_hit, &aabb_hit_normal);

				if (!aabb_hit)
				{
					scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + overlap_attribute_index);
					continue;
				}

				const b32 overlap_attribute_non_default_rotation = SUNDER_IS_ANY_BIT_SET(overlap_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
				const u32 overlap_mesh_batch_index = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_mesh_batch_index;
				const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_count;
				const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_buffer_offset;
				const sunder_v3_t* sentinel_local_space_vertex_position_buffer = scene->sentinel_local_space_vertex_position_buffer;

				u16 checked_overlap_mesh_bitmask = 0;

				for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
				{
					u32 overlap_mesh_flags = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].flags;
					b32 is_raw_mesh = SUNDER_FALSE;
					u16 chosen_index = 0;

					const b32 good = lightray_verify_overlap_mesh(scene, overlap_mesh_flags, overlap_mesh_buffer_offset, ovm, checked_overlap_mesh_bitmask, &chosen_index, &is_raw_mesh);

					if (!good)
					{
						SUNDER_SKIP;
					}

					SUNDER_SET_BIT(checked_overlap_mesh_bitmask, chosen_index, 1);

					u32 current_overlap_global_mesh_index = overlap_mesh_buffer_offset + chosen_index;

					u32 sentinel_local_space_vertex_position_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
					u32 index_count = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].index_count;
					u32 sentinel_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].sentinel_index_buffer_offset;
					u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].entity_index;
					u32 overlap_mesh_entity_flags = scene->entity_buffer[overlap_mesh_entity_index].flags;
					sunder_m4_t model = lightray_copy_glm_mat4_to_sunder(scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].model->model);
					sunder_v3_t overlap_mesh_scale = scene->scale_buffer[overlap_mesh_entity_index];
					sunder_v3_t overlap_mesh_translation = sunder_extract_translation_m4(model);

					f32 t_out = 0.0f;
					sunder_v3_t tri[3]{};

					b32 non_unit_scale = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
					b32 non_default_rotation = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);

					lightray_ray_t transformed_ray{};
					transformed_ray.distance = ray_distance;

					if(is_raw_mesh)
					{
						transformed_ray = lightray_transform_ray_to_ray_space(&data->ray, &scene->raycast_core.m4_inverse_cache_overlap_mesh_buffer, &scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer, model, overlap_mesh_scale, current_overlap_global_mesh_index, non_unit_scale, non_default_rotation, overlap_attribute_non_default_rotation);
						const b32 mesh_hit = lightray_ray_mesh_intersect(&transformed_ray, &t_out, data->culling_mode, tri, index_count, scene->sentinel_index_buffer, sentinel_index_buffer_offset, scene->sentinel_local_space_vertex_position_buffer, sentinel_local_space_vertex_position_buffer_offset);

						if (!mesh_hit)
						{
							scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + overlap_attribute_index);
							continue;
						}

						sunder_v3_t intersection_point{};
						sunder_v3_t triangle_normal{};
						f32 squared_distance = 0.0f;

						lightray_compute_ray_triangle_intersection_metadata(&transformed_ray, &data->ray, t_out, model, overlap_mesh_scale, tri, &intersection_point, &triangle_normal, &squared_distance, non_unit_scale, non_default_rotation, overlap_attribute_non_default_rotation);
						lightray_write_hit_attribute(scene, &data->memory_reserve_result, squared_distance, overlap_attribute_index, intersection_point, triangle_normal, &written_attribute_count, &closest_squared_distance, &closest_attribute_index, &closest_intersection_point, &closest_triangle_normal);

						scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_been_traced_attribute_bit_buffer_offset) + overlap_attribute_index);
						continue;
					}

					else
					{
						const b32 obb_hit = lightray_ray_aabb_intersect(&transformed_ray, overlap_mesh_translation, overlap_mesh_scale, &t_hit, &aabb_hit_normal);

						if (!obb_hit)
						{
							scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + overlap_attribute_index);
							continue;
						}

						chosen_index = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].obb_owner_index;
						SUNDER_SET_BIT(checked_overlap_mesh_bitmask, chosen_index, 1);
						current_overlap_global_mesh_index = overlap_mesh_buffer_offset + chosen_index;

						sentinel_local_space_vertex_position_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
						index_count = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].index_count;
						sentinel_index_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].sentinel_index_buffer_offset;
						overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].entity_index;
						overlap_mesh_entity_flags = scene->entity_buffer[overlap_mesh_entity_index].flags;
						model = lightray_copy_glm_mat4_to_sunder(scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].model->model);
						overlap_mesh_scale = scene->scale_buffer[overlap_mesh_entity_index];
						overlap_mesh_translation = sunder_extract_translation_m4(model);

						overlap_mesh_flags = scene->overlap_attribute_core.overlap_mesh_buffer[current_overlap_global_mesh_index].flags;
						non_unit_scale = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
						non_default_rotation = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);

						transformed_ray = lightray_transform_ray_to_ray_space(&data->ray, &scene->raycast_core.m4_inverse_cache_overlap_mesh_buffer, &scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer, model, overlap_mesh_scale, current_overlap_global_mesh_index, non_unit_scale, non_default_rotation, overlap_attribute_non_default_rotation);
						const b32 mesh_hit = lightray_ray_mesh_intersect(&transformed_ray, &t_out, data->culling_mode, tri, index_count, scene->sentinel_index_buffer, sentinel_index_buffer_offset, scene->sentinel_local_space_vertex_position_buffer, sentinel_local_space_vertex_position_buffer_offset);

						if (!mesh_hit)
						{
							scene->raycast_core.has_already_failed_to_trace_overlap_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + overlap_attribute_index);
							continue;
						}

						sunder_v3_t intersection_point{};
						sunder_v3_t triangle_normal{};
						f32 squared_distance = 0.0f;

						lightray_compute_ray_triangle_intersection_metadata(&transformed_ray, &data->ray, t_out, model, overlap_mesh_scale, tri, &intersection_point, &triangle_normal, &squared_distance, non_unit_scale, non_default_rotation, overlap_attribute_non_default_rotation);
						lightray_write_hit_attribute(scene, &data->memory_reserve_result, squared_distance, overlap_attribute_index, intersection_point, triangle_normal, &written_attribute_count, &closest_squared_distance, &closest_attribute_index, &closest_intersection_point, &closest_triangle_normal);

						scene->raycast_core.has_already_been_traced_overlap_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_been_traced_attribute_bit_buffer_offset) + overlap_attribute_index);
						continue;
					}
				}
			}
		}

		if (written_attribute_count > 0)
		{
			if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_PIERCE_BIT, 1u))
			{
				if (written_attribute_count > 1)
				{
					lightray_quick_sort_raycast_pierce_layer_test_data(&scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.has_already_been_traced_attribute_bit_buffer_offset], 0, written_attribute_count - 1, lightray_quick_sort_compare_raycast_pierce_layer_test_data_squared_distance_less);
				}

				lightray_raycast_pierce_layer_t previous_overlap_attribute_pierce_layer{};
				previous_overlap_attribute_pierce_layer.layer = UINT16_MAX;
				previous_overlap_attribute_pierce_layer.threshold = UINT16_MAX;

				for (u32 pltd = 0; pltd < written_attribute_count; pltd++)
				{
					const u32 current_overlap_attribute_index = scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset + pltd].attribute_index;
					const u16 current_overlap_attribute_pierce_layer = scene->overlap_attribute_core.overlap_attribute_buffer[current_overlap_attribute_index].pierce_layer;

					b32 ray_able_to_pierce_through = SUNDER_FALSE;

					for (u32 rpl = 0; rpl < data->pierce_layer_buffer.size; rpl++)
					{
						const u16 current_ray_pierce_layer = data->pierce_layer_buffer.buffer[rpl].layer;
						const u16 current_ray_pierce_layer_threshold = data->pierce_layer_buffer.buffer[rpl].threshold;

						filtered_attribute_count++;
						scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset + pltd].previous_hit_pierce_layer = previous_overlap_attribute_pierce_layer.layer;
						scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset + pltd].previous_hit_applied_threshold = previous_overlap_attribute_pierce_layer.threshold;

						if (current_ray_pierce_layer == current_overlap_attribute_pierce_layer)
						{
							if (current_ray_pierce_layer_threshold > 0 || SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_MAX_PIERCE_THRESHOLD_BIT, 1u))
							{
								previous_overlap_attribute_pierce_layer.layer = current_overlap_attribute_pierce_layer;
								previous_overlap_attribute_pierce_layer.threshold = sunder_clamp_u16(0, current_ray_pierce_layer_threshold, current_ray_pierce_layer_threshold - 1);
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
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].attribute_index = closest_attribute_index;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].intersection_point = closest_intersection_point;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].triangle_normal = closest_triangle_normal;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].squared_distance = closest_squared_distance;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].previous_hit_pierce_layer = UINT16_MAX;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].previous_hit_applied_threshold = UINT16_MAX;

				filtered_attribute_count = 1;
			}
		}

		result.hit_count = filtered_attribute_count;
		result.hit_any = SUNDER_TRUE;

		return result;
	}

	else if (data->intersection_mode == LIGHTRAY_RAYCAST_INTERSECTION_MODE_COLLISION_MESH)
	{
		for (u32 i = 0; i < written_grid_cell_index_count; i++)
		{
			const u32 grid_cell_index = scene->raycast_core.raycast_grid_cell_index_subarena[data->memory_reserve_result.grid_cell_index_subarena_offset + i];
			const u32 collision_attribute_index_count = data->grid->cell_buffer[grid_cell_index].collision_attribute_index_count;
			const u32 collision_attribute_index_buffer_offset = data->grid->cell_buffer[grid_cell_index].collision_attribute_index_buffer_offset;

			for (u32 cai = 0; cai < collision_attribute_index_count; cai++)
			{
				const u32 collision_attribute_index = data->grid->collision_attribute_index_buffer[collision_attribute_index_buffer_offset + cai];
				const u32 collision_attribute_flags = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].flags;

				if (!(SUNDER_IS_ANY_BIT_SET(collision_attribute_flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT, 1u)))
				{
					continue;
				}

				if (scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer == (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_been_traced_attribute_bit_buffer_offset) + collision_attribute_index))
				{
					continue;
				}

				if (scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer == (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + collision_attribute_index))
				{
					continue;
				}

				const u64 ray_collision_layer_bitmask = data->layer_bitmask;
				const u64 ray_cull_collision_layer_bitmask = data->cull_layer_bitmask;
				const u64 collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask;
				const u32 ray_collision_layer_count = data->layer_index_buffer.size;
				u64 common_collision_layer_bitmask = 0;

				for (u32 li = 0; li < ray_collision_layer_count; li++)
				{
					const u32 current_bit = data->layer_index_buffer.buffer[li];

					const b32 ray_layer_check = SUNDER_IS_ANY_BIT_SET(ray_collision_layer_bitmask, current_bit, 1ull);
					const b32 collision_attribute_check = SUNDER_IS_ANY_BIT_SET(collision_layer_bitmask, current_bit, 1ull);
					const b32 ray_cull_layer_check = SUNDER_IS_ANY_BIT_SET(ray_cull_collision_layer_bitmask, current_bit, 1ull);

					if (ray_layer_check && collision_attribute_check && !ray_cull_layer_check)
					{
						SUNDER_SET_BIT(common_collision_layer_bitmask, current_bit, 1ull);
					}
				}

				if (common_collision_layer_bitmask == 0)
				{
					continue;
				}

				const u32 collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].entity_index;
				const u32 collision_attribute_entity_flags = scene->entity_buffer[collision_attribute_entity_index].flags;
				const u32 aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].aabb_index;
				const u32 aabb_instance_model_binding_index = scene->entity_buffer[aabb_entity_index].instance_model_binding_index;
				const sunder_v3_t aabb_position = sunder_extract_translation_m4(lightray_copy_glm_mat4_to_sunder(render_instance_buffer[aabb_instance_model_binding_index].model.model));
				const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];

				f32 t_hit = 0.0f;
				sunder_v3_t aabb_hit_normal{};

				const b32 aabb_hit = lightray_ray_aabb_intersect(&aabb_intersection_ray, aabb_position, aabb_scale, &t_hit, &aabb_hit_normal);

				if (!aabb_hit)
				{
					scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + collision_attribute_index);
					continue;
				}

				const b32 collision_attribute_non_default_rotation = SUNDER_IS_ANY_BIT_SET(collision_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
				const u32 collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_mesh_batch_index;
				const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count;
				const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset;
				const u32 should_reproject_vertices_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].should_reproject_vertices_bitmask;
				const sunder_v3_t* sentinel_local_space_vertex_position_buffer = scene->sentinel_local_space_vertex_position_buffer;

				for (u32 cm = 0; cm < collision_mesh_count; cm++)
				{
					const u32 index_count = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].vertex_count;
					const u32 sentinel_index_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_index_buffer_offset;
					const u32 sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset;
					const sunder_m4_t model = lightray_copy_glm_mat4_to_sunder(scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].model->model);
					const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].entity_index;
					const u32 collision_mesh_entity_flags = scene->entity_buffer[collision_mesh_entity_index].flags;
					const sunder_v3_t collision_mesh_scale = scene->scale_buffer[collision_mesh_entity_index];
					const sunder_v3_t collision_mesh_translation = sunder_extract_translation_m4(model);

					lightray_ray_t transformed_ray{};
					f32 t_out = 0.0f;
					sunder_v3_t tri[3]{};

					const b32 non_unit_scale = SUNDER_IS_ANY_BIT_SET(collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
					const b32 non_default_rotation = SUNDER_IS_ANY_BIT_SET(collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);

					transformed_ray = lightray_transform_ray_to_ray_space(&data->ray, &scene->raycast_core.m4_inverse_cache_collision_mesh_buffer, &scene->raycast_core.has_already_cached_inverse_collision_mesh_bit_buffer, model, collision_mesh_scale, collision_mesh_buffer_offset + cm, non_unit_scale, non_default_rotation, collision_attribute_non_default_rotation);
					const b32 mesh_hit = lightray_ray_mesh_intersect(&transformed_ray, &t_out, data->culling_mode, tri, index_count, scene->sentinel_index_buffer, sentinel_index_buffer_offset, scene->sentinel_local_space_vertex_position_buffer, sentinel_local_space_vertex_position_buffer_offset);

					if (!mesh_hit)
					{
						scene->raycast_core.has_already_failed_to_trace_collision_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_failed_to_trace_attribute_bit_buffer_offset) + collision_attribute_index);
						continue;
					}

					sunder_v3_t intersection_point{};
					sunder_v3_t triangle_normal{};
					f32 squared_distance = 0.0f;

					lightray_compute_ray_triangle_intersection_metadata(&transformed_ray, &data->ray, t_out, model, collision_mesh_scale, tri, &intersection_point, &triangle_normal, &squared_distance, non_unit_scale, non_default_rotation, collision_attribute_non_default_rotation);
					lightray_write_hit_attribute(scene, &data->memory_reserve_result, squared_distance, collision_attribute_index, intersection_point, triangle_normal, &written_attribute_count, &closest_squared_distance, &closest_attribute_index, &closest_intersection_point, &closest_triangle_normal);

					scene->raycast_core.has_already_been_traced_collision_attribute_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(data->memory_reserve_result.has_already_been_traced_attribute_bit_buffer_offset) + collision_attribute_index);
				}
			}
		}

		if (written_attribute_count > 0)
		{
			if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_PIERCE_BIT, 1u))
			{
				if (written_attribute_count > 1)
				{
					lightray_quick_sort_raycast_pierce_layer_test_data(&scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.has_already_been_traced_attribute_bit_buffer_offset], 0, written_attribute_count - 1, lightray_quick_sort_compare_raycast_pierce_layer_test_data_squared_distance_less);
				}

				lightray_raycast_pierce_layer_t previous_collision_attribute_pierce_layer{};
				previous_collision_attribute_pierce_layer.layer = UINT16_MAX;
				previous_collision_attribute_pierce_layer.threshold = UINT16_MAX;

				for (u32 pltd = 0; pltd < written_attribute_count; pltd++)
				{
					const u32 current_collision_attribute_index = scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset + pltd].attribute_index;
					const u16 current_collision_attribute_pierce_layer = scene->collision_attribute_core.collision_attribute_buffer[current_collision_attribute_index].pierce_layer;

					b32 ray_able_to_pierce_through = SUNDER_FALSE;

					for (u32 rpl = 0; rpl < data->pierce_layer_buffer.size; rpl++)
					{
						const u16 current_ray_pierce_layer = data->pierce_layer_buffer.buffer[rpl].layer;
						const u16 current_ray_pierce_layer_threshold = data->pierce_layer_buffer.buffer[rpl].threshold;

						filtered_attribute_count++;
						scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset + pltd].previous_hit_pierce_layer = previous_collision_attribute_pierce_layer.layer;
						scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset + pltd].previous_hit_applied_threshold = previous_collision_attribute_pierce_layer.threshold;

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
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].attribute_index = closest_attribute_index;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].intersection_point = closest_intersection_point;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].triangle_normal = closest_triangle_normal;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].squared_distance = closest_squared_distance;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].previous_hit_pierce_layer = UINT16_MAX;
				scene->raycast_core.pierce_layer_test_data_subarena[data->memory_reserve_result.pierce_layer_test_data_subarena_offset].previous_hit_applied_threshold = UINT16_MAX;

				filtered_attribute_count = 1;
			}
		}

		result.hit_count = filtered_attribute_count;
		result.hit_any = SUNDER_TRUE;

		return result;
	}

	else
	{
		return result;
	}

	/*
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
			lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, next_up_direction, current_column_index, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->raycast_grid_cell_index_count, scene->raycast_grid_cell_index_subarena, data->raycast_grid_cell_index_subarena_offset);
		}

		if (valid_down_direction)
		{
			lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, next_down_direction, current_column_index, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->raycast_grid_cell_index_count, scene->raycast_grid_cell_index_subarena, data->raycast_grid_cell_index_subarena_offset);
		}

		if (valid_left_direction)
		{
			lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, current_row_index, next_left_direction, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->raycast_grid_cell_index_count, scene->raycast_grid_cell_index_subarena, data->raycast_grid_cell_index_subarena_offset);
		}

		if (valid_right_direction)
		{
			lightray_handle_grid_traversal_aftermath(data->grid, scene->position_buffer, scene->scale_buffer, &aabb_intersection_ray, data->cube_entity_index, current_row_index, next_right_direction, previous_grid_cell_index, &next_row_index, &next_column_index, &written_grid_cell_index_count, data->raycast_grid_cell_index_count, scene->raycast_grid_cell_index_subarena, data->raycast_grid_cell_index_subarena_offset);
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
	sunder_v3_t closest_triangle_normal{};

	f32 t_hit_aabb = 0.0f;
	sunder_v3_t aabb_hit_normal{};

	u16 external_inverse_buffer_iter = 0;

	u32 filtered_pierce_layer_count = 0;

	if (data->intersection_mode == LIGHTRAY_RAYCAST_INTERSECTION_MODE_OVERLAP_MESH)
	{
		for (u32 i = 0; i < written_grid_cell_index_count; i++)
		{
			const u32 raycast_grid_cell_index = scene->raycast_grid_cell_index_subarena[data->raycast_grid_cell_index_subarena_offset + i];
			const u32 overlap_attribute_index_buffer_offset = data->grid->cell_buffer[raycast_grid_cell_index].overlap_attribute_index_buffer_offset;
			const u32 overlap_attribute_count = data->grid->cell_buffer[raycast_grid_cell_index].overlap_attribute_index_count;

			for (u32 ovai = 0; ovai < overlap_attribute_count; ovai++)
			{
				const u32 current_overlap_attribute_index = data->grid->overlap_attribute_index_buffer[overlap_attribute_index_buffer_offset + ovai];
				const u32 overlap_attribute_flags = scene->overlap_attribute_buffer[current_overlap_attribute_index].flags;

				if (!(SUNDER_IS_ANY_BIT_SET(overlap_attribute_flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT, 1u)))
				{
					continue;
				}

			}
		}
	}

	else if (data->intersection_mode == LIGHTRAY_RAYCAST_INTERSECTION_MODE_COLLISION_MESH)
	{
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
						const b32 cull_collision_layer_check = SUNDER_IS_ANY_BIT_SET(data->cull_collision_layer_bitmask, current_collision_layer_bit, 1ull);

						if (collision_attribute_collision_layer_check && ray_collision_layer_check && (!cull_collision_layer_check))
						{
							SUNDER_SET_BIT(common_collision_layer_bitmask, current_collision_layer_bit, 1ull);
						}

						//else if (collision_attribute_collision_layer_check && ray_collision_layer_check && cull_collision_layer_check)
						//{
							//should_cull = SUNDER_TRUE;
							//break;
						//}

						//else if (collision_attribute_collision_layer_check && !ray_collision_layer_check)
						//{
							//should_cull = SUNDER_TRUE;
							//break;
						//}
					}

					//if (should_cull)
					//{
						//should_cull = SUNDER_FALSE;
						//continue;
					//}

					if (common_collision_layer_bitmask == 0)
					{
						continue;
					}
				}

				const u32 collision_attribute_aabb_instance_model_binding_index = scene->entity_buffer[collision_attribute_aabb_entity_index].instance_model_binding_index;
				const sunder_v3_t collision_attribute_aabb_position = sunder_extract_translation_m4(lightray_copy_glm_mat4_to_sunder(render_instance_buffer[collision_attribute_aabb_instance_model_binding_index].model.model));
				const sunder_v3_t collision_attribute_aabb_scale = sunder_extract_scale_m4(lightray_copy_glm_mat4_to_sunder(render_instance_buffer[collision_attribute_aabb_instance_model_binding_index].model.model));

				const b32 collision_attribute_aabb_ray_intersect = lightray_ray_aabb_intersect(&aabb_intersection_ray, collision_attribute_aabb_position, collision_attribute_aabb_scale, &t_hit_aabb, &aabb_hit_normal);

				if (collision_attribute_aabb_ray_intersect)
				{
					const u32 collision_mesh_batch_index = scene->collision_attribute_buffer[current_cai].collision_mesh_batch_index;
					const u32 collision_mesh_count = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_count;
					const u32 collision_mesh_buffer_offset = scene->collision_mesh_batch_buffer[collision_mesh_batch_index].collision_mesh_buffer_offset;

					f32 out_t = 0.0f;
					sunder_v3_t hit_v0{};
					sunder_v3_t hit_v1{};
					sunder_v3_t hit_v2{};

					for (u32 cm = 0; cm < collision_mesh_count; cm++)
					{
						const sunder_m4_t model = lightray_copy_glm_mat4_to_sunder(scene->collision_mesh_buffer[collision_mesh_buffer_offset + cm].model->model);
						sunder_m4_t model_inverse{};

						if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_EXTERNAL_INVERSE_READ_BIT, 1u))
						{
							model_inverse = external_inverse_buffer[external_inverse_buffer_iter];
						}

						else
						{
							model_inverse = sunder_m4_inverse(model);
						}

						if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_EXTERNAL_INVERSE_WRITE_BIT, 1u))
						{
							external_inverse_buffer[external_inverse_buffer_iter] = model_inverse;
							(*external_inverse_written_count)++;
							external_inverse_buffer_iter++;
						}

						const u32 sentinel_index_buffer_offset = scene->collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_index_buffer_offset;
						const u32 sentinel_local_space_vertex_position_buffer_offset = scene->collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset;
						const sunder_v3_t* sentinel_local_space_vertex_position_buffer = scene->sentinel_local_space_vertex_position_buffer;
						const u32 collision_mesh_vertex_count = scene->collision_mesh_buffer[collision_mesh_buffer_offset + cm].vertex_count;

						const sunder_v4_t local_ray_origin_v4 = model_inverse * sunder_v4(ray_origin.x, ray_origin.y, ray_origin.z, 1.0f);
						const sunder_v4_t local_ray_direction_v4 = model_inverse * sunder_v4(ray_direction.x, ray_direction.y, ray_direction.z, 0.0f);

						lightray_ray_t transformed_ray{};
						transformed_ray.origin = sunder_v3(local_ray_origin_v4.x, local_ray_origin_v4.y, local_ray_origin_v4.z);
						transformed_ray.direction = sunder_normalize_v3(sunder_v3(local_ray_direction_v4.x, local_ray_direction_v4.y, local_ray_direction_v4.z));
						transformed_ray.distance = ray_distance;

						b32 collision_mesh_ray_intersect = SUNDER_FALSE;
						sunder_v3_t intersection_point{};
						sunder_v3_t triangle_normal{};

						u32* chosen_sentinel_index_buffer = scene->sentinel_index_buffer;
						u32 chosen_sentinel_index_buffer_offset = sentinel_index_buffer_offset;
						u32 chosen_collision_mesh_vertex_count = collision_mesh_vertex_count;

						if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_ENTRY_BIT, 1ull))
						{
							if (current_cai == data->capsule_collision_attribute_index)
							{
								if (cm == data->capsule_collision_mesh_index)
								{
									chosen_sentinel_index_buffer = scene->capsule_beyond_lower_clipping_plane_triangle_index_buffer;
									chosen_sentinel_index_buffer_offset = 0;
									chosen_collision_mesh_vertex_count = scene->capsule_beyond_lower_clipping_plane_triangle_index_count;
								}
							}
						}

						for (u32 v = 0; v < chosen_collision_mesh_vertex_count; v += 3)
						{
							f32 out_u = 0.0f;
							f32 out_v = 0.0f;

							sunder_v3_t tri[3]{};
							u32 tri_indices[3]{};

							tri_indices[0] = chosen_sentinel_index_buffer[chosen_sentinel_index_buffer_offset + v + 0];
							tri_indices[1] = chosen_sentinel_index_buffer[chosen_sentinel_index_buffer_offset + v + 1];
							tri_indices[2] = chosen_sentinel_index_buffer[chosen_sentinel_index_buffer_offset + v + 2];

							tri[0] = scene->sentinel_local_space_vertex_position_buffer[sentinel_local_space_vertex_position_buffer_offset + tri_indices[0]];
							tri[1] = scene->sentinel_local_space_vertex_position_buffer[sentinel_local_space_vertex_position_buffer_offset + tri_indices[1]];
							tri[2] = scene->sentinel_local_space_vertex_position_buffer[sentinel_local_space_vertex_position_buffer_offset + tri_indices[2]];

							const b32 ray_triangle_intersect = lightray_ray_triangle_intersect(&transformed_ray, tri, &out_t, &out_u, &out_v, data->culling_mode, &hit_v0, &hit_v1, &hit_v2);

							if (ray_triangle_intersect)
							{
								intersection_point = transformed_ray.origin + transformed_ray.direction * out_t;
								intersection_point = sunder_v3_v4(model * sunder_v4(intersection_point.x, intersection_point.y, intersection_point.z, 1.0f));

								// normal computation is done in world space because blender is fucking awesome and distorts local space triangle vertices sometimes))
								hit_v0 = sunder_v3_v4(model * sunder_v4_v3(hit_v0, 1.0f));
								hit_v1 = sunder_v3_v4(model * sunder_v4_v3(hit_v1, 1.0f));
								hit_v2 = sunder_v3_v4(model * sunder_v4_v3(hit_v2, 1.0f));

								const sunder_v3_t AB = hit_v1 - hit_v0;
								const sunder_v3_t AC = hit_v2 - hit_v0;

								triangle_normal = sunder_normalize_v3(sunder_cross_v3(AB, AC));

								collision_mesh_ray_intersect = SUNDER_TRUE;
								break;
							}
						}

						if (collision_mesh_ray_intersect)
						{
							SUNDER_SET_BUFFERED_BIT(scene->has_already_been_traced_bitmask_subarena, buffered_bit_offset + current_cai);

							if (sunder_valid_index(written_pierce_layer_test_data_count, data->raycast_pierce_layer_test_data_count))
							{
								const f32 intersection_ray_squared_distance = sunder_squared_distance_v3(intersection_point, ray_origin);

								if (intersection_ray_squared_distance < closest_squared_distance)
								{
									closest_squared_distance = intersection_ray_squared_distance;
									closest_collision_attribute_index = current_cai;
									closest_intersection_point = intersection_point;
									closest_triangle_normal = triangle_normal;
								}

								scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + written_pierce_layer_test_data_count].intersection_point = intersection_point;
								scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset + written_pierce_layer_test_data_count].triangle_normal = triangle_normal;
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
				scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].triangle_normal = closest_triangle_normal;
				scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].squared_distance = closest_squared_distance;
				scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].collision_attribute_index = closest_collision_attribute_index;
				scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].previous_hit_pierce_layer = UINT16_MAX;
				scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].previous_hit_applied_threshold = UINT16_MAX;

				filtered_pierce_layer_count = 1;
			}
		}
	}

	*/

	//if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_MARK_INTERSECTION_POINT_BIT, 1u) && filtered_pierce_layer_count > 0)
	//{
		//scene->position_buffer[data->cube_entity_index] = scene->pierce_layer_test_data_subarena[data->raycast_pierce_layer_test_data_subarena_offset].intersection_point;
		//scene->scale_buffer[data->cube_entity_index] = sunder_v3(0.05f, 0.05f, 0.05f);
	//}
	
	/*
	if (SUNDER_IS_ANY_BIT_SET(data->flags, LIGHTRAY_RAYCAST_BITS_TRACE_LINE_BIT, 1u))
	{
		const sunder_quat_t line_trace_rotation = sunder_rotate_from_to_v3(sunder_v3(0.0f, 1.0f, 0.0f), ray_direction);

		const f32 ray_offset = 0.20f * ray_distance;

		scene->position_buffer[data->line_trace_entity_index] = ray_origin + ray_direction * ray_offset;
		scene->quat_rotation_buffer[data->line_trace_entity_index] = line_trace_rotation;
		scene->scale_buffer[data->line_trace_entity_index].y = ray_distance * 1.225f;
		scene->scale_buffer[data->line_trace_entity_index].z = 0.01f;
		scene->scale_buffer[data->line_trace_entity_index].x = 0.01f;
	}
	*/

	//*out_raycast_pierce_layer_test_data_count = filtered_pierce_layer_count;



	return result;
}

lightray_ray_t lightray_ray(const sunder_v3_t& origin, const sunder_v3_t& direction, f32 distance)
{
	lightray_ray_t result{};

	result.origin = origin;
	result.direction = direction;
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

void lightray_bind_entity_to_camera(lightray_scene_t* scene, u32 self_entity_index, u32 camera_index)
{
	//if (!(sunder_valid_index(self_entity_index, scene->total_entity_count)))
	//{
		//return;
	//}

	//scene->entity_buffer[self_entity_index].camera_binding_index = camera_index;
}

void lightray_bind_entity_to_bone(lightray_scene_t* scene, u32 self_entity_index, u32 bone_owner_entity_index, u32 bone_computed_transform_matrix_buffer_index)
{
	if (!(sunder_valid_index(self_entity_index, scene->total_entity_count)))
	{
		return;
	}

	if (!(sunder_valid_index(bone_owner_entity_index, scene->total_entity_count)))
	{
		return;
	}

	lightray_bind_entity(scene, self_entity_index, bone_owner_entity_index);
	scene->entity_buffer[self_entity_index].bone_binding_index = bone_computed_transform_matrix_buffer_index;
}

void lightray_traverse_entity_binding_chain(lightray_scene_t* scene, u32* current_binding_chain_entity_index, u32 entity_binding_chain_index_buffer_offset, u32* entity_binding_chain_index_count)
{
	if (*current_binding_chain_entity_index == UINT32_MAX)
	{
		return;
	}

	SUNDER_SET_BUFFERED_BIT(scene->already_part_of_other_binding_chain_bitmask_buffer, *current_binding_chain_entity_index);
	const u32 children_count = scene->entity_buffer[*current_binding_chain_entity_index].children_index_count;

	SUNDER_LOG("\nbinding_chain_entity_index: ");
	SUNDER_LOG(*current_binding_chain_entity_index);

	scene->entity_binding_chain_index_buffer[entity_binding_chain_index_buffer_offset + (*entity_binding_chain_index_count)] = *current_binding_chain_entity_index;
	(*entity_binding_chain_index_count)++;

	if (children_count == 0)
	{
		const u32 parent_index = scene->entity_buffer[*current_binding_chain_entity_index].parent_index;
		*current_binding_chain_entity_index = parent_index;

		return;
	}

	const u32 entity_children_index_buffer_offset = lightray_get_entity_children_index_buffer_offset(scene, *current_binding_chain_entity_index);
	const u32 next_binding_chain_entity_index = scene->entity_children_index_buffer[entity_children_index_buffer_offset];
	*current_binding_chain_entity_index = next_binding_chain_entity_index;

	for (u32 i = 0; i < children_count; i++)
	{
		lightray_traverse_entity_binding_chain(scene, current_binding_chain_entity_index, entity_binding_chain_index_buffer_offset, entity_binding_chain_index_count);
	}
}

void lightray_add_entity_flags(lightray_scene_t* scene, u32 entity_index, u16 flags)
{
	if (!(sunder_valid_index(entity_index, scene->total_entity_count)))
	{
		return;
	}

	scene->entity_buffer[entity_index].flags |= flags;
}

void lightray_remove_entity_flags(lightray_scene_t* scene, u32 entity_index, u16 flags)
{
	if (!(sunder_valid_index(entity_index, scene->total_entity_count)))
	{
		return;
	}

	scene->entity_buffer[entity_index].flags &= ~flags;
}

void lightray_epa_add_face(lightray_epa_face_t* f, u32 face_count, const lightray_gjk_support_point_t* verts, u32 vert_count)
{


}

lightray_epa_result_t lightray_solve_epa(lightray_gjk_support_point_t* simplex, u32 simplex_size, const sunder_v3_t* vertices_a, u32 vertex_count_a, const sunder_v3_t* vertices_b, u32 vertex_count_b)
{














	
	lightray_epa_result_t result{};
	result.hit = false;

	const int MAX_FACES = 64;
	const int MAX_VERTS = 64;
	const float EPA_EPSILON = 1e-4f;

	// 1. Copy simplex to EPA vertices
	lightray_gjk_support_point_t verts[MAX_VERTS];
	int vert_count = simplex_size;
	for (u32 i = 0; i < simplex_size; i++) verts[i] = simplex[i];

	// 2. Build initial tetrahedron faces
	struct Face { int a, b, c; sunder_v3_t normal; float distance; bool obsolete; };
	Face faces[MAX_FACES];
	int face_count = 0;

	auto add_face = [&](int i0, int i1, int i2) 
	{

			if (face_count >= MAX_FACES) return;
		Face f{ i0, i1, i2 };
		sunder_v3_t v0 = verts[i0].p;
		sunder_v3_t v1 = verts[i1].p;
		sunder_v3_t v2 = verts[i2].p;
		f.normal = sunder_normalize_v3(sunder_cross_v3(v1 - v0, v2 - v0));
		f.distance = sunder_dot_v3(f.normal, v0);
		f.obsolete = false;
		faces[face_count++] = f;
	};

	add_face(0, 1, 2);
	add_face(0, 2, 3);
	add_face(0, 3, 1);
	add_face(1, 3, 2);

	// 3. Main EPA loop
	for (int iter = 0; iter < 64; iter++)
	{
		// Find closest face
		float minDist = FLT_MAX;
		int closest = -1;
		for (int i = 0; i < face_count; i++)
		{
			if (!faces[i].obsolete && faces[i].distance < minDist)
			{
				minDist = faces[i].distance;
				closest = i;
			}
		}
		if (closest == -1) break;

		Face& cf = faces[closest];

		// Support in direction of face normal
		lightray_gjk_support_point_t newPt = lightray_gjk_support(vertices_a, vertex_count_a, vertices_b, vertex_count_b, cf.normal);
		float newDist = sunder_dot_v3(newPt.p, cf.normal);

		// Convergence
		if (newDist - cf.distance < EPA_EPSILON)
		{
			result.hit = true;
			result.depth = cf.distance;
			result.normal = cf.normal;

			// Compute contact point in A/B
			result.contact_point = (verts[cf.a].a + verts[cf.b].a + verts[cf.c].a +
				verts[cf.a].b + verts[cf.b].b + verts[cf.c].b) / 6.0f;
			return result;
		}

		// Add new point
		if (vert_count >= MAX_VERTS) break;
		int new_i = vert_count++;
		verts[new_i] = newPt;

		// Mark faces visible to new point as obsolete
		for (int i = 0; i < face_count; i++)
		{
			if (!faces[i].obsolete) 
			{
				sunder_v3_t a = verts[faces[i].a].p;
				if (sunder_dot_v3(newPt.p - a, faces[i].normal) > 0)
					faces[i].obsolete = true;
			}
		}

		// Build horizon & new faces
		// (simplified for clarity; see real EPA for proper horizon construction)
		int oldCount = face_count;
		for (int i = 0; i < oldCount; i++)
		{
			if (faces[i].obsolete) {
				add_face(faces[i].a, faces[i].b, new_i);
				add_face(faces[i].b, faces[i].c, new_i);
				add_face(faces[i].c, faces[i].a, new_i);
			}
		}
	}

	return result;
	

	









	//////////////////////////////////////////////////





}

sunder_v3_t lightray_add_impulse(const sunder_v3_t& velocity, const sunder_v3_t& impulse)
{
	return velocity + impulse;
}

b32 lightray_aabb_swept(const sunder_v3_t& movement_direction, const sunder_v3_t& aabb_position_a, const sunder_v3_t& aabb_scale_a, const sunder_v3_t& aabb_position_b, const sunder_v3_t& aabb_scale_b, f32* time)
{
	sunder_v3_t a_min{};
	sunder_v3_t a_max{};

	lightray_compute_aabb_min_max(aabb_position_a, aabb_scale_a, &a_min, &a_max);

	sunder_v3_t b_min{};
	sunder_v3_t b_max{};

	lightray_compute_aabb_min_max(aabb_position_b, aabb_scale_b, &b_min, &b_max);

	SUNDER_LOG("\na_min: ");
	sunder_log_v3(a_min);
	SUNDER_LOG("\na_max: ");
	sunder_log_v3(a_max);

	SUNDER_LOG("\nb_max: ");
	sunder_log_v3(b_min);
	SUNDER_LOG("\nb_max: ");
	sunder_log_v3(b_max);


	sunder_v3_t size_a = (a_max - a_min) * 0.5f;
	SUNDER_LOG("\nsize_a: ");
	sunder_log_v3(size_a);
	

	const sunder_v3_t converted_size = aabb_scale_a / 10.0f; // need this for correct aabb conversion for my cube mesh
	const sunder_v3_t b_expanded_min = b_min - size_a;
	const sunder_v3_t b_expanded_max = b_max + size_a;
	
	SUNDER_LOG("\nb_expanded_min: ");
	sunder_log_v3(b_expanded_min);
	SUNDER_LOG("\nb_expanded_max: ");
	sunder_log_v3(b_expanded_max);

	f32 t_hit = 0.0f;

	lightray_ray_t ray{};
	ray.origin = aabb_position_a;
	ray.direction = movement_direction;
	ray.distance = 100.0f;

	sunder_v3_t normal{};
	const b32 ray_hit = lightray_ray_aabb_intersect_precomputed(&ray, b_expanded_min, b_expanded_max, &t_hit, &normal);

	*time = t_hit;

	return ray_hit;
}


b32 lightray_capsule_triangle_swept(const lightray_capsule_t* capsule, const sunder_v3_t* tri, sunder_v3_t* normal, f32* t)
{
	sunder_v3_t p1 = capsule->p1;
	sunder_v3_t p2 = capsule->p2;
	f32 r = capsule->r;
	sunder_v3_t d = p2 - p1;

	const sunder_v3_t AB = tri[1] - tri[0];
	const sunder_v3_t AC = tri[2] - tri[0];

	sunder_v3_t N = sunder_cross_v3(AB, AC);
	N = sunder_normalize_v3(N);

	f32 dist1 = sunder_dot_v3(p1 - tri[0], N);
	f32 dist2 = sunder_dot_v3(p2 - tri[0], N);

	if ((dist1 > r && dist2 > r) || (dist1 < -r && dist2 < -r))
	{
		return SUNDER_FALSE;
	}

	const f32 t_plane = (r - dist2) / (dist2 - dist1);

	if (t_plane < 0.0f || t_plane > 1.0f)
	{
		return SUNDER_FALSE;
	}

	const sunder_v3_t hit_center = p1 + d * t_plane - N * r;
	const sunder_v3_t AP = hit_center - tri[0];

	const f32 dot_AB_AB = sunder_dot_v3(AB, AB);
	const f32 dot_AB_AC = sunder_dot_v3(AB, AC);
	const f32 dot_AC_AC = sunder_dot_v3(AC, AC);
	const f32 dot_AP_AB = sunder_dot_v3(AP, AB);
	const f32 dot_AP_AC = sunder_dot_v3(AP, AC);

	const f32 denom = dot_AB_AB * dot_AC_AC - dot_AB_AC * dot_AB_AC;
	const f32 u = (dot_AC_AC * dot_AP_AB - dot_AB_AC * dot_AP_AC) / denom;
	const f32 v = (dot_AB_AB * dot_AP_AC - dot_AB_AC * dot_AP_AB) / denom;

	if (u >= 0.0f && v >= 0.0f && u + v <= 1.0f)
	{
		*normal = N;
		*t = t_plane;
		return SUNDER_TRUE;
	}

	return SUNDER_FALSE;
}

void lightray_cast_ray_cluster(u64 capsule_vertex_count, const u32* capsule_index_buffer, const sunder_v3_t* capsule_local_space_vertex_position_buffer, const sunder_m4_t& capsule_model, const sunder_m4_t& capsule_inverse, sunder_v3_t* capsule_intersection_point_buffer, u64 plane_vertex_count, const sunder_v3_t* plane_local_space_vertex_position_buffer, const sunder_m4_t& plane_model, const sunder_m4_t& plane_inverse, sunder_v3_t* plane_intersection_point_buffer, u32 ray_count, const sunder_v3_t* ray_origin_buffer, const sunder_v3_t& cluster_direction, u64* hit_bitmask_buffer_capsule, u64* hit_bitmask_buffer_plane)
{
	const sunder_v3_t capsule_ray_direction = sunder_normalize_v3(sunder_v3_v4(capsule_inverse * sunder_v4_v3(cluster_direction, 0.0f)));
	const sunder_v3_t plane_ray_direction = sunder_normalize_v3(sunder_v3_v4(plane_inverse * sunder_v4_v3(cluster_direction, 0.0f)));

	for (u32 i = 0; i < ray_count; i++)
	{
		lightray_ray_t capsule_ray{};
		capsule_ray.origin = sunder_v3_v4(capsule_inverse * sunder_v4_v3(ray_origin_buffer[i], 1.0f));
		capsule_ray.direction = capsule_ray_direction;
		capsule_ray.distance = 100.0f;

		for (u64 cv = 0; cv < capsule_vertex_count; cv += 3)
		{
			const u32 v0_index = capsule_index_buffer[cv + 0];
			const u32 v1_index = capsule_index_buffer[cv + 1];
			const u32 v2_index = capsule_index_buffer[cv + 2];

			sunder_v3_t tri[3]{};
			tri[0] = capsule_local_space_vertex_position_buffer[v0_index];
			tri[1] = capsule_local_space_vertex_position_buffer[v1_index];
			tri[2] = capsule_local_space_vertex_position_buffer[v2_index];

			f32 out_t = 0.0f;
			f32 out_u = 0.0f;
			f32 out_v = 0.0f;

			sunder_v3_t hit_v0{};
			sunder_v3_t hit_v1{};
			sunder_v3_t hit_v2{};

			const b32 ray_hit = lightray_ray_triangle_intersect(&capsule_ray, tri, &out_t, &out_u, &out_v, LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_NONE, &hit_v0, &hit_v1, &hit_v2);

			if (ray_hit)
			{
				sunder_v3_t intersection_point = capsule_ray.origin + capsule_ray.direction * out_t;
				intersection_point = sunder_v3_v4(capsule_model * sunder_v4_v3(intersection_point, 1.0f));
				capsule_intersection_point_buffer[i] = intersection_point;
				SUNDER_SET_BUFFERED_BIT(hit_bitmask_buffer_capsule, i);

				break;
			}
		}

		//if (!(SUNDER_IS_BUFFERED_BIT_SET(hit_bitmask_buffer_capsule, i)))
		//{
			//continue;
		//}

		lightray_ray_t plane_ray{};
		plane_ray.origin = sunder_v3_v4(plane_inverse * sunder_v4_v3(ray_origin_buffer[i], 1.0f));
		plane_ray.direction = plane_ray_direction;
		plane_ray.distance = 1000.0f;

		for (u64 pv = 0; pv < plane_vertex_count; pv += 3)
		{
			f32 out_t = 0.0f;
			f32 out_u = 0.0f;
			f32 out_v = 0.0f;

			sunder_v3_t hit_v0{};
			sunder_v3_t hit_v1{};
			sunder_v3_t hit_v2{};

			const b32 ray_hit = lightray_ray_triangle_intersect(&plane_ray, &plane_local_space_vertex_position_buffer[pv], &out_t, &out_u, &out_v, LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_BACKFACE, &hit_v0, &hit_v1, &hit_v2);

			if (ray_hit)
			{
				sunder_v3_t intersection_point = plane_ray.origin + plane_ray.direction * out_t;
				intersection_point = sunder_v3_v4(plane_model * sunder_v4_v3(intersection_point, 1.0f));
				plane_intersection_point_buffer[i] = intersection_point;
				SUNDER_SET_BUFFERED_BIT(hit_bitmask_buffer_plane, i);

				break;
			}
		}
	}
}

sunder_v3_t lightray_compute_segment_point(const sunder_v3_t& A, const sunder_v3_t& B, u32 index, f32 step, f32 z_alignment, b32 align_z)
{
	const sunder_v3_t V = B - A;
	sunder_v3_t W = A + V * SUNDER_CAST(f32, index) / step;

	if (align_z)
	{
		W.z = z_alignment;
	}

	return W;
}

f32 lightray_compute_capsule_to_plane_distance(lightray_capsule_to_plane_distance_computation_data_t* data)
{
	/*
	const sunder_v3_t v0 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[0]], 1.0f));
	const sunder_v3_t v1 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[1]], 1.0f));
	const sunder_v3_t v2 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[2]], 1.0f));
	const sunder_v3_t v3 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[3]], 1.0f));
	const sunder_v3_t v4 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[4]], 1.0f));
	const sunder_v3_t v5 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[5]], 1.0f));

	const sunder_v3_t v6 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[6]], 1.0f));
	const sunder_v3_t v7 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[7]], 1.0f));
	const sunder_v3_t v8 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[8]], 1.0f));
	const sunder_v3_t v9 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[9]], 1.0f));
	const sunder_v3_t v10 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[10]], 1.0f));
	const sunder_v3_t v11 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[11]], 1.0f));

	const sunder_v3_t v12 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[12]], 1.0f));
	const sunder_v3_t v13 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[13]], 1.0f));
	const sunder_v3_t v14 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[14]], 1.0f));
	const sunder_v3_t v15 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[15]], 1.0f));

	const sunder_v3_t v16 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[16]], 1.0f));
	const sunder_v3_t v17 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[17]], 1.0f));
	const sunder_v3_t v18 = sunder_v3_v4((*data->capsule_model) * sunder_v4_v3(data->capsule_local_space_vertex_position_buffer[data->lower_clipping_plane_vertex_index_buffer[18]], 1.0f));
	*/

	//const f32 capsule_z = data->capsule_position.z;

	//for (u32 v = 1; v < 10 + 1; v++)
	//{
	//	data->segments->v0v17[v - 1] = lightray_compute_segment_point(v0, v17, v, 11.0f, capsule_z, SUNDER_TRUE);
	//	data->segments->v8v15[v - 1] = lightray_compute_segment_point(v8, v15, v, 11.0f, capsule_z, SUNDER_TRUE);

	//	data->segments->v0v8[v - 1] = lightray_compute_segment_point(v0, v8, v, 11.0f, capsule_z, SUNDER_TRUE);
	//	data->segments->v17v15[v - 1] = lightray_compute_segment_point(v17, v15, v, 11.0f, capsule_z, SUNDER_TRUE);

	//	data->segments->v0v1[v - 1] = lightray_compute_segment_point(v0, v1, v, 11.0f, capsule_z, SUNDER_TRUE);
	//	data->segments->v1v17[v - 1] = lightray_compute_segment_point(v17, v1, v, 11.0f, capsule_z, SUNDER_TRUE);
	//	data->segments->v8v10[v - 1] = lightray_compute_segment_point(v8, v10, v, 11.0f, capsule_z, SUNDER_TRUE);
	//	data->segments->v10v15[v - 1] = lightray_compute_segment_point(v15, v10, v, 11.0f, capsule_z, SUNDER_TRUE);
	//}

	/*
	data->ray_cluster_origin_buffer[0] = sunder_v3(v0.x, v0.y, capsule_z);
	data->ray_cluster_origin_buffer[1] = sunder_v3(v1.x, v1.y, capsule_z);
	data->ray_cluster_origin_buffer[2] = sunder_v3(v2.x, v2.y, capsule_z);
	data->ray_cluster_origin_buffer[3] = sunder_v3(v3.x, v3.y, capsule_z);
	data->ray_cluster_origin_buffer[4] = sunder_v3(v4.x, v4.y, capsule_z);
	data->ray_cluster_origin_buffer[5] = sunder_v3(v5.x, v5.y, capsule_z);
	data->ray_cluster_origin_buffer[6] = sunder_v3(v6.x, v6.y, capsule_z);
	data->ray_cluster_origin_buffer[7] = sunder_v3(v7.x, v7.y, capsule_z);
	data->ray_cluster_origin_buffer[8] = sunder_v3(v8.x, v8.y, capsule_z);
	data->ray_cluster_origin_buffer[9] = sunder_v3(v9.x, v9.y, capsule_z);
	data->ray_cluster_origin_buffer[10] = sunder_v3(v10.x, v10.y, capsule_z);
	data->ray_cluster_origin_buffer[11] = sunder_v3(v11.x, v11.y, capsule_z);
	data->ray_cluster_origin_buffer[12] = sunder_v3(v12.x, v12.y, capsule_z);
	data->ray_cluster_origin_buffer[13] = sunder_v3(v13.x, v13.y, capsule_z);
	data->ray_cluster_origin_buffer[14] = sunder_v3(v14.x, v14.y, capsule_z);
	data->ray_cluster_origin_buffer[15] = sunder_v3(v15.x, v15.y, capsule_z);
	data->ray_cluster_origin_buffer[16] = sunder_v3(v16.x, v16.y, capsule_z);
	data->ray_cluster_origin_buffer[17] = sunder_v3(v17.x, v17.y, capsule_z);
	data->ray_cluster_origin_buffer[18] = sunder_v3(v18.x, v18.y, capsule_z);

	u32 ray_cluster_origin_buffer_iter = 19;
	*/

	/*

	for (u32 v = 0; v < 10; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v0v17[v];
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v8v15[v];
		ray_cluster_origin_buffer_iter++;


		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v0v8[v];
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v17v15[v];
		ray_cluster_origin_buffer_iter++;


		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v0v1[v];
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v1v17[v];
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v8v10[v];
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = data->segments->v10v15[v];
		ray_cluster_origin_buffer_iter++;
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	for (u32 v = 0; v < 10; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[0], data->segments->v1v17[0], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[0], data->segments->v10v15[0], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 9; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[1], data->segments->v1v17[1], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[1], data->segments->v10v15[1], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 8; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[2], data->segments->v1v17[2], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[2], data->segments->v10v15[2], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 7; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[3], data->segments->v1v17[3], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[3], data->segments->v10v15[3], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 6; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[4], data->segments->v1v17[4], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[4], data->segments->v10v15[4], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 5; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[5], data->segments->v1v17[5], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[5], data->segments->v10v15[5], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 4; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[6], data->segments->v1v17[6], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[6], data->segments->v10v15[6], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 3; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[7], data->segments->v1v17[7], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[7], data->segments->v10v15[7], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 2; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[8], data->segments->v1v17[8], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[8], data->segments->v10v15[8], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 1; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v0v1[9], data->segments->v1v17[9], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;

		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[9], data->segments->v10v15[9], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}
	*/

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*

	for (u32 v = 0; v < 10; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[0], data->segments->v10v15[0], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 9; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[1], data->segments->v10v15[1], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 8; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[2], data->segments->v10v15[2], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 7; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[3], data->segments->v10v15[3], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 6; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[4], data->segments->v10v15[4], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 5; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[5], data->segments->v10v15[5], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 4; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[6], data->segments->v10v15[6], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 3; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[7], data->segments->v10v15[7], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 2; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[8], data->segments->v10v15[8], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}

	for (u32 v = 0; v < 1; v++)
	{
		data->ray_cluster_origin_buffer[ray_cluster_origin_buffer_iter] = lightray_compute_segment_point(data->segments->v8v10[9], data->segments->v10v15[9], v + 1, 11.0f, capsule_z, SUNDER_TRUE);
		ray_cluster_origin_buffer_iter++;
	}
	*/

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	lightray_cast_ray_cluster(data->capsule_vertex_count, data->capsule_local_space_vertex_position_buffer, *data->capsule_model, *data->capsule_inverse, data->ray_capsule_intersection_point_buffer, data->plane_vertex_count, data->plane_local_space_vertex_position_buffer, *data->plane_model, *data->plane_inverse, data->ray_plane_intersection_point_buffer, ray_cluster_origin_buffer_iter, data->ray_cluster_origin_buffer, data->cluster_direction, data->capsule_hit_bitmask_buffer, data->plane_hit_bitmask_buffer);

	u32 cluster_hit_count = 0;
	
	// merge that into ray cluster cast function
	for (u32 i = 0; i < ray_cluster_origin_buffer_iter; i++)
	{
		if (SUNDER_IS_BUFFERED_BIT_SET(data->capsule_hit_bitmask_buffer, i) && SUNDER_IS_BUFFERED_BIT_SET(data->plane_hit_bitmask_buffer, i))
		{
			data->hit_mapping_buffer[cluster_hit_count] = i;
			cluster_hit_count++;
		}
	}

	//SUNDER_LOG("\n");
	//SUNDER_LOG(cluster_hit_count);

	u16 closest_ray_index = 0;
	f32 closest_squared_distance = FLT_MAX;

	for (u32 i = 0; i < cluster_hit_count; i++)
	{
		const u16 mi = data->hit_mapping_buffer[i];

		const f32 squared_distance = sunder_squared_distance_v3(data->ray_capsule_intersection_point_buffer[mi], data->ray_plane_intersection_point_buffer[mi]);

		if (squared_distance < closest_squared_distance)
		{
			closest_squared_distance = squared_distance;
			closest_ray_index = i;
		}
	}

	const u16 mcri = data->hit_mapping_buffer[closest_ray_index];

	const f32 plane_projection_distance = sunder_distance_v3(data->ray_plane_intersection_point_buffer[mcri], data->ray_capsule_intersection_point_buffer[mcri]) - 0.00001f; // wacky epsilon

	return plane_projection_distance;

	*/

	return 0.0f;
}

sunder_v3_t lightray_glm_vec3_to_sunder(const glm::vec3& v)
{
	return sunder_v3(v.x, v.y, v.z);
}

void lightray_advance_animation_playback_data(lightray_animation_core_t* core, u32 animation_playback_command_index, f32 delta_time, lightray_animation_playback_data_advance_kind_e advance_kind)
{
	const u16 animation_index = core->playback_command_buffer[animation_playback_command_index].animation_index;
	const f32 animation_duration = core->animation_buffer[animation_index].duration;
	const f32 animation_tickrate = core->animation_buffer[animation_index].tickrate;
	const f32 scale = core->playback_command_buffer[animation_playback_command_index].scale;

	if (advance_kind == LIGHTRAY_ANIMATION_PLAYBACK_DATA_ADVANCE_KIND_LOOPED)
	{
		core->playback_command_buffer[animation_playback_command_index].time += delta_time * scale;
		const f32 time_in_ticks = core->playback_command_buffer[animation_playback_command_index].time * animation_tickrate;
		const f32 animation_ticks = SUNDER_CAST2(f32)fmod(time_in_ticks, animation_duration);
		core->playback_command_buffer[animation_playback_command_index].ticks = animation_ticks;
	}

	else if (advance_kind == LIGHTRAY_ANIMATION_PLAYBACK_DATA_ADVANCE_KIND_FULL)
	{
		core->playback_command_buffer[animation_playback_command_index].time += delta_time * scale;
		const f32 time_in_ticks = core->playback_command_buffer[animation_playback_command_index].time * animation_tickrate;
		core->playback_command_buffer[animation_playback_command_index].ticks = time_in_ticks;
	}
}

f32 lightray_compute_animation_blend_step(f32 seconds)
{
	return 1.0f / seconds;
}

u32 lightray_push_overlap_attribute(lightray_scene_t* scene, u32 entity_index, u32 flags)
{
	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return UINT32_MAX;
	}

	const u32 index = scene->overlap_attribute_core.overlap_attribute_buffer.size;

	if (!sunder_valid_index(index, scene->overlap_attribute_core.overlap_attribute_buffer.capacity)) 
	{
		return UINT32_MAX;
	}

	scene->entity_buffer[entity_index].overlap_attribute_index = index;
	scene->overlap_attribute_core.overlap_attribute_buffer[index].entity_index = entity_index;
	scene->overlap_attribute_core.overlap_attribute_buffer[index].grid_cell_index_buffer_offset = scene->overlap_attribute_core.overlap_grid_cell_row_index_buffer.count_per * index;
	scene->overlap_attribute_core.overlap_attribute_buffer[index].overlap_layer_index_buffer_offset = 64 * index;
	scene->overlap_attribute_core.overlap_attribute_buffer[index].has_already_overlapped_with_bitmask_buffer_offset = scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer.count_per * index;
	scene->overlap_attribute_core.overlap_attribute_buffer[index].overlap_attribute_index_buffer_offset = scene->overlap_attribute_core.overlap_attribute_index_buffer.count_per * index;
	scene->overlap_attribute_core.overlap_attribute_buffer[index].flags = flags;

	scene->overlap_attribute_core.overlap_attribute_buffer.size++;

	return index;
}

u32 lightray_push_overlap_mesh_batch(lightray_scene_t* scene)
{
	const u32 index = scene->overlap_attribute_core.overlap_mesh_batch_buffer.size;

	if (!sunder_valid_index(index, scene->overlap_attribute_core.overlap_mesh_batch_buffer.capacity))
	{
		return UINT32_MAX;
	}

	scene->overlap_attribute_core.overlap_mesh_batch_buffer[index].overlap_mesh_buffer_offset = index * scene->overlap_attribute_core.overlap_mesh_batch_buffer.count_per;
	scene->overlap_attribute_core.overlap_mesh_batch_buffer[index].overlap_mesh_count = 0;

	scene->overlap_attribute_core.overlap_mesh_batch_buffer.size++;

	return index;
}

void lightray_bind_overlap_mesh_batch(lightray_scene_t* scene, u32 overlap_mesh_batch_index, u32 overlap_attribute_index)
{
	if (!sunder_valid_index(overlap_mesh_batch_index, scene->overlap_attribute_core.overlap_mesh_batch_buffer.size))
	{
		return;
	}

	if (!sunder_valid_index(overlap_attribute_index, scene->overlap_attribute_core.overlap_attribute_buffer.size))
	{
		return;
	}

	scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_mesh_batch_index = overlap_mesh_batch_index;
	const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_count;
	const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_buffer_offset;
	const u32 overlap_attribute_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].entity_index;

	for (u32 i = 0; i < overlap_mesh_count; i++)
	{
		const u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + i].entity_index;
		lightray_bind_entity(scene, overlap_mesh_entity_index, overlap_attribute_entity_index);
	}
}

u32 lightray_push_overlap_mesh(lightray_scene_t* scene, u32 entity_index, u32 overlap_mesh_batch_index, u32 mesh_index_count, u32 mesh_index_buffer_offset, const lightray_model_t* model, u32 flags)
{
	if (!sunder_valid_index(overlap_mesh_batch_index, scene->overlap_attribute_core.overlap_mesh_batch_buffer.size))
	{
		return UINT32_MAX;
	}

	const u32 index = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_count;

	if (!sunder_valid_index(index, scene->overlap_attribute_core.overlap_mesh_batch_buffer.count_per))
	{
		return UINT32_MAX;
	}

	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return UINT32_MAX;
	}

	const u32 overlap_mesh_buffer_index = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_buffer_offset + index;
	
	scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_index].entity_index = entity_index;
	scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_index].index_buffer_offset = mesh_index_buffer_offset;
	scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_index].index_count = mesh_index_count;
	scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_index].model = model;
	scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_index].obb_index = UINT16_MAX;
	scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_index].obb_owner_index = UINT16_MAX;

	if (SUNDER_IS_ANY_BIT_SET(flags, LIGHTRAY_OVERLAP_MESH_BITS_IS_OBB_BIT, 1u))
	{
		SUNDER_SET_BIT(scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_index].flags, LIGHTRAY_OVERLAP_MESH_BITS_IS_OBB_BIT, 1u);
	}

	scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_count++;
	scene->overlap_attribute_core.overlap_mesh_buffer.size++;

	return index;
}

void lightray_bind_overlap_obb(lightray_scene_t* scene, u32 overlap_mesh_batch_index, u16 obb_overlap_mesh_index, u16 target_overlap_mesh_index)
{
	if (!sunder_valid_index(overlap_mesh_batch_index, scene->overlap_attribute_core.overlap_mesh_batch_buffer.size))
	{
		return;
	}

	if(!sunder_valid_index(obb_overlap_mesh_index, scene->overlap_attribute_core.overlap_mesh_batch_buffer.count_per))
	{
		return;
	}

	if (obb_overlap_mesh_index == target_overlap_mesh_index)
	{
		return;
	}

	if (!sunder_valid_index(target_overlap_mesh_index, scene->overlap_attribute_core.overlap_mesh_batch_buffer.count_per))
	{
		return;
	}

	const u16 target_index = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_buffer_offset + target_overlap_mesh_index;
	const u16 obb_index = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_buffer_offset + obb_overlap_mesh_index;

	scene->overlap_attribute_core.overlap_mesh_buffer[target_index].obb_index = obb_overlap_mesh_index;
	scene->overlap_attribute_core.overlap_mesh_buffer[obb_index].obb_owner_index = target_overlap_mesh_index;
	SUNDER_ZERO_BIT(scene->overlap_attribute_core.overlap_mesh_buffer[target_index].flags, LIGHTRAY_OVERLAP_MESH_BITS_IS_OBB_BIT, 1u);
	SUNDER_SET_BIT(scene->overlap_attribute_core.overlap_mesh_buffer[obb_index].flags, LIGHTRAY_OVERLAP_MESH_BITS_IS_OBB_BIT, 1u);
}

void lightray_bind_overlap_aabb(lightray_scene_t* scene, u32 parent_entity_index, u32 aabb_entity_index)
{
	if (!sunder_valid_index(parent_entity_index, scene->current_entity_count))
	{
		return;
	}

	if (!sunder_valid_index(aabb_entity_index, scene->current_entity_count))
	{
		return;
	}

	const u32 overlap_attribute_index = scene->entity_buffer[parent_entity_index].overlap_attribute_index;

	if (!sunder_valid_index(overlap_attribute_index, scene->overlap_attribute_core.overlap_attribute_buffer.size))
	{
		return;
	}

	scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].aabb_index = aabb_entity_index;
	lightray_bind_entity(scene, aabb_entity_index, parent_entity_index);
}

b32 lightray_ray_mesh_intersect(const lightray_ray_t* transformed_ray, f32* t_out, lightray_ray_triangle_face_culling_mode_e culling_mode, sunder_v3_t* tri_out, u32 index_count, const u32* sentinel_index_buffer, u32 sentinel_index_buffer_offset, const sunder_v3_t* sentinel_local_space_vertex_position_buffer, u32 sentinel_local_space_vertex_position_buffer_offset)
{
	b32 mesh_hit = SUNDER_FALSE;

	f32 u_out{};
	f32 v_out{};

	for (u32 si = 0; si < index_count; si += 3)
	{
		const u32 v0_index = sentinel_index_buffer[sentinel_index_buffer_offset + si + 0];
		const u32 v1_index = sentinel_index_buffer[sentinel_index_buffer_offset + si + 1];
		const u32 v2_index = sentinel_index_buffer[sentinel_index_buffer_offset + si + 2];

		sunder_v3_t tri[3]{};
		tri[0] = sentinel_local_space_vertex_position_buffer[sentinel_local_space_vertex_position_buffer_offset + v0_index];
		tri[1] = sentinel_local_space_vertex_position_buffer[sentinel_local_space_vertex_position_buffer_offset + v1_index];
		tri[2] = sentinel_local_space_vertex_position_buffer[sentinel_local_space_vertex_position_buffer_offset + v2_index];

		const b32 tri_hit = lightray_ray_triangle_intersect(transformed_ray, tri, t_out, &u_out, &v_out, culling_mode, &tri_out[0], &tri_out[1], &tri_out[2]);

		if (tri_hit)
		{
			mesh_hit = SUNDER_TRUE;
			break;
		}
	}

	return mesh_hit;
}

lightray_ray_t lightray_transform_ray_to_ray_space(const lightray_ray_t* ray, lightray_buffer_m4_cache_t* inverse_cache, sunder_bit_buffer_t* inverse_cache_bit_buffer, const sunder_m4_t& model, const sunder_v3_t& scale, u32 mesh_global_index, b32 non_unit_scale, b32 non_default_rotation, b32 attribute_non_default_rotation)
{
	lightray_ray_t result{};
	result.distance = ray->distance;

	const sunder_v3_t mesh_translation = sunder_extract_translation_m4(model);

	if (!attribute_non_default_rotation)
	{
		if (non_unit_scale)
		{
			result.origin = sunder_inverse_position_ts_v3(ray->origin, mesh_translation, scale);
			result.direction = sunder_inverse_direction_hacky_v3(ray->direction, scale);
		}

		else if (!non_default_rotation)
		{
			result.origin = sunder_inverse_position_t_v3(ray->origin, mesh_translation);
			result.direction = ray->direction;
		}

		else
		{
			sunder_m4_t inverse{};

			if ((*inverse_cache_bit_buffer) == mesh_global_index)
			{
				const lightray_m4_cache_t ic = (*inverse_cache)[mesh_global_index];
				inverse = lightray_reconstruct_cached_m4(&ic);
			}

			else
			{
				inverse = sunder_m4_inverse(model);
				lightray_cache_m4(inverse_cache, inverse_cache_bit_buffer, inverse, mesh_global_index);
			}

			result.origin = sunder_transform_position_to_ray_space_v3(ray->origin, inverse);
			result.direction = sunder_transform_direction_to_ray_space_v3(ray->direction, inverse);
		}
	}

	else
	{
		sunder_m4_t inverse{};

		if ((*inverse_cache_bit_buffer) == mesh_global_index)
		{
			//SUNDER_LOG("\nALREADY CACHED");

			const lightray_m4_cache_t ic = (*inverse_cache)[mesh_global_index];
			inverse = lightray_reconstruct_cached_m4(&ic);
		}

		else
		{
			//SUNDER_LOG("\nCOMPUTING INVERSE");

			inverse = sunder_m4_inverse(model);
			lightray_cache_m4(inverse_cache, inverse_cache_bit_buffer, inverse, mesh_global_index);
		}

		result.origin = sunder_transform_position_to_ray_space_v3(ray->origin, inverse);
		result.direction = sunder_transform_direction_to_ray_space_v3(ray->direction, inverse);
	}

	return result;
}

void lightray_compute_ray_triangle_intersection_metadata(const lightray_ray_t* transformed_ray, const lightray_ray_t* ray, f32 t, const sunder_m4_t& model, const sunder_v3_t& scale, sunder_v3_t* tri, sunder_v3_t* intersection_point, sunder_v3_t* triangle_normal, f32* squared_distance, b32 non_unit_scale, b32 non_default_rotation, b32 attribute_non_default_rotation)
{
	const sunder_v3_t translation = sunder_extract_translation_m4(model);
	sunder_v3_t intersection_point_internal = transformed_ray->origin + transformed_ray->direction * t;

	if (!attribute_non_default_rotation)
	{
		if (non_unit_scale)
		{
			intersection_point_internal = sunder_v3_world_space_ts(intersection_point_internal, translation, scale);

			tri[0] = sunder_v3_world_space_ts(tri[0], translation, scale);
			tri[1] = sunder_v3_world_space_ts(tri[1], translation, scale);
			tri[2] = sunder_v3_world_space_ts(tri[2], translation, scale);
		}

		else if (!non_default_rotation)
		{
			intersection_point_internal = sunder_v3_world_space_t(intersection_point_internal, translation);

			tri[0] = sunder_v3_world_space_t(tri[0], translation);
			tri[1] = sunder_v3_world_space_t(tri[1], translation);
			tri[2] = sunder_v3_world_space_t(tri[2], translation);
		}

		else
		{
			intersection_point_internal = sunder_v3_v4(model * sunder_v4_v3(intersection_point_internal, 1.0f));

			tri[0] = sunder_v3_v4(model * sunder_v4_v3(tri[0], 1.0f));
			tri[1] = sunder_v3_v4(model * sunder_v4_v3(tri[1], 1.0f));
			tri[2] = sunder_v3_v4(model * sunder_v4_v3(tri[2], 1.0f));
		}
	}

	else
	{
		intersection_point_internal = sunder_v3_v4(model * sunder_v4_v3(intersection_point_internal, 1.0f));

		tri[0] = sunder_v3_v4(model * sunder_v4_v3(tri[0], 1.0f));
		tri[1] = sunder_v3_v4(model * sunder_v4_v3(tri[1], 1.0f));
		tri[2] = sunder_v3_v4(model * sunder_v4_v3(tri[2], 1.0f));
	}

	*squared_distance = sunder_squared_distance_v3(intersection_point_internal, ray->origin);

	const sunder_v3_t AB = tri[1] - tri[0];
	const sunder_v3_t AC = tri[2] - tri[0];
	*triangle_normal = sunder_normalize_v3(sunder_cross_v3(AB, AC));

	*intersection_point = intersection_point_internal;
}

void lightray_write_hit_attribute(lightray_scene_t* scene, const lightray_raycast_memory_reserve_result_t* memory_reserve_result, f32 squared_distance, u32 attribute_index, const sunder_v3_t& intersection_point, const sunder_v3_t& triangle_normal, u32* written_attribute_count, f32* closest_squared_distance, u32* closest_attribute_index, sunder_v3_t* closest_intersection_point, sunder_v3_t* closest_triangle_normal)
{
	if (squared_distance < (*closest_squared_distance))
	{
		*closest_squared_distance = squared_distance;
		*closest_attribute_index = attribute_index;
		*closest_intersection_point = intersection_point;
		*closest_triangle_normal = triangle_normal;
	}

	if (sunder_valid_index(*written_attribute_count, memory_reserve_result->pierce_layer_test_data_per_ray_count))
	{
		scene->raycast_core.pierce_layer_test_data_subarena[memory_reserve_result->pierce_layer_test_data_subarena_offset + (*written_attribute_count)].attribute_index = attribute_index;
		scene->raycast_core.pierce_layer_test_data_subarena[memory_reserve_result->pierce_layer_test_data_subarena_offset + (*written_attribute_count)].intersection_point = intersection_point;
		scene->raycast_core.pierce_layer_test_data_subarena[memory_reserve_result->pierce_layer_test_data_subarena_offset + (*written_attribute_count)].triangle_normal = triangle_normal;
		scene->raycast_core.pierce_layer_test_data_subarena[memory_reserve_result->pierce_layer_test_data_subarena_offset + (*written_attribute_count)].squared_distance = squared_distance;

		(*written_attribute_count)++;
	}
}

void lightray_add_overlap_layer(lightray_scene_t* scene, u32 overlap_attribute_index, u32 layer)
{
	if (layer > 63)
	{
		return;
	}

	SUNDER_SET_BIT(scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_bitmask, layer, 1ull);
	const u32 overlap_layer_index_buffer_offset = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_index_buffer_offset;
	const u32 overlap_layer_count = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_count;
	scene->overlap_attribute_core.overlap_layer_index_buffer[overlap_layer_index_buffer_offset + overlap_layer_count] = layer;
	scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_count++;
}

void lightray_set_overlap_attribute_flags(lightray_scene_t* scene, u32 overlap_attribute_index, u32 flags)
{
	if (SUNDER_IS_ANY_BIT_SET(flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_OVERLAPPED_BIT, 1u))
	{
		SUNDER_SET_BIT(scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_OVERLAPPED_BIT, 1u);
	}

	if(SUNDER_IS_ANY_BIT_SET(flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT, 1u))
	{
		SUNDER_SET_BIT(scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT, 1u);
	}
}

void lightray_enable_overlap_mesh_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 overlap_attribute_index, u32 overlap_mesh_index)
{
	SUNDER_SET_BIT(scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].should_reproject_vertices_bitmask, overlap_mesh_index, 1u);
}

b32 lightray_verify_overlap_mesh(lightray_scene_t* scene, u32 overlap_mesh_flags, u32 overlap_mesh_buffer_offset, u32 overlap_mesh_index, u16 bitmask, u16* chosen_index, b32* is_raw_mesh)
{
	const b32 check_first_pass = SUNDER_IS_ANY_BIT_SET(bitmask, overlap_mesh_index, 1);

	if (check_first_pass)
	{
		SUNDER_FALSE;
	}

	const b32 is_obb = SUNDER_IS_ANY_BIT_SET(overlap_mesh_flags, LIGHTRAY_OVERLAP_MESH_BITS_IS_OBB_BIT, 1u);

	if (!is_obb)
	{
		const u16 obb_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + overlap_mesh_index].obb_index;

		if (obb_index != UINT16_MAX)
		{
			*chosen_index = obb_index;
		}

		else
		{
			*is_raw_mesh = SUNDER_TRUE;
			*chosen_index = overlap_mesh_index;
		}
	}

	else
	{
		*chosen_index = overlap_mesh_index;
	}

	const b32 check_second_pass = SUNDER_IS_ANY_BIT_SET(bitmask, (*chosen_index), 1);

	if (check_second_pass)
	{
		return SUNDER_FALSE;
	}

	return SUNDER_TRUE;
}

void lightray_handle_overlap_mesh_vertex_projection(lightray_scene_t* scene, const sunder_m4_t& overlap_attribute_entity_transform, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, u32 overlap_mesh_global_index, b32 overlap_attribute_non_default_rotation)
{

	const u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].entity_index;
	const u32 overlap_mesh_entity_flags = scene->entity_buffer[overlap_mesh_entity_index].flags;
	const b32 non_unit_scale = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
	const b32 non_default_rotation = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
	const sunder_v3_t overlap_attribute_translation = sunder_extract_translation_m4(overlap_attribute_entity_transform);

	if (!overlap_attribute_non_default_rotation)
	{
		if (non_unit_scale)
		{
			const u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].entity_index;
			const sunder_v3_t overlap_mesh_scale = scene->scale_buffer[overlap_mesh_entity_index];
			const sunder_v3_t overlap_mesh_translation = overlap_attribute_translation + scene->position_buffer[overlap_mesh_entity_index];
			lightray_compute_sentinel_world_space_vertex_positions_ts(scene->sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, overlap_mesh_translation, overlap_mesh_scale);
		}

		else if (!non_default_rotation)
		{
			const u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].entity_index;
			const sunder_v3_t overlap_mesh_translation = overlap_attribute_translation + scene->position_buffer[overlap_mesh_entity_index];
			lightray_compute_sentinel_world_space_vertex_positions_t(scene->sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, overlap_mesh_translation);
		}

		else
		{
			const u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].entity_index;
			const sunder_v3_t overlap_mesh_translation = overlap_attribute_translation + scene->position_buffer[overlap_mesh_entity_index];
			const sunder_m4_t overlap_mesh_transform = sunder_m4_model(overlap_mesh_translation, scene->quat_rotation_buffer[overlap_mesh_entity_index], scene->scale_buffer[overlap_mesh_entity_index]);
			lightray_compute_sentinel_world_space_vertex_positions(scene->sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, overlap_mesh_transform);
		}
	}

	else
	{
		const u32 sentinel_local_space_vertex_position_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].sentinel_local_space_vertex_position_buffer_offset;
		const u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].entity_index;
		sunder_m4_t overlap_mesh_transform = sunder_m4_model(scene->position_buffer[overlap_mesh_entity_index], scene->quat_rotation_buffer[overlap_mesh_entity_index], scene->scale_buffer[overlap_mesh_entity_index]);
		overlap_mesh_transform = overlap_attribute_entity_transform * overlap_mesh_transform;
		lightray_compute_sentinel_world_space_vertex_positions(scene->sentinel_local_space_vertex_position_buffer + sentinel_local_space_vertex_position_buffer_offset, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, overlap_mesh_transform);
	}
}

void lightray_handle_collision_mesh_vertex_projection(lightray_scene_t* scene, const sunder_m4_t& collision_attribute_entity_transform, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, u32 collision_mesh_global_index, b32 collision_attribute_non_default_rotation)
{
	const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_global_index].entity_index;
	const u32 collision_mesh_entity_flags = scene->entity_buffer[collision_mesh_entity_index].flags;
	const b32 non_unit_scale = SUNDER_IS_ANY_BIT_SET(collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
	const b32 non_default_rotation = SUNDER_IS_ANY_BIT_SET(collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
	const sunder_v3_t collision_attribute_translation = sunder_extract_translation_m4(collision_attribute_entity_transform);

	if (!collision_attribute_non_default_rotation)
	{
		if (non_unit_scale)
		{
			const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_global_index].entity_index;
			const sunder_v3_t collision_mesh_scale = scene->scale_buffer[collision_mesh_entity_index];
			const sunder_v3_t collision_mesh_translation = collision_attribute_translation + scene->position_buffer[collision_mesh_entity_index];
			lightray_compute_sentinel_world_space_vertex_positions_ts(scene->sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, collision_mesh_translation, collision_mesh_scale);
		}

		else if (!non_default_rotation)
		{
			const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_global_index].entity_index;
			const sunder_v3_t collision_mesh_translation = collision_attribute_translation + scene->position_buffer[collision_mesh_entity_index];
			lightray_compute_sentinel_world_space_vertex_positions_t(scene->sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, collision_mesh_translation);
		}

		else
		{
			const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_global_index].entity_index;
			const sunder_v3_t collision_mesh_translation = collision_attribute_translation + scene->position_buffer[collision_mesh_entity_index];
			const sunder_m4_t collision_mesh_transform = sunder_m4_model(collision_mesh_translation, scene->quat_rotation_buffer[collision_mesh_entity_index], scene->scale_buffer[collision_mesh_entity_index]);
			lightray_compute_sentinel_world_space_vertex_positions(scene->sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, collision_mesh_transform);
		}
	}

	else
	{
		const u32 sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_global_index].sentinel_local_space_vertex_position_buffer_offset;
		const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_global_index].entity_index;
		sunder_m4_t collision_mesh_transform = sunder_m4_model(scene->position_buffer[collision_mesh_entity_index], scene->quat_rotation_buffer[collision_mesh_entity_index], scene->scale_buffer[collision_mesh_entity_index]);
		collision_mesh_transform = collision_attribute_entity_transform * collision_mesh_transform;
		lightray_compute_sentinel_world_space_vertex_positions(scene->sentinel_local_space_vertex_position_buffer + sentinel_local_space_vertex_position_buffer_offset, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, collision_mesh_transform);
	}
}

b32 lightray_cast_convex_hull(lightray_scene_t* scene, lightray_grid_t* grid, const lightray_render_instance_t* render_instance_buffer, u32 overlap_attribute_index)
{
	const u32 overlap_attribute_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].entity_index;
	const u32 overlap_attribute_entity_flags = scene->entity_buffer[overlap_attribute_entity_index].flags;

	if (SUNDER_IS_ANY_BIT_SET(overlap_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_BOUND_BIT, 1u))
	{
		return SUNDER_FALSE;
	}

	if (SUNDER_IS_ANY_BIT_SET(scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_STATIC_BIT, 1u))
	{
		return SUNDER_FALSE;
	}

	b32 hit_any = SUNDER_FALSE;

	const u32 has_already_overlapped_buffer_offset = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].has_already_overlapped_with_bitmask_buffer_offset;
	const u32 has_already_failed_to_overlap_buffer_offset = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].has_already_failed_to_overlap_bitmask_buffer_offset;

	const u32 has_already_overlapped_count_per = scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer.count_per;
	const u32 has_already_failed_to_overlap_count_per = scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer.count_per;

	scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].row_count = 0;
	scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].width_per_row = 0;

	for (u32 i = 0; i < has_already_overlapped_count_per; i++)
	{
		scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer.buffer[has_already_overlapped_buffer_offset + i] = 0;
	}

	for (u32 i = 0; i < has_already_failed_to_overlap_count_per; i++)
	{
		scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer.buffer[has_already_failed_to_overlap_buffer_offset + i] = 0;
	}

	const u32 aabb_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].aabb_index;
	
	const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];
	const sunder_v3_t aabb_translation = scene->position_buffer[aabb_entity_index];

	const b32 overlap_attribute_non_default_rotation = SUNDER_IS_ANY_BIT_SET(overlap_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
	sunder_m4_t overlap_attribute_entity_transform{};

	if (overlap_attribute_non_default_rotation)
	{
		overlap_attribute_entity_transform = sunder_m4_model(scene->position_buffer[overlap_attribute_entity_index], scene->quat_rotation_buffer[overlap_attribute_entity_index], scene->scale_buffer[overlap_attribute_entity_index]);
	}

	const sunder_v3_t overlap_attribute_translation = scene->position_buffer[overlap_attribute_entity_index];

	const sunder_v3_t aabb_new_translation = aabb_translation + overlap_attribute_translation;

	lightray_grid_cell_aabb_coordinates_t row_coordinates{};
	lightray_grid_cell_aabb_coordinates_t column_coordinates{};

	lightray_get_grid_cell_coordinates_aabb(grid, &aabb_new_translation, &aabb_scale, &row_coordinates, &column_coordinates);

	if (row_coordinates.min < 0 || row_coordinates.max < 0 || column_coordinates.min < 0 || column_coordinates.max < 0)
	{
		return SUNDER_FALSE;
	}

	const u32 overlap_mesh_batch_index = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_mesh_batch_index;
	const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_count;
	const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[overlap_mesh_batch_index].overlap_mesh_buffer_offset;

	const u32 grid_cell_row_index_buffer_offset = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].grid_cell_index_buffer_offset;
	const u32 aabb_width_per_row = column_coordinates.max - column_coordinates.min + 1;

	for (i32 ri = row_coordinates.min; ri < row_coordinates.max + 1; ri++)
	{
		const u32 current_row_index = lightray_get_grid_cell_index(grid, ri, column_coordinates.min);
		const u32 current_row_count = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].row_count;

		scene->overlap_attribute_core.overlap_grid_cell_row_index_buffer[grid_cell_row_index_buffer_offset + current_row_count] = current_row_index;
		scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].row_count++;
	}

	scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].width_per_row = aabb_width_per_row;
	const u32 row_count = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].row_count;

	const u64 overlap_layer_bitmask = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_bitmask;
	const u64 cull_overlap_layer_bitmask = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].cull_overlap_layer_bitmask;
	const u32 overlap_layer_count = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_count;
	const u32 overlap_layer_index_buffer_offset = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_layer_index_buffer_offset;

	for (u32 i = 0; i < row_count; i++)
	{
		const u32 grid_cell_row_index = scene->overlap_attribute_core.overlap_grid_cell_row_index_buffer[grid_cell_row_index_buffer_offset + i];

		for (u32 w = 0; w < aabb_width_per_row; w++)
		{
			const u32 grid_cell_index = grid_cell_row_index + w;
			const u32 overlap_attribute_index_buffer_index = grid->cell_buffer[grid_cell_index].overlap_attribute_index_buffer_offset;
			const u32 overlap_attribute_index_count = grid->cell_buffer[grid_cell_index].overlap_attribute_index_count;

			for (u32 ovai = 0; ovai < overlap_attribute_index_count; ovai++)
			{
				const u32 other_overlap_attribute_index = grid->overlap_attribute_index_buffer[overlap_attribute_index_buffer_index + ovai];
				const u32 other_overlap_attribute_flags = scene->overlap_attribute_core.overlap_attribute_buffer[other_overlap_attribute_index].flags;

				if (other_overlap_attribute_index == overlap_attribute_index)
				{
					continue;
				}

				if (!(SUNDER_IS_ANY_BIT_SET(other_overlap_attribute_flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_OVERLAPPED_BIT, 1u)))
				{
					continue;
				}

				if (scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer == (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(has_already_overlapped_buffer_offset) + other_overlap_attribute_index))
				{
					continue;
				}

				if (scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer == (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(has_already_failed_to_overlap_buffer_offset) + other_overlap_attribute_index))
				{
					continue;
				}

				const u64 other_overlap_layer_bitmask = scene->overlap_attribute_core.overlap_attribute_buffer[other_overlap_attribute_index].overlap_layer_bitmask;
				u64 common_overlap_layer_bitmask = 0;

				for (u32 li = 0; li < overlap_layer_count; li++)
				{
					const u32 current_bit = scene->overlap_attribute_core.overlap_layer_index_buffer[overlap_layer_index_buffer_offset + li];

					const b32 self_check = SUNDER_IS_ANY_BIT_SET(overlap_layer_bitmask, current_bit, 1ull);
					const b32 other_check = SUNDER_IS_ANY_BIT_SET(other_overlap_layer_bitmask, current_bit, 1ull);
					const b32 cull_check = SUNDER_IS_ANY_BIT_SET(cull_overlap_layer_bitmask, current_bit, 1ull);

					if (self_check && other_check && !cull_check)
					{
						SUNDER_SET_BIT(common_overlap_layer_bitmask, current_bit, 1ull);
					}
				}

				if (common_overlap_layer_bitmask == 0)
				{
					continue;
				}

				const u32 other_aabb_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[other_overlap_attribute_index].aabb_index;
				const u32 other_aabb_instance_model_binding_index = scene->entity_buffer[other_aabb_entity_index].instance_model_binding_index;
				const sunder_v3_t other_aabb_scale = scene->scale_buffer[other_aabb_entity_index];
				const sunder_v3_t other_aabb_translation = sunder_extract_translation_m4(lightray_copy_glm_mat4_to_sunder(render_instance_buffer[other_aabb_instance_model_binding_index].model.model));

				const b32 aabbs_intersect = lightray_aabbs_intersect(aabb_new_translation, aabb_scale, other_aabb_translation, other_aabb_scale);

				if (!aabbs_intersect)
				{
					scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(has_already_failed_to_overlap_buffer_offset) + other_overlap_attribute_index);

					continue;
				}

				const u32 other_overlap_mesh_batch_index = scene->overlap_attribute_core.overlap_attribute_buffer[other_overlap_attribute_index].overlap_mesh_batch_index;
				const u32 other_overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[other_overlap_mesh_batch_index].overlap_mesh_count;
				const u32 other_overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[other_overlap_mesh_batch_index].overlap_mesh_buffer_offset;

				u16 checked_mesh_bitmask = 0;
				u16 other_checked_mesh_bitmask = 0;

				for (u32 sovm = 0; sovm < overlap_mesh_count; sovm++)
				{
					const u32 overlap_mesh_flags = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + sovm].flags;
					u16 chosen_index = 0;
					b32 is_raw_mesh = SUNDER_FALSE;
					const b32 good = lightray_verify_overlap_mesh(scene, overlap_mesh_flags, overlap_mesh_buffer_offset, sovm, checked_mesh_bitmask, &chosen_index, &is_raw_mesh);

					if (!good)
					{
						continue;
					}

					SUNDER_SET_BIT(checked_mesh_bitmask, chosen_index, 1u);

					for (u32 oovm = 0; oovm < other_overlap_mesh_count; oovm++)
					{
						const u32 other_overlap_mesh_flags = scene->overlap_attribute_core.overlap_mesh_buffer[other_overlap_mesh_buffer_offset + sovm].flags;
						u16 other_chosen_index = 0;
						b32 other_is_raw_mesh = SUNDER_FALSE;
						const b32 good = lightray_verify_overlap_mesh(scene, other_overlap_mesh_flags, other_overlap_mesh_buffer_offset, oovm, other_checked_mesh_bitmask, &other_chosen_index, &other_is_raw_mesh);

						if (!good)
						{
							continue;
						}

						SUNDER_SET_BIT(other_checked_mesh_bitmask, other_chosen_index, 1u);

						const u32 overlap_mesh_global_index = overlap_mesh_buffer_offset + chosen_index;
						u32 sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].sentinel_vertex_count;
						sunder_v3_t* sentinel_world_space_vertex_position_buffer = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_global_index].sentinel_world_space_vertex_position_buffer;

						lightray_handle_overlap_mesh_vertex_projection(scene, overlap_attribute_entity_transform, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, overlap_mesh_global_index, overlap_attribute_non_default_rotation);

						lightray_gjk_support_point_t simplex[4]{};
						u32 simplex_size = 0;

						u32 other_sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[other_overlap_mesh_buffer_offset + other_chosen_index].sentinel_vertex_count;
						sunder_v3_t* other_sentinel_world_space_vertex_position_buffer = scene->overlap_attribute_core.overlap_mesh_buffer[other_overlap_mesh_buffer_offset + other_chosen_index].sentinel_world_space_vertex_position_buffer;

						const b32 obbs_intersect = lightray_gjk_intersect(sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, other_sentinel_world_space_vertex_position_buffer, other_sentinel_vertex_count, simplex, &simplex_size);

						if (!obbs_intersect)
						{
							scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(has_already_failed_to_overlap_buffer_offset) + other_overlap_attribute_index);

							continue;
						}

						if (is_raw_mesh && other_is_raw_mesh)
						{
							scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(has_already_overlapped_buffer_offset) + other_overlap_attribute_index);

							const u32 overlapped_attribute_index_count = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_attribute_index_count;
							const u32 overlapped_attribute_index_buffer_offset = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_attribute_index_buffer_offset;

							if (sunder_valid_index(overlapped_attribute_index_count, scene->overlap_attribute_core.overlap_attribute_index_buffer.count_per))
							{
								scene->overlap_attribute_core.overlap_attribute_index_buffer[overlapped_attribute_index_buffer_offset + overlapped_attribute_index_count] = other_overlap_attribute_index;
								scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_attribute_index_count++;
							}

							hit_any = SUNDER_TRUE;

							goto post_successful_overlap;
						}

						if (!is_raw_mesh)
						{
							chosen_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + chosen_index].obb_owner_index;
							sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + chosen_index].sentinel_vertex_count;
							sentinel_world_space_vertex_position_buffer = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + chosen_index].sentinel_world_space_vertex_position_buffer;

							lightray_handle_overlap_mesh_vertex_projection(scene, overlap_attribute_entity_transform, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, overlap_mesh_buffer_offset + chosen_index, overlap_attribute_non_default_rotation);
						}

						if (!other_is_raw_mesh)
						{
							other_chosen_index = scene->overlap_attribute_core.overlap_mesh_buffer[other_overlap_mesh_buffer_offset + other_chosen_index].obb_owner_index;
							other_sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[other_overlap_mesh_buffer_offset + other_chosen_index].sentinel_vertex_count;
							other_sentinel_world_space_vertex_position_buffer = scene->overlap_attribute_core.overlap_mesh_buffer[other_overlap_mesh_buffer_offset + other_chosen_index].sentinel_world_space_vertex_position_buffer;
						}

						SUNDER_SET_BIT(other_checked_mesh_bitmask, other_chosen_index, 1u);
						SUNDER_SET_BIT(checked_mesh_bitmask, chosen_index, 1u);

						simplex[0].p = sunder_v3_scalar(0.0f);
						simplex[1].p = sunder_v3_scalar(0.0f);
						simplex[2].p = sunder_v3_scalar(0.0f);
						simplex[3].p = sunder_v3_scalar(0.0f);
						simplex_size = 0;

						const b32 meshes_intersect = lightray_gjk_intersect(sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, other_sentinel_world_space_vertex_position_buffer, other_sentinel_vertex_count, simplex, &simplex_size);

						if (!meshes_intersect)
						{
							scene->overlap_attribute_core.has_already_failed_to_overlap_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(has_already_failed_to_overlap_buffer_offset) + other_overlap_attribute_index);

							continue;
						}

						scene->overlap_attribute_core.has_already_overlapped_with_bit_buffer = (SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(has_already_overlapped_buffer_offset) + other_overlap_attribute_index);

						const u32 overlapped_attribute_index_count = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_attribute_index_count;
						const u32 overlapped_attribute_index_buffer_offset = scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_attribute_index_buffer_offset;

						if (sunder_valid_index(overlapped_attribute_index_count, scene->overlap_attribute_core.overlap_attribute_index_buffer.count_per))
						{
							scene->overlap_attribute_core.overlap_attribute_index_buffer[overlapped_attribute_index_buffer_offset + overlapped_attribute_index_count] = other_overlap_attribute_index;
							scene->overlap_attribute_core.overlap_attribute_buffer[overlap_attribute_index].overlap_attribute_index_count++;
						}

						hit_any = SUNDER_TRUE;

						goto post_successful_overlap;
					}
				}

				post_successful_overlap:
				const u8 temp_var = 0; // so that compiler doesnt shit it'self
			}
		}
	}

	return hit_any;
}

void lightray_push_dynamic_collision_attribute_trace_data(lightray_scene_t* scene, u32 collision_attribute_index, const sunder_v3_t& velocity, const sunder_v3_t& movement_this_frame)
{
	const u32 entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].entity_index;
	const sunder_v3_t previous_position = scene->position_buffer[entity_index];
	const u32 trace_data_buffer_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].trace_data_buffer_index;
	scene->collision_attribute_core.trace_data_buffer[trace_data_buffer_index].previous_position = previous_position;
	scene->collision_attribute_core.trace_data_buffer[trace_data_buffer_index].velocity = velocity;
	scene->collision_attribute_core.trace_data_buffer[trace_data_buffer_index].movement_this_frame = movement_this_frame;
}

void lightray_trace_dynamic_collision_attributes(lightray_scene_t* scene, lightray_grid_t* grid)
{
	// do rough snapshot
	// then compute their per axis movement in parallel based on the rough snapshot
	

	// step_index_buffer access = (collision_attribute_index * step_count) + step_index
	// step_position_buffer access = (collision_attribute_index * step_count) + step_index
	// step_grid_cell_index_buffer access (collision_attribute_index * (step_count * grid_cell_index_count)) + (step_index * grid_cell_index_count) + grid_cell_index
	
	const u32 step_count = 4u;
	const u32 axis_count = 3u;
	const u32 grid_cell_index_count = 8u;

	f32 step_factor_table[3]{};
	step_factor_table[0] = 0.5f; // 1 + 1
	step_factor_table[1] = 0.4f; // 2 + 1
	step_factor_table[2] = 0.3f; // 3 + 1

	const u32 collision_attribute_count = scene->collision_attribute_core.trace_data_buffer.size;

	// trace instance snapshot phase
	// single thread only

	for (u32 i = 0; i < collision_attribute_count; i++)
	{
		const u32 collision_attribute_index = scene->collision_attribute_core.trace_data_buffer[i].collision_attribute_index;
		const u32 max_trace_step_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].max_trace_step_count;
		const sunder_v3_t velocity = scene->collision_attribute_core.trace_data_buffer[i].velocity;
		u32 trace_step_count = sunder_clamp_u32(0, max_trace_step_count - 1, ((u32)(sunder_squared_magnitude_v3(velocity) * 0.1f)));

		scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].trace_step_count = trace_step_count + 1;

		const u32 trace_step_buffer_offset = collision_attribute_index * scene->collision_attribute_core.trace_step_buffer.count_per;

		const u32 aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].aabb_index;
		const sunder_v3_t aabb_translation = scene->position_buffer[aabb_entity_index];
		const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];

		if (trace_step_count == 0)
		{
			step_factor_table[0] = 1.0f;
		}

		else
		{
			trace_step_count -= 1;
		}

		const f32 trace_step_factor = step_factor_table[trace_step_count];
		f32 t = trace_step_factor;

		scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].trace_step_factor = trace_step_factor;

		for (u32 sc = 0; sc < trace_step_count + 1; sc++)
		{
			const sunder_v3_t trace_step_position = scene->collision_attribute_core.trace_data_buffer[i].previous_position + (scene->collision_attribute_core.trace_data_buffer[i].movement_this_frame * t);

			scene->collision_attribute_core.trace_step_buffer[trace_step_buffer_offset + sc].position = trace_step_position;

			lightray_grid_cell_aabb_coordinates_t row_coordinates{};
			lightray_grid_cell_aabb_coordinates_t column_coordinates{};
			const sunder_v3_t aabb_translation_step = aabb_translation + trace_step_position;

			lightray_get_grid_cell_coordinates_aabb(grid, &aabb_translation_step, &aabb_scale, &row_coordinates, &column_coordinates);

			const b32 aabb_in_bounds = lightray_aabb_in_grid_bounds(grid, &row_coordinates, &column_coordinates);

			if (!aabb_in_bounds)
			{
				SUNDER_SKIP;
			}

			const u32 aabb_width_per_row = column_coordinates.max - column_coordinates.min + 1;

			for (i32 ri = row_coordinates.min; ri < row_coordinates.max + 1; ri++)
			{
				const u32 current_row_index = lightray_get_grid_cell_index(grid, ri, column_coordinates.min);

				for (u32 w = 0; w < aabb_width_per_row; w++)
				{
					const u32 current_grid_cell_index = current_row_index + w;

					if (grid->cell_buffer[current_grid_cell_index].collision_attribute_written_bit_buffer == collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					const u32 collision_attribute_index_buffer_offset = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_buffer_offset;
					const u32 trace_instance_count = grid->cell_buffer[current_grid_cell_index].trace_instance_count;

					if (!sunder_valid_index(trace_instance_count, grid->trace_instance_buffer.count_per))
					{
						SUNDER_SKIP;
					}

					grid->cell_buffer[current_grid_cell_index].collision_attribute_written_bit_buffer = collision_attribute_index;
					grid->trace_instance_buffer[collision_attribute_index_buffer_offset + trace_instance_count].collision_attribute_index = collision_attribute_index;
					grid->trace_instance_buffer[collision_attribute_index_buffer_offset + trace_instance_count].trace_step_global_index = trace_step_buffer_offset + sc;
					grid->cell_buffer[current_grid_cell_index].trace_instance_count++;

					const u32 trace_step_grid_cell_index_count = scene->collision_attribute_core.trace_step_buffer[trace_step_buffer_offset + sc].grid_cell_index_count;
					const u32 trace_step_grid_cell_buffer_index = ((trace_step_buffer_offset + sc) * scene->collision_attribute_core.trace_step_grid_cell_index_buffer.count_per) + trace_step_grid_cell_index_count;
					scene->collision_attribute_core.trace_step_grid_cell_index_buffer[trace_step_grid_cell_buffer_index] = current_grid_cell_index;
					scene->collision_attribute_core.trace_step_buffer[trace_step_buffer_offset + sc].grid_cell_index_count++;
				}
			}

			t += trace_step_factor;
			t = sunder_clamp_f32(0.0f, 1.0f, t);
		}
	}
	
	// threadable

	for (u32 i = 0; i < collision_attribute_count; i++)
	{
		const u32 collision_attribute_index = scene->collision_attribute_core.trace_data_buffer[i].collision_attribute_index;
		const u32  aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].aabb_index;
		const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];
		const u32 trace_step_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].trace_step_count;
		const u32 has_already_collided_with_bit_buffer_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_collided_with_bitmask_buffer_offset);
		const u32 has_already_failed_to_collide_bit_buffer_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_failed_to_collide_bit_buffer_offset);
		const u32 collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].entity_index;
		const u32 collision_attribute_entity_flags = scene->entity_buffer[collision_attribute_entity_index].flags;
		const b32 collision_attribute_entity_non_default_rotation = SUNDER_IS_ANY_BIT_SET(collision_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
		const u32 bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_mesh_batch_index;
		const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_count;
		const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_buffer_offset;

		const u64 collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask;
		const u64 cull_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].cull_collision_layer_bitmask;
		const u32 collision_layer_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_count;
		const u32 collision_layer_index_buffer_offset = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_index_buffer_offset;

		for (u32 sc = 0; sc < trace_step_count; sc++)
		{
			const u32 trace_step_buffer_index = (collision_attribute_index * scene->collision_attribute_core.trace_step_buffer.count_per) + sc;
			const u32 trace_step_grid_cell_index_count = scene->collision_attribute_core.trace_step_buffer[trace_step_buffer_index].grid_cell_index_count;
			const sunder_v3_t aabb_translation = scene->collision_attribute_core.trace_step_buffer[trace_step_buffer_index].position;
			sunder_m4_t collision_attribute_entity_transform{};

			if (collision_attribute_entity_non_default_rotation)
			{
				collision_attribute_entity_transform = sunder_m4_model(aabb_translation, scene->quat_rotation_buffer[collision_attribute_entity_index], scene->scale_buffer[collision_attribute_entity_index]);
			}

			else
			{
				collision_attribute_entity_transform = sunder_m4_translation(aabb_translation);
			}

			for (u32 gci = 0; gci < trace_step_grid_cell_index_count; gci++)
			{
				const u32 trace_step_grid_cell_buffer_index = ((trace_step_buffer_index) * scene->collision_attribute_core.trace_step_grid_cell_index_buffer.count_per) + gci;
				const u32 current_grid_cell_index = scene->collision_attribute_core.trace_step_grid_cell_index_buffer[trace_step_grid_cell_buffer_index];

				const u32 collision_attribute_index_buffer_offset = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_buffer_offset;
				const u32 trace_step_instance_count = grid->cell_buffer[current_grid_cell_index].trace_instance_count;
				const u32 other_collision_attribute_index_count = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_count;

				// trace instance pass
				for (u32 tsi = 0; tsi < trace_step_instance_count; tsi++)
				{
					const u32 other_collision_attribute_index = grid->trace_instance_buffer[collision_attribute_index_buffer_offset + tsi].collision_attribute_index;

					if (other_collision_attribute_index == collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					if (scene->collision_attribute_core.has_already_collided_with_bit_buffer == has_already_collided_with_bit_buffer_offset + other_collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					if (scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer == has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					const u64 other_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_layer_bitmask;
					u64 common_collision_layer_bitmask = 0;

					for (u32 li = 0; li < collision_layer_count; li++)
					{
						const u32 current_bit = scene->collision_attribute_core.collision_layer_index_buffer[collision_layer_index_buffer_offset + li];

						const b32 self_check = SUNDER_IS_ANY_BIT_SET(collision_layer_bitmask, current_bit, 1ull);
						const b32 self_cull_check = SUNDER_IS_ANY_BIT_SET(cull_collision_layer_bitmask, current_bit, 1ull);
						const b32 other_check = SUNDER_IS_ANY_BIT_SET(other_collision_layer_bitmask, current_bit, 1ull);

						if (self_check && other_check && !self_cull_check)
						{
							SUNDER_SET_BIT(common_collision_layer_bitmask, current_bit, 1ull);
						}
					}

					if (common_collision_layer_bitmask == 0)
					{
						scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index;
						SUNDER_SKIP;
					}

					const u32 other_trace_step_global_index = grid->trace_instance_buffer[collision_attribute_index_buffer_offset + tsi].trace_step_global_index;

					const u32 other_aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].aabb_index;
					const sunder_v3_t other_aabb_translation = scene->collision_attribute_core.trace_step_buffer[other_trace_step_global_index].position;
					const sunder_v3_t other_aabb_scale = scene->scale_buffer[other_aabb_entity_index];

					const b32 aabbs_intersect = lightray_aabbs_intersect(aabb_translation, aabb_scale, other_aabb_translation, other_aabb_scale);

					if (!aabbs_intersect)
					{
						scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index;
						SUNDER_SKIP;
					}
					
					scene->collision_attribute_core.has_already_collided_with_bit_buffer = has_already_collided_with_bit_buffer_offset + other_collision_attribute_index;

					for (u32 cm = 0; cm < collision_mesh_count; cm++)
					{
						const u32 sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_vertex_count;
						sunder_v3_t* trace_step_sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].trace_step_sentinel_world_space_vertex_position_buffer + (sentinel_vertex_count * sc);

						lightray_handle_collision_mesh_vertex_projection(scene, collision_attribute_entity_transform, trace_step_sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, collision_mesh_buffer_offset + cm, collision_attribute_entity_non_default_rotation);
					}

					scene->collision_attribute_core.trace_step_buffer[trace_step_buffer_index].marked = SUNDER_TRUE;
				}

				// static collision attribute pass
				for (u32 ocai = 0; ocai < other_collision_attribute_index_count; ocai++)
				{
					const b32 marked = scene->collision_attribute_core.trace_step_buffer[trace_step_grid_cell_buffer_index].marked;

					if (marked)
					{
						SUNDER_SKIP;
					}

					const u32 other_collision_attribute_index = grid->collision_attribute_index_buffer[collision_attribute_index_buffer_offset + ocai];

					if (other_collision_attribute_index == collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					if (scene->collision_attribute_core.has_already_collided_with_bit_buffer == has_already_collided_with_bit_buffer_offset + other_collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					if (scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer == has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					const u64 other_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_layer_bitmask;
					u64 common_collision_layer_bitmask = 0;

					for (u32 li = 0; li < collision_layer_count; li++)
					{
						const u32 current_bit = scene->collision_attribute_core.collision_layer_index_buffer[collision_layer_index_buffer_offset + li];

						const b32 self_check = SUNDER_IS_ANY_BIT_SET(collision_layer_bitmask, current_bit, 1ull);
						const b32 self_cull_check = SUNDER_IS_ANY_BIT_SET(cull_collision_layer_bitmask, current_bit, 1ull);
						const b32 other_check = SUNDER_IS_ANY_BIT_SET(other_collision_layer_bitmask, current_bit, 1ull);

						if (self_check && other_check && !self_cull_check)
						{
							SUNDER_SET_BIT(common_collision_layer_bitmask, current_bit, 1ull);
						}
					}

					if (common_collision_layer_bitmask == 0)
					{
						scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index;
						SUNDER_SKIP;
					}

					const u32 other_aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].aabb_index;
					const u32 other_collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].entity_index;
					const sunder_v3_t other_collision_attribute_entity_translation = scene->position_buffer[other_collision_attribute_entity_index];

					const sunder_v3_t other_aabb_translation = scene->position_buffer[other_aabb_entity_index] + other_collision_attribute_entity_translation;
					const sunder_v3_t other_aabb_scale = scene->scale_buffer[other_aabb_entity_index];

					const b32 aabbs_intersect = lightray_aabbs_intersect(aabb_translation, aabb_scale, other_aabb_translation, other_aabb_scale);

					if (!aabbs_intersect)
					{
						scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index;
						SUNDER_SKIP;
					}

					scene->collision_attribute_core.has_already_collided_with_bit_buffer = has_already_collided_with_bit_buffer_offset + other_collision_attribute_index;

					for (u32 cm = 0; cm < collision_mesh_count; cm++)
					{
						const u32 sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_vertex_count;
						sunder_v3_t* trace_step_sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].trace_step_sentinel_world_space_vertex_position_buffer + (sentinel_vertex_count * sc);

						lightray_handle_collision_mesh_vertex_projection(scene, collision_attribute_entity_transform, trace_step_sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, collision_mesh_buffer_offset + cm, collision_attribute_entity_non_default_rotation);
					}

					scene->collision_attribute_core.trace_step_buffer[trace_step_buffer_index].marked = SUNDER_TRUE;
					scene->collision_attribute_core.trace_data_buffer[i].should_resolve = SUNDER_TRUE;
				}
			}
		}
	}
}

b32 lightray_step(lightray_scene_t* scene, lightray_grid_t* grid, u32 collision_attribute_index, const sunder_v3_t& step_position, u32 flags, f32* penetration_depth)
{
	const b32 include_dynamic = SUNDER_IS_ANY_BIT_SET(flags, LIGHTRAY_STEP_BITS_INCLUDE_DYNAMIC_BIT, 1u);
	const b32 solve_epa = SUNDER_IS_ANY_BIT_SET(flags, LIGHTRAY_STEP_BITS_SOLVE_EPA_BIT, 1u);

	const u32 collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].entity_index;
	const u32 collision_attribute_entity_flags = scene->entity_buffer[collision_attribute_entity_index].flags;
	const u32 collision_attribute_flags = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].flags;
	const u32 max_trace_step_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].max_trace_step_count;

	const u32 bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_mesh_batch_index;
	const u32 capsule_mesh_index = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_buffer_offset;
	sunder_v3_t* sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].sentinel_world_space_vertex_position_buffer;
	const u32 sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].sentinel_vertex_count;
	const u32 sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
	const sunder_v3_t* capsule_sentinel_local_space_vertex_position_buffer = scene->sentinel_local_space_vertex_position_buffer + sentinel_local_space_vertex_position_buffer_offset;
	const b32 non_unit_scale = SUNDER_IS_ANY_BIT_SET(collision_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
	const u32 capsule_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].entity_index;
	const sunder_v3_t capsule_mesh_scale = scene->scale_buffer[capsule_mesh_entity_index];

	const u32 aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].aabb_index;
	const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];
	const sunder_v3_t aabb_translation = scene->position_buffer[aabb_entity_index];

	const u32 has_already_collided_with_bit_buffer_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_collided_with_bitmask_buffer_offset);
	const u32 has_already_failed_to_collide_bit_buffer_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_failed_to_collide_bit_buffer_offset);

	const u64 collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask;
	const u64 cull_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].cull_collision_layer_bitmask;
	const u32 collision_layer_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_count;
	const u32 collision_layer_index_buffer_offset = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_index_buffer_offset;
	
	b32 intersects = SUNDER_FALSE;

	for (u32 j = 0; j < scene->collision_attribute_core.has_already_collided_with_bit_buffer.count_per; j++)
	{
		scene->collision_attribute_core.has_already_collided_with_bit_buffer.buffer[scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_collided_with_bitmask_buffer_offset + j] = 0;
	}

	for (u32 j = 0; j < scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.count_per; j++)
	{
		scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.buffer[scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_failed_to_collide_bit_buffer_offset + j] = 0;
	}

		lightray_grid_cell_aabb_coordinates_t row_coordinates{};
		lightray_grid_cell_aabb_coordinates_t column_coordinates{};
		const sunder_v3_t aabb_translation_step = aabb_translation + step_position;

		// out of grid bounds!!!
		lightray_get_grid_cell_coordinates_aabb(grid, &aabb_translation_step, &aabb_scale, &row_coordinates, &column_coordinates);

		const b32 in_grid_bounds = lightray_aabb_in_grid_bounds(grid, &row_coordinates, &column_coordinates);

		if (!in_grid_bounds)
		{
			SUNDER_LOG("\nOUT OF BOUNDS!!!");
			return SUNDER_FALSE;
		}

		const u32 aabb_width_per_row = column_coordinates.max - column_coordinates.min + 1;

		for (i32 ri = row_coordinates.min; ri < row_coordinates.max + 1; ri++)
		{
			const u32 current_row_index = lightray_get_grid_cell_index(grid, ri, column_coordinates.min);

			for (u32 w = 0; w < aabb_width_per_row; w++)
			{
				const u32 current_grid_cell_index = current_row_index + w;
				const u32 collision_attribute_index_buffer_offset = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_buffer_offset; // segfault will happen here if aabb is out of grid bounds
				const u32 collision_attribute_index_count = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_count;
				const u32 trace_instance_count = grid->cell_buffer[current_grid_cell_index].trace_instance_count;

				if (include_dynamic)
				{
					for (u32 ti = 0; ti < trace_instance_count; ti++)
					{
						const u32 other_collision_attribute_index = grid->trace_instance_buffer[collision_attribute_index_buffer_offset + ti].collision_attribute_index;

						if (other_collision_attribute_index == collision_attribute_index)
						{
							SUNDER_SKIP;
						}

						if (scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer == (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index))
						{
							SUNDER_SKIP;
						}

						const u64 other_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_layer_bitmask;
						u64 common_collision_layer_bitmask = 0;

						for (u32 li = 0; li < collision_layer_count; li++)
						{
							const u32 current_bit = scene->collision_attribute_core.collision_layer_index_buffer[collision_layer_index_buffer_offset + li];

							const b32 self_check = SUNDER_IS_ANY_BIT_SET(collision_layer_bitmask, current_bit, 1ull);
							const b32 self_cull_check = SUNDER_IS_ANY_BIT_SET(cull_collision_layer_bitmask, current_bit, 1ull);
							const b32 other_check = SUNDER_IS_ANY_BIT_SET(other_collision_layer_bitmask, current_bit, 1ull);

							if (self_check && other_check && !other_check)
							{
								SUNDER_SET_BIT(common_collision_layer_bitmask, current_bit, 1ull);
							}
						}

						if (common_collision_layer_bitmask == 0)
						{
							scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
							SUNDER_SKIP;
						}

						const u32 other_trace_step_buffer_index = other_collision_attribute_index * scene->collision_attribute_core.trace_step_buffer.count_per;
						const sunder_v3_t other_aabb_position = scene->collision_attribute_core.trace_step_buffer[other_trace_step_buffer_index].position;
						const u32 other_aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].aabb_index;
						const sunder_v3_t other_aabb_scale = scene->scale_buffer[other_aabb_entity_index];

						const b32 aabbs_intersect = lightray_aabbs_intersect(step_position, aabb_scale, other_aabb_position, other_aabb_scale);

						if (!aabbs_intersect)
						{
							scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
							SUNDER_SKIP;
						}

						if (non_unit_scale)
						{
							for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
							{
								sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_ts(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position, capsule_mesh_scale);
							}
						}

						else
						{
							for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
							{
								sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_t(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position);
							}
						}

						const u32 other_bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_mesh_batch_index;
						const u32 other_collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_count;
						const u32 other_collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_buffer_offset;

						lightray_gjk_support_point_t simplex[4]{};
						u32 simplex_size = 0;

						for (u32 ocm = 0; ocm < other_collision_mesh_count; ocm++)
						{
							sunder_v3_t* other_sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_world_space_vertex_position_buffer;
							const u32 other_sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_vertex_count;

							const b32 meshes_intersect = lightray_gjk_intersect(sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, other_sentinel_world_space_vertex_position_buffer, other_sentinel_vertex_count, simplex, &simplex_size);

							if (!meshes_intersect)
							{
								scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
								SUNDER_SKIP;
							}

							simplex[0].p = sunder_v3_scalar(0.0f);
							simplex[1].p = sunder_v3_scalar(0.0f);
							simplex[2].p = sunder_v3_scalar(0.0f);
							simplex[3].p = sunder_v3_scalar(0.0f);
							simplex_size = 0;

							intersects = SUNDER_TRUE;

							goto post_trace_instance_collision;
						}
					}
				}
				
			post_trace_instance_collision:

				for (u32 cai = 0; cai < collision_attribute_index_count; cai++)
				{
					const u32 other_collision_attribute_index = grid->collision_attribute_index_buffer[collision_attribute_index_buffer_offset + cai];

					if (other_collision_attribute_index == collision_attribute_index)
					{
						SUNDER_SKIP;
					}

					if (scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer == (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index))
					{
						SUNDER_SKIP;
					}

					const u64 other_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_layer_bitmask;
					u64 common_collision_layer_bitmask = 0;

					for (u32 li = 0; li < collision_layer_count; li++)
					{
						const u32 current_bit = scene->collision_attribute_core.collision_layer_index_buffer[collision_layer_index_buffer_offset + li];

						const b32 self_check = SUNDER_IS_ANY_BIT_SET(collision_layer_bitmask, current_bit, 1ull);
						const b32 self_cull_check = SUNDER_IS_ANY_BIT_SET(cull_collision_layer_bitmask, current_bit, 1ull);
						const b32 other_check = SUNDER_IS_ANY_BIT_SET(other_collision_layer_bitmask, current_bit, 1ull);

						if (self_check && other_check && !self_cull_check)
						{
							SUNDER_SET_BIT(common_collision_layer_bitmask, current_bit, 1ull);
						}
					}

					if (common_collision_layer_bitmask == 0)
					{
						scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
						SUNDER_SKIP;
					}

					const u32 other_collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].entity_index;
					const u32 other_aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].aabb_index;
					const sunder_v3_t other_aabb_position = scene->position_buffer[other_collision_attribute_entity_index] + scene->position_buffer[other_aabb_entity_index];
					const sunder_v3_t other_aabb_scale = scene->scale_buffer[other_aabb_entity_index];

					const b32 aabbs_intersect = lightray_aabbs_intersect(step_position, aabb_scale, other_aabb_position, other_aabb_scale);

					if (!aabbs_intersect)
					{
						scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
						SUNDER_SKIP;
					}

					if (non_unit_scale)
					{
						for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
						{
							sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_ts(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position, capsule_mesh_scale);
						}
					}

					else
					{
						for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
						{
							sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_t(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position);
						}
					}

					const u32 other_bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_mesh_batch_index;
					const u32 other_collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_count;
					const u32 other_collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_buffer_offset;

					lightray_gjk_support_point_t simplex[4]{};
					u32 simplex_size = 0;

					for (u32 ocm = 0; ocm < other_collision_mesh_count; ocm++)
					{
						sunder_v3_t* other_sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_world_space_vertex_position_buffer;
						const u32 other_sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_vertex_count;

						const b32 meshes_intersect = lightray_gjk_intersect(sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, other_sentinel_world_space_vertex_position_buffer, other_sentinel_vertex_count, simplex, &simplex_size);

						if (!meshes_intersect)
						{
							scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
							SUNDER_SKIP;
						}

						if (solve_epa)
						{
							const lightray_epa_result_t epa_res = lightray_solve_epa(simplex, simplex_size, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, other_sentinel_world_space_vertex_position_buffer, other_sentinel_vertex_count);
							*penetration_depth = epa_res.depth;
						}

						simplex[0].p = sunder_v3_scalar(0.0f);
						simplex[1].p = sunder_v3_scalar(0.0f);
						simplex[2].p = sunder_v3_scalar(0.0f);
						simplex[3].p = sunder_v3_scalar(0.0f);
						simplex_size = 0;

						intersects = SUNDER_TRUE;

						goto post_static_intersection;
					}
				}
			}
		}

	post_static_intersection:

	return intersects;
}

void lightray_resolve_sentient_collision(lightray_scene_t* scene, lightray_grid_t* grid)
{
	const u32 trace_data_count = scene->collision_attribute_core.trace_data_buffer.size;

	for (u32 i = 0; i < trace_data_count; i++)
	{
		if (!scene->collision_attribute_core.trace_data_buffer[i].should_resolve)
		{
			scene->position_buffer[scene->collision_attribute_core.collision_attribute_buffer[scene->collision_attribute_core.trace_data_buffer[i].collision_attribute_index].entity_index] = scene->collision_attribute_core.trace_data_buffer[i].previous_position + scene->collision_attribute_core.trace_data_buffer[i].movement_this_frame;
			
			SUNDER_SKIP;
		}

		const u32 step_flags = (1u << LIGHTRAY_STEP_BITS_INCLUDE_DYNAMIC_BIT);
		const u32 collision_attribute_index = scene->collision_attribute_core.trace_data_buffer[i].collision_attribute_index;
		const u32 collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].entity_index;
		const u32 collision_attribute_entity_flags = scene->entity_buffer[collision_attribute_entity_index].flags;
		const u32 collision_attribute_flags = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].flags;
		const u32 max_trace_step_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].max_trace_step_count;

		const b32 bound = SUNDER_IS_ANY_BIT_SET(collision_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_BOUND_BIT, 1);
		const b32 non_default_rotation = SUNDER_IS_ANY_BIT_SET(collision_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1);
		const b32 sentient = SUNDER_IS_ANY_BIT_SET(collision_attribute_flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_SENTIENT_BIT, 1u);
		const b32 deferred = SUNDER_IS_ANY_BIT_SET(collision_attribute_flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_DEFERRED_BIT, 1u);

		if (bound || non_default_rotation || !sentient || deferred)
		{
			SUNDER_SKIP;
		}

		const u32 bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_mesh_batch_index;
		const u32 capsule_mesh_index = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_buffer_offset;
		sunder_v3_t* sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].sentinel_world_space_vertex_position_buffer;
		const u32 sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].sentinel_vertex_count;
		const u32 sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].sentinel_local_space_vertex_position_buffer_offset;
		const sunder_v3_t* capsule_sentinel_local_space_vertex_position_buffer = scene->sentinel_local_space_vertex_position_buffer + sentinel_local_space_vertex_position_buffer_offset;
		const b32 non_unit_scale = SUNDER_IS_ANY_BIT_SET(collision_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
		const u32 capsule_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[capsule_mesh_index].entity_index;
		const sunder_v3_t capsule_mesh_scale = scene->scale_buffer[capsule_mesh_entity_index];

		const u32 aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].aabb_index;
		const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];
		const sunder_v3_t aabb_translation = scene->position_buffer[aabb_entity_index];

		const u32 has_already_collided_with_bit_buffer_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_collided_with_bitmask_buffer_offset);
		const u32 has_already_failed_to_collide_bit_buffer_offset = SUNDER_COMPUTE_BUFFERED_BIT_OFFSET(scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_failed_to_collide_bit_buffer_offset);

		for (u32 j = 0; j < scene->collision_attribute_core.has_already_collided_with_bit_buffer.count_per; j++)
		{
			scene->collision_attribute_core.has_already_collided_with_bit_buffer.buffer[scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_collided_with_bitmask_buffer_offset + j] = 0;
		}

		for (u32 j = 0; j < scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.count_per; j++)
		{
			scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.buffer[scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_failed_to_collide_bit_buffer_offset + j] = 0;
		}

		const u64 collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_bitmask;
		const u64 cull_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].cull_collision_layer_bitmask;
		const u32 collision_layer_count = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_count;
		const u32 collision_layer_index_buffer_offset = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].collision_layer_index_buffer_offset;

		const sunder_v3_t velocity_x = sunder_v3(scene->collision_attribute_core.trace_data_buffer[i].velocity.x, 0.0f, 0.0f);
		const sunder_v3_t velocity_y = sunder_v3(0.0f, scene->collision_attribute_core.trace_data_buffer[i].velocity.y, 0.0f);
		const sunder_v3_t velocity_z = sunder_v3(0.0f, 0.0f, scene->collision_attribute_core.trace_data_buffer[i].velocity.z);

		u32 trace_step_count_table[3]{};

		trace_step_count_table[0] = sunder_clamp_u32(0, max_trace_step_count - 1, ((u32)(sunder_squared_magnitude_v3(velocity_x) * 0.1f)));
		trace_step_count_table[1] = sunder_clamp_u32(0, max_trace_step_count - 1, ((u32)(sunder_squared_magnitude_v3(velocity_y) * 0.1f)));
		trace_step_count_table[2] = sunder_clamp_u32(0, max_trace_step_count - 1, ((u32)(sunder_squared_magnitude_v3(velocity_z) * 0.1f)));

		const f32 trace_step_factor = scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].trace_step_factor;

		sunder_v3_t previous_position = scene->collision_attribute_core.trace_data_buffer[i].previous_position;

		f32 step_factor_table[3]{};
		step_factor_table[0] = 0.5f; // 1 + 1
		step_factor_table[1] = 0.4f; // 2 + 1
		step_factor_table[2] = 0.3f; // 3 + 1

		f32 t_table[3]{};

		for (u32 sfti = 0; sfti < 3; sfti++)
		{
			if (trace_step_count_table[sfti] == 0)
			{
				step_factor_table[sfti] = 1.0f;
			}

			else
			{
				trace_step_count_table[sfti] -= 1;
			}

			t_table[sfti] = step_factor_table[trace_step_count_table[sfti]];
		}

		f32 t_prev_table[3]{};

		sunder_v3_t previous_axes{};
		previous_axes.x = scene->collision_attribute_core.trace_data_buffer[i].movement_this_frame.x;

		const sunder_v3_t movement_this_frame = scene->collision_attribute_core.trace_data_buffer[i].movement_this_frame;

		f32* movement_this_frame_ptr = &scene->collision_attribute_core.trace_data_buffer[i].movement_this_frame.x;

		u32 trace_step_iter = 0;

		f32 trace_instance_safe_t_prev_table[3]{};
		f32 static_collision_attribute_safe_t_prev_table[3]{};

		f32 t_x = 1.0f;
		f32 t_prev_x = 0.0f;

		f32 t_y = 1.0f;
		f32 t_prev_y = 0.0f;

		f32 t_z = 1.0f;
		f32 t_prev_z = 0.0f;

		f32 step = 1.0f;

		while (t_x <= 1.0f)
		{
			if (t_prev_x == 1.0f)
			{
				break;
			}

			const sunder_v3_t step_position = previous_position + (sunder_v3(movement_this_frame.x, 0.0f, 0.0f) * t_x);

			const b32 intersects = lightray_step(scene, grid, collision_attribute_index, step_position, step_flags, SUNDER_NULLPTR);

			if (intersects)
			{
				break;
			}

			const f32 t_diff = 1.0f - t_x;

			if (t_diff < step)
			{
				t_prev_x = t_x;
				t_x = 1.0f;
			}

			else
			{
				t_prev_x = t_x;
				t_x += step;
			}
		}

		const sunder_v3_t y_movement = previous_position + (sunder_v3(movement_this_frame.x, 0.0f, 0.0f) * t_prev_x);

		while (t_y <= 1.0f)
		{
			if (t_prev_y == 1.0f)
			{
				break;
			}

			const sunder_v3_t step_position = y_movement + (sunder_v3(0.0f, movement_this_frame.y, 0.0f) * t_y);

			const b32 intersects = lightray_step(scene, grid, collision_attribute_index, step_position, step_flags, SUNDER_NULLPTR);

			if (intersects)
			{
				break;
			}

			const f32 t_diff = 1.0f - t_y;

			if (t_diff < step)
			{
				t_prev_y = t_y;
				t_y = 1.0f;
			}

			else
			{
				t_prev_y = t_y;
				t_y += step;
			}
		}

		const sunder_v3_t z_movement  = y_movement + (sunder_v3(0.0f, movement_this_frame.y, 0.0f) * t_prev_y);

		while (t_z <= 1.0f)
		{
			if (t_prev_z == 1.0f)
			{
				break;
			}

			const sunder_v3_t step_position = z_movement + (sunder_v3(0.0f, 0.0f, movement_this_frame.z) * t_z);

			const b32 intersects = lightray_step(scene, grid, collision_attribute_index, step_position, step_flags, SUNDER_NULLPTR);

			if (intersects)
			{
				break;
			}

			const f32 t_diff = 1.0f - t_z;

			if (t_diff < step)
			{
				t_prev_z = t_z;
				t_z = 1.0f;
			}

			else
			{
				t_prev_z = t_z;
				t_z += step;
			}
		}

		scene->position_buffer[collision_attribute_entity_index] = z_movement + (sunder_v3(0.0f, 0.0f, movement_this_frame.z) * t_prev_z);

		/*
		for (u32 axis = 0; axis < 3; axis++)
		{
			trace_step_iter = 0;

			while (t_table[axis] <= 1.0f)
			{
				if (t_prev_table[axis] == 1.0f)
				{
					SUNDER_TERMINATE;
				}

				const sunder_v3_t step_position = previous_position + (previous_axes * t_table[axis]);

				b32 intersects = SUNDER_FALSE;

				lightray_grid_cell_aabb_coordinates_t row_coordinates{};
				lightray_grid_cell_aabb_coordinates_t column_coordinates{};
				const sunder_v3_t aabb_translation_step = aabb_translation + step_position;

				lightray_get_grid_cell_coordinates_aabb(grid, &aabb_translation_step, &aabb_scale, &row_coordinates, &column_coordinates);

				const u32 aabb_width_per_row = column_coordinates.max - column_coordinates.min + 1;

				for (i32 ri = row_coordinates.min; ri < row_coordinates.max + 1; ri++)
				{
					const u32 current_row_index = lightray_get_grid_cell_index(grid, ri, column_coordinates.min);

					for (u32 w = 0; w < aabb_width_per_row; w++)
					{
						const u32 current_grid_cell_index = current_row_index + w;
						const u32 collision_attribute_index_buffer_offset = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_buffer_offset;
						const u32 collision_attribute_index_count = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_count;
						const u32 trace_instance_count = grid->cell_buffer[current_grid_cell_index].trace_instance_count;

						for (u32 ti = 0; ti < trace_instance_count; ti++)
						{
							const u32 other_collision_attribute_index = grid->trace_instance_buffer[collision_attribute_index_buffer_offset + ti].collision_attribute_index;

							if (other_collision_attribute_index == collision_attribute_index)
							{
								SUNDER_SKIP;
							}

							if (scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer == (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index))
							{
								SUNDER_SKIP;
							}

							const u64 other_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_layer_bitmask;
							u64 common_collision_layer_bitmask = 0;

							for (u32 li = 0; li < collision_layer_count; li++)
							{
								const u32 current_bit = scene->collision_attribute_core.collision_layer_index_buffer[collision_layer_index_buffer_offset + li];

								const b32 self_check = SUNDER_IS_ANY_BIT_SET(collision_layer_bitmask, current_bit, 1ull);
								const b32 self_cull_check = SUNDER_IS_ANY_BIT_SET(cull_collision_layer_bitmask, current_bit, 1ull);
								const b32 other_check = SUNDER_IS_ANY_BIT_SET(other_collision_layer_bitmask, current_bit, 1ull);

								if (self_check && other_check && !other_check)
								{
									SUNDER_SET_BIT(common_collision_layer_bitmask, current_bit, 1ull);
								}
							}

							if (common_collision_layer_bitmask == 0)
							{
								scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
								SUNDER_SKIP;
							}

							const u32 other_trace_step_buffer_index = other_collision_attribute_index * scene->collision_attribute_core.trace_step_buffer.count_per;
							const sunder_v3_t other_aabb_position = scene->collision_attribute_core.trace_step_buffer[other_trace_step_buffer_index].position;
							const u32 other_aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].aabb_index;
							const sunder_v3_t other_aabb_scale = scene->scale_buffer[other_aabb_entity_index];

							const b32 aabbs_intersect = lightray_aabbs_intersect(step_position, aabb_scale, other_aabb_position, other_aabb_scale);

							if (!aabbs_intersect)
							{
								scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
								SUNDER_SKIP;
							}

							if (non_unit_scale)
							{
								for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
								{
									sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_ts(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position, capsule_mesh_scale);
								}
							}

							else
							{
								for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
								{
									sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_t(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position);
								}
							}

							const u32 other_bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_mesh_batch_index;
							const u32 other_collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_count;
							const u32 other_collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_buffer_offset;

							lightray_gjk_support_point_t simplex[4]{};
							u32 simplex_size = 0;

							for (u32 ocm = 0; ocm < other_collision_mesh_count; ocm++)
							{
								sunder_v3_t* other_sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_world_space_vertex_position_buffer;
								const u32 other_sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_vertex_count;

								const b32 meshes_intersect = lightray_gjk_intersect(sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, other_sentinel_world_space_vertex_position_buffer, other_sentinel_vertex_count, simplex, &simplex_size);

								if (!meshes_intersect)
								{
									scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
									SUNDER_SKIP;
								}

								intersects = SUNDER_TRUE;
								trace_instance_safe_t_prev_table[axis] = t_prev_table[axis];

								goto post_trace_instance_collision;
							}
						}

					post_trace_instance_collision:

						for (u32 cai = 0; cai < collision_attribute_index_count; cai++)
						{
							const u32 other_collision_attribute_index = grid->collision_attribute_index_buffer[collision_attribute_index_buffer_offset + cai];

							if (other_collision_attribute_index == collision_attribute_index)
							{
								SUNDER_SKIP;
							}

							if (scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer == (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index))
							{
								SUNDER_SKIP;
							}

							const u64 other_collision_layer_bitmask = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_layer_bitmask;
							u64 common_collision_layer_bitmask = 0;

							for (u32 li = 0; li < collision_layer_count; li++)
							{
								const u32 current_bit = scene->collision_attribute_core.collision_layer_index_buffer[collision_layer_index_buffer_offset + li];

								const b32 self_check = SUNDER_IS_ANY_BIT_SET(collision_layer_bitmask, current_bit, 1ull);
								const b32 self_cull_check = SUNDER_IS_ANY_BIT_SET(cull_collision_layer_bitmask, current_bit, 1ull);
								const b32 other_check = SUNDER_IS_ANY_BIT_SET(other_collision_layer_bitmask, current_bit, 1ull);

								if (self_check && other_check && !self_cull_check)
								{
									SUNDER_SET_BIT(common_collision_layer_bitmask, current_bit, 1ull);
								}
							}

							if (common_collision_layer_bitmask == 0)
							{
								scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
								SUNDER_SKIP;
							}

							const u32 other_collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].entity_index;
							const u32 other_aabb_entity_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].aabb_index;
							const sunder_v3_t other_aabb_position = scene->position_buffer[other_collision_attribute_entity_index] + scene->position_buffer[other_aabb_entity_index];
							const sunder_v3_t other_aabb_scale = scene->scale_buffer[other_aabb_entity_index];

							const b32 aabbs_intersect = lightray_aabbs_intersect(step_position, aabb_scale, other_aabb_position, other_aabb_scale);

							if (!aabbs_intersect)
							{
								scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
								SUNDER_SKIP;
							}

							if (non_unit_scale)
							{
								for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
								{
									sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_ts(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position, capsule_mesh_scale);
								}
							}

							else
							{
								for (u32 sv = 0; sv < sentinel_vertex_count; sv++)
								{
									sentinel_world_space_vertex_position_buffer[sv] = sunder_v3_world_space_t(capsule_sentinel_local_space_vertex_position_buffer[sv], step_position);
								}
							}

							const u32 other_bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[other_collision_attribute_index].collision_mesh_batch_index;
							const u32 other_collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_count;
							const u32 other_collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[other_bound_collision_mesh_batch_index].collision_mesh_buffer_offset;

							lightray_gjk_support_point_t simplex[4]{};
							u32 simplex_size = 0;

							for (u32 ocm = 0; ocm < other_collision_mesh_count; ocm++)
							{
								sunder_v3_t* other_sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_world_space_vertex_position_buffer;
								const u32 other_sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[other_collision_mesh_buffer_offset + ocm].sentinel_vertex_count;

								const b32 meshes_intersect = lightray_gjk_intersect(sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, other_sentinel_world_space_vertex_position_buffer, other_sentinel_vertex_count, simplex, &simplex_size);

								if (!meshes_intersect)
								{
									scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer = (has_already_failed_to_collide_bit_buffer_offset + other_collision_attribute_index);
									SUNDER_SKIP;
								}

								intersects = SUNDER_TRUE;
								static_collision_attribute_safe_t_prev_table[axis] = t_prev_table[axis];

								goto post_static_intersection;
							}
						}
					}
				}

				post_static_intersection:

				if (intersects)
				{
					if (static_collision_attribute_safe_t_prev_table[axis] < trace_instance_safe_t_prev_table[axis])
					{
						t_prev_table[axis] = static_collision_attribute_safe_t_prev_table[axis];
					}

					else if (static_collision_attribute_safe_t_prev_table[axis] == trace_instance_safe_t_prev_table[axis])
					{
						t_prev_table[axis] = static_collision_attribute_safe_t_prev_table[axis];
					}

					else
					{
						t_prev_table[axis] = trace_instance_safe_t_prev_table[axis];
					}

					SUNDER_TERMINATE;
				}

				const f32 t_diff = 1.0f - t_table[axis];

				if (t_diff < trace_step_factor)
				{
					t_prev_table[axis] = t_table[axis];
					t_table[axis] = 1.0f;
				}

				else
				{
					t_prev_table[axis] = t_table[axis];
					t_table[axis] += step_factor_table[axis];
				}

				trace_step_iter++;
			}

			f32* previous_axes_ptr = &previous_axes.x;
			previous_position = previous_position + (previous_axes * t_prev_table[axis]);
			previous_axes = sunder_v3_scalar(0.0f);
			previous_axes_ptr[axis] = movement_this_frame_ptr[axis];

			SUNDER_LOG("previous_axes: ");
			sunder_log_v3(previous_axes);

			for (u32 j = 0; j < scene->collision_attribute_core.has_already_collided_with_bit_buffer.count_per; j++)
			{
				scene->collision_attribute_core.has_already_collided_with_bit_buffer.buffer[scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_collided_with_bitmask_buffer_offset + j] = 0;
			}

			for (u32 j = 0; j < scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.count_per; j++)
			{
				scene->collision_attribute_core.has_already_failed_to_collide_bit_buffer.buffer[scene->collision_attribute_core.collision_attribute_buffer[collision_attribute_index].has_already_failed_to_collide_bit_buffer_offset + j] = 0;
			}
		}
		*/


		//scene->position_buffer[collision_attribute_entity_index] = previous_position;
	}

	

	// do a pass for populating a default collision attribute index buffer
}

void lightray_resolve_primal_collision(lightray_scene_t* scene, lightray_grid_t* grid)
{

}

void lightray_pre_compute_collision_and_overlap_static_meshes_data(lightray_scene_t* scene, lightray_grid_t* grid)
{
	const u32 overlap_attribute_count = scene->overlap_attribute_core.overlap_attribute_buffer.size;
	const u32 collision_attribute_count = scene->collision_attribute_core.collision_attribute_buffer.size;

	for (u32 i = 0; i < overlap_attribute_count; i++)
	{
		const u32 overlap_attribute_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[i].entity_index;
		const u32 overlap_attribute_entity_flags = scene->entity_buffer[overlap_attribute_entity_index].flags;

		if (SUNDER_IS_ANY_BIT_SET(overlap_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_BOUND_BIT, 1u))
		{
			SUNDER_SKIP;
		}

		if (!(SUNDER_IS_ANY_BIT_SET(scene->overlap_attribute_core.overlap_attribute_buffer[i].flags, LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_STATIC_BIT, 1u)))
		{
			SUNDER_SKIP;
		}

		const u32 bound_overlap_mesh_batch_index = scene->overlap_attribute_core.overlap_attribute_buffer[i].overlap_mesh_batch_index;
		const u32 overlap_mesh_count = scene->overlap_attribute_core.overlap_mesh_batch_buffer[bound_overlap_mesh_batch_index].overlap_mesh_count;
		const u32 overlap_mesh_buffer_offset = scene->overlap_attribute_core.overlap_mesh_batch_buffer[bound_overlap_mesh_batch_index].overlap_mesh_buffer_offset;
		const sunder_m4_t overlap_attribute_entity_transform = sunder_m4_model(scene->position_buffer[overlap_attribute_entity_index], scene->quat_rotation_buffer[overlap_attribute_entity_index], scene->scale_buffer[overlap_attribute_entity_index]);

		for (u32 ovm = 0; ovm < overlap_mesh_count; ovm++)
		{
			const u32 sentinel_local_space_vertex_position_buffer_offset = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_local_space_vertex_position_buffer_offset;
			const sunder_v3_t* sentinel_local_space_vertex_position_buffer = scene->sentinel_local_space_vertex_position_buffer + sentinel_local_space_vertex_position_buffer_offset;
			sunder_v3_t* sentinel_world_space_vertex_position_buffer = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_world_space_vertex_position_buffer;
			const u32 sentinel_vertex_count = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].sentinel_vertex_count;
			const u32 overlap_mesh_entity_index = scene->overlap_attribute_core.overlap_mesh_buffer[overlap_mesh_buffer_offset + ovm].entity_index;
			sunder_m4_t overlap_mesh_transform = sunder_m4_model(scene->position_buffer[overlap_mesh_entity_index], scene->quat_rotation_buffer[overlap_mesh_entity_index], scene->scale_buffer[overlap_mesh_entity_index]);
			const u32 overlap_mesh_entity_flags = scene->entity_buffer[overlap_mesh_entity_index].flags;

			const b32 inherits_translation = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_TRANSLATION_BIT, 1);
			const b32 inherits_scale = SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_SCALE_BIT, 1);
			const b32 inherits_rotation= SUNDER_IS_ANY_BIT_SET(overlap_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_ROTATION_BIT, 1);

			b32 identity = SUNDER_FALSE;
			const sunder_m4_t inherited_transform = lightray_compute_inherited_transform(overlap_attribute_entity_transform, scene->position_buffer[overlap_attribute_entity_index], scene->quat_rotation_buffer[overlap_attribute_entity_index], scene->scale_buffer[overlap_attribute_entity_index], inherits_translation, inherits_rotation, inherits_scale, &identity);

			if (!identity)
			{
				overlap_mesh_transform = inherited_transform * overlap_mesh_transform;
			}

			const sunder_m4_t overlap_mesh_transform_inverse = sunder_m4_inverse(overlap_mesh_transform);
			lightray_cache_m4(&scene->raycast_core.m4_inverse_cache_overlap_mesh_buffer, &scene->raycast_core.has_already_cached_inverse_overlap_mesh_bit_buffer, overlap_mesh_transform_inverse, overlap_mesh_buffer_offset + ovm);

			lightray_compute_sentinel_world_space_vertex_positions(sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, overlap_mesh_transform);
		}

		// grid population pass
		const sunder_v3_t overlap_attribute_entity_position = scene->position_buffer[overlap_attribute_entity_index];
		const u32 aabb_entity_index = scene->overlap_attribute_core.overlap_attribute_buffer[i].aabb_index;
		const sunder_v3_t aabb_position = scene->position_buffer[aabb_entity_index] + overlap_attribute_entity_position;
		const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_entity_index];

		lightray_grid_cell_aabb_coordinates_t row_coordinates{};
		lightray_grid_cell_aabb_coordinates_t column_coordinates{};

		lightray_get_grid_cell_coordinates_aabb(grid, &aabb_position, &aabb_scale, &row_coordinates, &column_coordinates);

		const b32 aabb_in_bounds = lightray_aabb_in_grid_bounds(grid, &row_coordinates, &column_coordinates);

		if (!aabb_in_bounds)
		{
			SUNDER_SKIP;
		}

		const u32 aabb_width_per_row = column_coordinates.max - column_coordinates.min + 1;

		for (i32 ri = row_coordinates.min; ri < row_coordinates.max + 1; ri++)
		{
			const u32 current_row_index = lightray_get_grid_cell_index(grid, ri, column_coordinates.min);

			for (u32 w = 0; w < aabb_width_per_row; w++)
			{
				const u32 current_grid_cell_index = current_row_index + w;
				const u32 overlap_attribute_index_buffer_offset = grid->cell_buffer[current_grid_cell_index].overlap_attribute_index_buffer_offset;
				const u32 overlap_attribute_index_count = grid->cell_buffer[current_grid_cell_index].overlap_attribute_index_count;

				grid->overlap_attribute_index_buffer[overlap_attribute_index_buffer_offset + overlap_attribute_index_count] = i;
				grid->cell_buffer[current_grid_cell_index].overlap_attribute_index_count++;
			}
		}
	}


	for (u32 i = 0; i < collision_attribute_count; i++)
	{
		const u32 collision_attribute_entity_index = scene->collision_attribute_core.collision_attribute_buffer[i].entity_index;
		const u32 collision_attribute_entity_flags = scene->entity_buffer[collision_attribute_entity_index].flags;

		if (SUNDER_IS_ANY_BIT_SET(collision_attribute_entity_flags, LIGHTRAY_ENTITY_BITS_BOUND_BIT, 1u))
		{
			SUNDER_SKIP;
		}

		if (!(SUNDER_IS_ANY_BIT_SET(scene->collision_attribute_core.collision_attribute_buffer[i].flags, LIGHTRAY_COLLISION_ATTRIBUTE_BITS_STATIC_BIT, 1u)))
		{
			SUNDER_SKIP;
		}

		const u32 bound_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[i].collision_mesh_batch_index;
		const u32 collision_mesh_count = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_count;
		const u32 collision_mesh_buffer_offset = scene->collision_attribute_core.collision_mesh_batch_buffer[bound_collision_mesh_batch_index].collision_mesh_buffer_offset;

		const sunder_m4_t collision_attribute_entity_transform = sunder_m4_model(scene->position_buffer[collision_attribute_entity_index], scene->quat_rotation_buffer[collision_attribute_entity_index], scene->scale_buffer[collision_attribute_entity_index]);
		sunder_m4_t collision_attribute_entity_transform_temp = collision_attribute_entity_transform;

		for (u32 cm = 0; cm < collision_mesh_count; cm++)
		{
			const u32 sentinel_local_space_vertex_position_buffer_offset = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_local_space_vertex_position_buffer_offset;
			const sunder_v3_t* sentinel_local_space_vertex_position_buffer = scene->sentinel_local_space_vertex_position_buffer + sentinel_local_space_vertex_position_buffer_offset;
			sunder_v3_t* sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_world_space_vertex_position_buffer;
			const u32 sentinel_vertex_count = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].sentinel_vertex_count;
			const u32 collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[collision_mesh_buffer_offset + cm].entity_index;
			const u32 collision_mesh_entity_flags = scene->entity_buffer[collision_mesh_entity_index].flags;
			
			const b32 inherits_scale = SUNDER_IS_ANY_BIT_SET(collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_SCALE_BIT, 1);
			const b32 inherits_rotation = SUNDER_IS_ANY_BIT_SET(collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_ROTATION_BIT, 1);
			const b32 inherits_translation = SUNDER_IS_ANY_BIT_SET(collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_TRANSLATION_BIT, 1);

			const sunder_v3_t extracted_translation = scene->position_buffer[collision_attribute_entity_index];
			const sunder_quat_t extracted_rotation = scene->quat_rotation_buffer[collision_attribute_entity_index];
			const sunder_v3_t extracted_scale = scene->scale_buffer[collision_attribute_entity_index];

			b32 identity = SUNDER_FALSE;
			collision_attribute_entity_transform_temp = lightray_compute_inherited_transform(collision_attribute_entity_transform, extracted_translation, extracted_rotation, extracted_scale, inherits_translation, inherits_rotation, inherits_scale, &identity);

			sunder_m4_t collision_mesh_transform = sunder_m4_model(scene->position_buffer[collision_mesh_entity_index], scene->quat_rotation_buffer[collision_mesh_entity_index], scene->scale_buffer[collision_mesh_entity_index]);

			if (!identity)
			{
				collision_mesh_transform = collision_attribute_entity_transform_temp * collision_mesh_transform;
			}

			const sunder_m4_t collision_mesh_transform_inverse = sunder_m4_inverse(collision_mesh_transform);
			lightray_cache_m4(&scene->raycast_core.m4_inverse_cache_collision_mesh_buffer, &scene->raycast_core.has_already_cached_inverse_collision_mesh_bit_buffer, collision_mesh_transform_inverse, collision_mesh_buffer_offset + cm);

			lightray_compute_sentinel_world_space_vertex_positions(sentinel_local_space_vertex_position_buffer, sentinel_world_space_vertex_position_buffer, sentinel_vertex_count, collision_mesh_transform);
		}

		// grid population pass
		const sunder_v3_t collision_attribute_entity_position = scene->position_buffer[collision_attribute_entity_index];
		const u32 aabb_index = scene->collision_attribute_core.collision_attribute_buffer[i].aabb_index;
		const sunder_v3_t aabb_position = scene->position_buffer[aabb_index] + collision_attribute_entity_position;
		const sunder_v3_t aabb_scale = scene->scale_buffer[aabb_index];

		lightray_grid_cell_aabb_coordinates_t row_coordinates{};
		lightray_grid_cell_aabb_coordinates_t column_coordinates{};

		lightray_get_grid_cell_coordinates_aabb(grid, &aabb_position, &aabb_scale, &row_coordinates, &column_coordinates);

		const b32 aabb_in_bounds = lightray_aabb_in_grid_bounds(grid, &row_coordinates, &column_coordinates);

		if (!aabb_in_bounds)
		{
			SUNDER_SKIP;
		}

		const u32 aabb_width_per_row = column_coordinates.max - column_coordinates.min + 1;

		for (i32 ri = row_coordinates.min; ri < row_coordinates.max + 1; ri++)
		{
			const u32 current_row_index = lightray_get_grid_cell_index(grid, ri, column_coordinates.min);

			for (u32 w = 0; w < aabb_width_per_row; w++)
			{
				const u32 current_grid_cell_index = current_row_index + w;
				const u32 collision_attribute_index_buffer_offset = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_buffer_offset;
				const u32 collision_attribute_index_count = grid->cell_buffer[current_grid_cell_index].collision_attribute_index_count;

				grid->collision_attribute_index_buffer[collision_attribute_index_buffer_offset + collision_attribute_index_count] = i;
				grid->cell_buffer[current_grid_cell_index].collision_attribute_index_count++;
			}
		}
	}
}

void lightray_set_entity_position(lightray_scene_t* scene, u32 entity_index, const sunder_v3_t& position)
{
	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return;
	}

	scene->position_buffer[entity_index] = position;
}

void lightray_set_entity_scale(lightray_scene_t* scene, u32 entity_index, const sunder_v3_t& scale)
{
	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return;
	}

	scene->scale_buffer[entity_index] = scale;

	if (scale != sunder_v3(1.0f, 1.0f, 1.0f))
	{
		SUNDER_SET_BIT(scene->entity_buffer[entity_index].flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
	}
}

void lightray_set_entity_rotation(lightray_scene_t* scene, u32 entity_index, const sunder_quat_t& rotation)
{
	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return;
	}

	scene->quat_rotation_buffer[entity_index] = rotation;

	if (rotation != sunder_quat_raw(0.0f, 0.0f, 0.0f, 1.0f))
	{
		SUNDER_SET_BIT(scene->entity_buffer[entity_index].flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
	}
}

void lightray_clear_entity_position(lightray_scene_t* scene, u32 entity_index)
{
	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return;
	}

	scene->position_buffer[entity_index] = sunder_v3_scalar(0.0f);
}

void lightray_clear_entity_scale(lightray_scene_t* scene, u32 entity_index)
{
	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return;
	}

	scene->scale_buffer[entity_index] = sunder_v3_scalar(1.0f);
	SUNDER_ZERO_BIT(scene->entity_buffer[entity_index].flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
}

void lightray_clear_entity_rotation(lightray_scene_t* scene, u32 entity_index)
{
	if (!sunder_valid_index(entity_index, scene->current_entity_count))
	{
		return;
	}

	scene->quat_rotation_buffer[entity_index] = sunder_quat_raw(0.0f, 0.0f, 0.0f, 1.0f);
	SUNDER_ZERO_BIT(scene->entity_buffer[entity_index].flags, LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT, 1u);
}

b32 lightray_aabb_in_grid_bounds(const lightray_grid_t* grid, const lightray_grid_cell_aabb_coordinates_t* row_coordinates, const lightray_grid_cell_aabb_coordinates_t* column_coordinates)
{
	return row_coordinates->min >= 0 && row_coordinates->max < (i32)grid->row_count && column_coordinates->min >= 0 && column_coordinates->max < (i32)grid->column_count;
}

void lightray_extract_transform(const sunder_m4_t& transform, sunder_v3_t* translation, sunder_quat_t* rotation, sunder_v3_t* scale, b32 non_unit_scale,  b32 non_default_rotation)
{
	if (!non_default_rotation)
	{
		if (non_unit_scale)
		{
			*scale = sunder_extract_scale_m4(transform);
		}

		else
		{
			scale->x = transform.x0;
			scale->y = transform.y1;
			scale->z = transform.z2;
		}

		*translation = sunder_extract_translation_m4(transform);
		*rotation = sunder_quat_raw(0.0f, 0.0f, 0.0f, 1.0f);
	}
	
	else
	{
		sunder_m4_t scaleless{};

		if (non_unit_scale)
		{
			*scale = sunder_extract_scale_m4(transform);
		}

		else
		{
			*scale = sunder_v3_scalar(1.0f);
		}

		*translation = sunder_extract_translation_m4(transform);
		scaleless = sunder_remove_scale_m4(transform);
		*rotation = sunder_extract_rotation_m4(scaleless);
	}
}

sunder_m4_t lightray_compute_inherited_transform(const sunder_m4_t& parent_transform, const sunder_v3_t& extracted_translation, const sunder_quat_t& extracted_rotation, const sunder_v3_t& extracted_scale, b32 inherits_translation, b32 inherits_rotation, b32 inherits_scale, b32* identity)
{
	sunder_m4_t temp_transform = parent_transform;

	*identity = SUNDER_FALSE;

	if (!inherits_rotation)
	{
		temp_transform = sunder_m4_identity();
		temp_transform = sunder_m4_ts(extracted_translation, extracted_scale);
	}

	if (!inherits_translation)
	{
		temp_transform.x3 = 0.0f;
		temp_transform.y3 = 0.0f;
		temp_transform.z3 = 0.0f;
	}

	if (!inherits_scale)
	{
		if (!inherits_rotation)
		{
			if (inherits_translation)
			{
				temp_transform = sunder_m4_translation(extracted_translation);
			}

			else
			{
				temp_transform = sunder_m4_identity();
				*identity = SUNDER_TRUE;
			}
		}

		else
		{
			if (inherits_translation)
			{
				temp_transform = sunder_m4_model(extracted_translation, extracted_rotation, sunder_v3_scalar(1.0f));
			}

			else
			{
				temp_transform = sunder_m4_model(sunder_v3_scalar(0.0f), extracted_rotation, sunder_v3_scalar(1.0f));
			}
		}
	}

	return temp_transform;
}

void lightray_cache_m4(lightray_buffer_m4_cache_t* m4_cache, sunder_bit_buffer_t* bit_buffer, const sunder_m4_t& inverse, u32 mesh_global_index)
{
	(*m4_cache)[mesh_global_index].row0.x = inverse.x0;
	(*m4_cache)[mesh_global_index].row0.y = inverse.x1;
	(*m4_cache)[mesh_global_index].row0.z = inverse.x2;
	(*m4_cache)[mesh_global_index].row0.w = inverse.x3;

	(*m4_cache)[mesh_global_index].row1.x = inverse.y0;
	(*m4_cache)[mesh_global_index].row1.y = inverse.y1;
	(*m4_cache)[mesh_global_index].row1.z = inverse.y2;
	(*m4_cache)[mesh_global_index].row1.w = inverse.y3;

	(*m4_cache)[mesh_global_index].row2.x = inverse.z0;
	(*m4_cache)[mesh_global_index].row2.y = inverse.z1;
	(*m4_cache)[mesh_global_index].row2.z = inverse.z2;
	(*m4_cache)[mesh_global_index].row2.w = inverse.z3;

	(*bit_buffer) = mesh_global_index;
}

sunder_m4_t lightray_reconstruct_cached_m4(const lightray_m4_cache_t* cache)
{
	return sunder_m4(cache->row0.x, cache->row0.y, cache->row0.z, cache->row0.w,
								cache->row1.x, cache->row1.y, cache->row1.z, cache->row1.w,
								cache->row2.x, cache->row2.y, cache->row2.z, cache->row2.w,
								0.0f,			  0.0f,			0.0f,			  1.0f);
}

sunder_v3_t lightray_project_capsule_onto_plane(lightray_scene_t* scene, lightray_grid_t* grid, u32 capsule_collision_attribute_index, const sunder_v3_t& pre_raycast_capsule_position, u32 cube_entity_index, const lightray_render_instance_t* render_instance_buffer)
{
	const u32 capsule_collision_mesh_batch_index = scene->collision_attribute_core.collision_attribute_buffer[capsule_collision_attribute_index].collision_mesh_batch_index;
	const u32 capsule_collision_mesh_global_index = scene->collision_attribute_core.collision_mesh_batch_buffer[capsule_collision_mesh_batch_index].collision_mesh_buffer_offset;
	const u32 capsule_collision_mesh_entity_index = scene->collision_attribute_core.collision_mesh_buffer[capsule_collision_mesh_global_index].entity_index;
	const u32 capsule_collision_mesh_entity_flags = scene->entity_buffer[capsule_collision_mesh_entity_index].flags;
	const b32 non_unit_scale = SUNDER_IS_ANY_BIT_SET(capsule_collision_mesh_entity_flags, LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT, 1u);
	const sunder_v3_t capsule_scale = scene->scale_buffer[capsule_collision_mesh_entity_index];

	const u32 capsule_beyond_lower_clipping_plane_sentinel_vertex_count = scene->capsule_beyond_lower_clipping_plane_sentinel_index_count;
	const u32 capsule_sentinel_local_space_vertex_position_buffer_offset = scene->capsule_sentinel_local_space_vertex_position_buffer_offset;
	const u32* capsule_beyond_lower_clipping_plane_sentinel_index_buffer = scene->capsule_beyond_lower_clipping_plane_sentinel_index_buffer;
	sunder_v3_t* capsule_sentinel_world_space_vertex_position_buffer = scene->collision_attribute_core.collision_mesh_buffer[capsule_collision_mesh_global_index].sentinel_world_space_vertex_position_buffer;

	if (non_unit_scale)
	{
		for (u32 v = 0; v < capsule_beyond_lower_clipping_plane_sentinel_vertex_count; v++)
		{
			const u32 vertex_index = capsule_beyond_lower_clipping_plane_sentinel_index_buffer[v];
			const sunder_v3_t ls = scene->sentinel_local_space_vertex_position_buffer[capsule_sentinel_local_space_vertex_position_buffer_offset + vertex_index];
			capsule_sentinel_world_space_vertex_position_buffer[v] = sunder_v3_world_space_ts(ls, pre_raycast_capsule_position, capsule_scale);
		}
	}

	else
	{
		for (u32 v = 0; v < capsule_beyond_lower_clipping_plane_sentinel_vertex_count; v++)
		{
			const u32 vertex_index = capsule_beyond_lower_clipping_plane_sentinel_index_buffer[v];
			const sunder_v3_t ls = scene->sentinel_local_space_vertex_position_buffer[capsule_sentinel_local_space_vertex_position_buffer_offset + vertex_index];
			capsule_sentinel_world_space_vertex_position_buffer[v] = sunder_v3_world_space_t(ls, pre_raycast_capsule_position);
		}
	}

	lightray_raycast_pierce_layer_t pierce_layer{};
	pierce_layer.layer = 0;
	pierce_layer.threshold = 0;

	u32 collision_layer_index = 1;

	lightray_raycast_data_t raycast_data{};
	raycast_data.grid = grid;
	raycast_data.pierce_layer_buffer.buffer = &pierce_layer;
	raycast_data.pierce_layer_buffer.size = 1;
	raycast_data.layer_bitmask = (1ull << 1);
	raycast_data.cull_layer_bitmask = 0;
	raycast_data.layer_index_buffer.buffer = &collision_layer_index;
	raycast_data.layer_index_buffer.size = 1;
	raycast_data.flags = (1u << LIGHTRAY_RAYCAST_BITS_IGNORE_DYNAMIC_COLLISION_ATTRIBUTES_BIT);
	raycast_data.cube_entity_index = cube_entity_index;
	raycast_data.intersection_mode = LIGHTRAY_RAYCAST_INTERSECTION_MODE_COLLISION_MESH;
	raycast_data.culling_mode = LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_BACKFACE;

	lightray_ray_t ray{};
	ray.distance = 100.0f;
	ray.direction = sunder_v3(0.0f, 0.0f, -1.0f);

	sunder_v3_t plane_intersection_point_buffer[19]{};
	sunder_v3_t ray_origin_buffer[19]{};
	u32 hit_index_buffer[19]{};
	u32 hit_count = 0;

	for (u32 i = 0; i < 19; i++)
	{
		ray_origin_buffer[i] = capsule_sentinel_world_space_vertex_position_buffer[i];
	}

	f32 closest_squared_distance = FLT_MAX;
	u32 closest_ray_index = 0;

	for (u32 i = 0; i < capsule_beyond_lower_clipping_plane_sentinel_vertex_count; i++)
	{
		const lightray_raycast_memory_reserve_result_t memory_reserve_result = lightray_reserve_raycast_memory(scene, 1, 1, LIGHTRAY_RAYCAST_INTERSECTION_MODE_COLLISION_MESH);
		raycast_data.memory_reserve_result = memory_reserve_result;
		ray.origin = ray_origin_buffer[i];
		raycast_data.ray = ray;

		const lightray_raycast_result_t raycast_result = lightray_cast_ray(scene, &raycast_data, render_instance_buffer);

		const sunder_v3_t intersection_point = scene->raycast_core.pierce_layer_test_data_subarena[memory_reserve_result.pierce_layer_test_data_subarena_offset].intersection_point;
		plane_intersection_point_buffer[i] = intersection_point;

		if (raycast_result.hit_any)
		{
			hit_index_buffer[hit_count] = i;
			hit_count++;
		}
	}

	if (hit_count == 0)
	{
		return sunder_v3_scalar(0.0f);
	}

	for (u32 i = 0; i < hit_count; i++)
	{
		const u32 hit_index = hit_index_buffer[i];
		const f32 squared_distance = sunder_squared_distance_v3(plane_intersection_point_buffer[hit_index], ray_origin_buffer[hit_index]);

		if (squared_distance < closest_squared_distance)
		{
			closest_squared_distance = squared_distance;
			closest_ray_index = i;
		}
	}
	
	const u32 cs = hit_index_buffer[closest_ray_index];
	const f32 distance = sunder_distance_v3(plane_intersection_point_buffer[cs], ray_origin_buffer[cs]) - LIGHTRAY_CAPSULE_ONTO_PLANE_PROJECTION_RAY_RECOVERY_EPSILON;

	sunder_v3_t new_capsule_position = pre_raycast_capsule_position;
	new_capsule_position.z -= distance;

	const u32 step_flags = 1u << LIGHTRAY_STEP_BITS_SOLVE_EPA_BIT;

	while (SUNDER_TRUE)
	{
		f32 penetration_depth = 0.0f;
		const b32 intersects = lightray_step(scene, grid, capsule_collision_attribute_index, new_capsule_position, step_flags, &penetration_depth);

		if (intersects)
		{
			new_capsule_position.z += (penetration_depth + LIGHTRAY_CAPSULE_ONTO_PLANE_POST_PROJECTION_PLACEMENT_RECOVERY_EPSILON);
		}

		else
		{
			SUNDER_TERMINATE;
		}
	}

	return new_capsule_position;
}