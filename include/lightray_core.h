#pragma once

#include "snd_lib.h"
#define GLFW_STATIC
#include "glfw3.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "glfw3native.h"
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <fstream>
#include <thread>
#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/version.h"
#include <intrin.h>
#include <immintrin.h>

#define LIGHTRAY_KEY_SPACE              32
#define LIGHTRAY_KEY_APOSTROPHE    39  /* ' */
#define LIGHTRAY_KEY_COMMA              44  /* , */
#define LIGHTRAY_KEY_MINUS              45  /* - */
#define LIGHTRAY_KEY_PERIOD             46  /* . */
#define LIGHTRAY_KEY_SLASH              47  /* / */
#define LIGHTRAY_KEY_0                  48
#define LIGHTRAY_KEY_1                  49
#define LIGHTRAY_KEY_2                  50
#define LIGHTRAY_KEY_3                  51
#define LIGHTRAY_KEY_4                  52
#define LIGHTRAY_KEY_5                  53
#define LIGHTRAY_KEY_6                  54
#define LIGHTRAY_KEY_7                  55
#define LIGHTRAY_KEY_8                  56
#define LIGHTRAY_KEY_9                  57
#define LIGHTRAY_KEY_SEMICOLON          59  /* ; */
#define LIGHTRAY_KEY_EQUAL              61  /* = */
#define LIGHTRAY_KEY_A                  65
#define LIGHTRAY_KEY_B                  66
#define LIGHTRAY_KEY_C                  67
#define LIGHTRAY_KEY_D                  68
#define LIGHTRAY_KEY_E                  69
#define LIGHTRAY_KEY_F                  70
#define LIGHTRAY_KEY_G                  71
#define LIGHTRAY_KEY_H                  72
#define LIGHTRAY_KEY_I                  73
#define LIGHTRAY_KEY_J                  74
#define LIGHTRAY_KEY_K                  75
#define LIGHTRAY_KEY_L                  76
#define LIGHTRAY_KEY_M                  77
#define LIGHTRAY_KEY_N                  78
#define LIGHTRAY_KEY_O                  79
#define LIGHTRAY_KEY_P                  80
#define LIGHTRAY_KEY_Q                  81
#define LIGHTRAY_KEY_R                  82
#define LIGHTRAY_KEY_S                  83
#define LIGHTRAY_KEY_T                  84
#define LIGHTRAY_KEY_U                  85
#define LIGHTRAY_KEY_V                  86
#define LIGHTRAY_KEY_W                  87
#define LIGHTRAY_KEY_X                  88
#define LIGHTRAY_KEY_Y                  89
#define LIGHTRAY_KEY_Z                  90
#define LIGHTRAY_KEY_LEFT_BRACKET       91  /* [ */
#define LIGHTRAY_KEY_BACKSLASH          92  /* \ */
#define LIGHTRAY_KEY_RIGHT_BRACKET      93  /* ] */
#define LIGHTRAY_KEY_GRAVE_ACCENT       96  /* ` */
#define LIGHTRAY_KEY_WORLD_1            161 /* non-US #1 */
#define LIGHTRAY_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define LIGHTRAY_KEY_ESCAPE             256
#define LIGHTRAY_KEY_ENTER              257
#define LIGHTRAY_KEY_TAB                258
#define LIGHTRAY_KEY_BACKSPACE          259
#define LIGHTRAY_KEY_INSERT             260
#define LIGHTRAY_KEY_DELETE             261
#define LIGHTRAY_KEY_RIGHT              262
#define LIGHTRAY_KEY_LEFT               263
#define LIGHTRAY_KEY_DOWN               264
#define LIGHTRAY_KEY_UP                 265
#define LIGHTRAY_KEY_PAGE_UP            266
#define LIGHTRAY_KEY_PAGE_DOWN          267
#define LIGHTRAY_KEY_HOME               268
#define LIGHTRAY_KEY_END                269
#define LIGHTRAY_KEY_CAPS_LOCK          280
#define LIGHTRAY_KEY_SCROLL_LOCK        281
#define LIGHTRAY_KEY_NUM_LOCK           282
#define LIGHTRAY_KEY_PRINT_SCREEN       283
#define LIGHTRAY_KEY_PAUSE              284
#define LIGHTRAY_KEY_F1                 290
#define LIGHTRAY_KEY_F2                 291
#define LIGHTRAY_KEY_F3                 292
#define LIGHTRAY_KEY_F4                 293
#define LIGHTRAY_KEY_F5                 294
#define LIGHTRAY_KEY_F6                 295
#define LIGHTRAY_KEY_F7                 296
#define LIGHTRAY_KEY_F8                 297
#define LIGHTRAY_KEY_F9                 298
#define LIGHTRAY_KEY_F10                299
#define LIGHTRAY_KEY_F11                300
#define LIGHTRAY_KEY_F12                301
#define LIGHTRAY_KEY_F13                302
#define LIGHTRAY_KEY_F14                303
#define LIGHTRAY_KEY_F15                304
#define LIGHTRAY_KEY_F16                305
#define LIGHTRAY_KEY_F17                306
#define LIGHTRAY_KEY_F18                307
#define LIGHTRAY_KEY_F19                308
#define LIGHTRAY_KEY_F20                309
#define LIGHTRAY_KEY_F21                310
#define LIGHTRAY_KEY_F22                311
#define LIGHTRAY_KEY_F23                312
#define LIGHTRAY_KEY_F24                313
#define LIGHTRAY_KEY_F25                314
#define LIGHTRAY_KEY_KP_0               320
#define LIGHTRAY_KEY_KP_1               321
#define LIGHTRAY_KEY_KP_2               322
#define LIGHTRAY_KEY_KP_3               323
#define LIGHTRAY_KEY_KP_4               324
#define LIGHTRAY_KEY_KP_5               325
#define LIGHTRAY_KEY_KP_6               326
#define LIGHTRAY_KEY_KP_7               327
#define LIGHTRAY_KEY_KP_8               328
#define LIGHTRAY_KEY_KP_9               329
#define LIGHTRAY_KEY_KP_DECIMAL         330
#define LIGHTRAY_KEY_KP_DIVIDE          331
#define LIGHTRAY_KEY_KP_MULTIPLY        332
#define LIGHTRAY_KEY_KP_SUBTRACT        333
#define LIGHTRAY_KEY_KP_ADD             334
#define LIGHTRAY_KEY_KP_ENTER           335
#define LIGHTRAY_KEY_KP_EQUAL           336
#define LIGHTRAY_KEY_LEFT_SHIFT         340
#define LIGHTRAY_KEY_LEFT_CONTROL       341
#define LIGHTRAY_KEY_LEFT_ALT           342
#define LIGHTRAY_KEY_LEFT_SUPER         343
#define LIGHTRAY_KEY_RIGHT_SHIFT        344
#define LIGHTRAY_KEY_RIGHT_CONTROL      345
#define LIGHTRAY_KEY_RIGHT_ALT          346
#define LIGHTRAY_KEY_RIGHT_SUPER        347
#define LIGHTRAY_KEY_MENU               348

#define LIGHTRAY_MOUSE_BUTTON_1         0
#define LIGHTRAY_MOUSE_BUTTON_2         1
#define LIGHTRAY_MOUSE_BUTTON_3         2
#define LIGHTRAY_MOUSE_BUTTON_4         3
#define LIGHTRAY_MOUSE_BUTTON_5         4
#define LIGHTRAY_MOUSE_BUTTON_6         5
#define LIGHTRAY_MOUSE_BUTTON_7         6
#define LIGHTRAY_MOUSE_BUTTON_8         7
#define LIGHTRAY_MOUSE_BUTTON_LAST      LIGHTRAY_MOUSE_BUTTON_8
#define LIGHTRAY_MOUSE_BUTTON_LEFT      LIGHTRAY_MOUSE_BUTTON_1
#define LIGHTRAY_MOUSE_BUTTON_RIGHT     LIGHTRAY_MOUSE_BUTTON_2
#define LIGHTRAY_MOUSE_BUTTON_MIDDLE    LIGHTRAY_MOUSE_BUTTON_3

#define LIGHTRAY_LOG(a) std::cout << a
#define LIGHTRAY_WORLD_FORWARD_VECTOR 0.0f, 1.0f, 0.0f
#define LIGHTRAY_WORLD_RIGHT_VECTOR 1.0f, 0.0f, 0.0f
#define LIGHTRAY_WORLD_UP_VECTOR 0.0f, 0.0f, 1.0f
// based
#define plex struct

struct u128_t
{
	u64 high64 = 0;
	u64 low64 = 0;

	bool operator==(const u128_t& other) const
	{
		return (high64 == other.high64) && (low64 == other.low64);
	}
};

enum lightray_result : u32
{
	LIGHTRAY_RESULT_SUCCESS = 0u,
	LIGHTRAY_RESULT_FAILURE = 1u,
	LIGHTRAY_RESULT_INVALID_MESH_INDEX = 3u,
	LIGHTRAY_RESULT_INVALID_ENTITY_INDEX = 4u,
	LIGHTRAY_RESULT_ENTITY_BUFFER_OVERFLOW = 5u,
	LIGHTRAY_RESULT_TOTAL_ENTITY_COUNT_IS_ZERO = 6u,
	LIGHTRAY_RESULT_COLLISION_MESH_COUNT_OF_SPECIFIED_MESH_IS_ZERO = 7u,
	LIGHTRAY_RESULT_INSTANCE_COUNT_OF_SPECIFIED_MESH_IS_ZERO = 8u,
	LIGHTRAY_RESULT_COUNT_OF_MESH_BINDINGS_FOR_SPECIFIED_MESH_HAS_BEEN_EXCEEDED = 9u
};


enum lightray_render_target_kind : u32
{
	LIGHTRAY_RENDER_TARGET_KIND_UNDEFINED = 0u,
	LIGHTRAY_RENDER_TARGET_KIND_OPAQUE_MESH = 1u,
	LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH = 2u 
};

enum lightray_bits : u64
{
	LIGHTRAY_BITS_KEY_CAN_BE_PRESSED = 0ull,
	LIGHTRAY_BITS_USER_CHOSEN_PRESENT_MODE_BIT = 1ull,
	LIGHTRAY_BITS_PRESENT_MODE_FIFO_SUPPORTED_BIT = 2ull,
	LIGHTRAY_BITS_PRESENT_MODE_MAILBOX_SUPPORTED_BIT = 3ull,
	LIGHTRAY_BITS_PRESENT_MODE_IMMEDIATE_SUPPORTED_BIT = 4ull,
	LIGHTRAY_BITS_IS_FPS_CAPPED = 5ull
};

enum lightray_spg_entity_kind : u32
{
	LIGHTRAY_SPG_ENTITY_KIND_STATIC = 0u,
	LIGHTRAY_SPG_ENTITY_KIND_DYNAMIC = 1u
};

// spatial partitioning grid
struct lightray_spg_t
{

};

struct lightray_vertex_t
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texture_coordinates;
	f32 alpha;
};

struct lightray_model_t
{
	glm::mat4 model{};
};

struct alignas (16) lightray_render_instance_t
{
	lightray_model_t model{};
	f32 layer_index = 0.0f;
};

// camera's view, projection matrices
struct lightray_cvp_t
{
	glm::mat4 view;
	glm::mat4 projection;
};

struct lightray_cursor_t
{
	glm::dvec2 last_position;
	glm::dvec2 current_position;
};

struct lightray_camera_t
{
	glm::vec3 position;
	glm::vec2 rotation; // yaw - x , pitch - y 
	f32 sensetivity;
	f32 near_plane;
	f32 far_plane;
	f32 fov;
	bool first_camera_tick;
};

struct lightray_ray_t
{
	glm::vec3 origin{};
	glm::vec3 direction{};
	f32 distance = 0;
};

struct lightray_buffer_index_query_result_t
{
	u32 value_at_index = UINT32_MAX;
	u32 return_index = UINT32_MAX;
};

enum lightray_undefined_value : u32
{
	LIGHTRAY_UNDEFINED_VALUE_ENTITY_BINDING_INDEX = UINT16_MAX,
	LIGHTRAY_UNDEFINED_VALUE_MESH_BINDING_INDEX,
	LIGHTRAY_UNDEFINED_VALUE_INSTANCE_MODEL_BINDING_INDEX,
	LIGHTRAY_UNDEFINED_VALUE_CONVERTED_INSTANCE_MODEL_BUFFER_INDEX,
	LIGHTRAY_UNDEFINED_VALUE_AABB_INDEX,
	LIGHTRAY_UNDEFINED_VALUE_COLLISION_MESH_INDEX
};

struct lightray_entity_creation_result_t
{
	lightray_result result = LIGHTRAY_RESULT_FAILURE;
	u32 index = 0;
};

enum lightray_entity_kind : u32
{
	LIGHTRAY_ENTITY_KIND_UNDEFINED = 0u,
	LIGHTRAY_ENTITY_KIND_STATIC_MESH = 1u,
	LIGHTRAY_ENTITY_KIND_SKELETAL_MESH = 2u,
	LIGHTRAY_ENTITY_KIND_COLLISION_MESH = 3u,
	LIGHTRAY_ENTITY_KIND_AABB = 4u
};

enum lightray_entity_bits : u32
{
	LIGHTRAY_ENTITY_BITS_IS_COLLIDING = 0u
};

struct lightray_entity_t
{
	u128_t guid;
	lightray_entity_kind kind;
	u32 mesh_binding_index;
	u32 instance_model_binding_index;
	u32 entity_binding_index;
	u32 flags;
};

struct lightray_scene_suballocation_data_t
{
	u32 total_instance_model_count = 0;
	u32 total_entity_count = 0;
	u32 total_mesh_count = 0;

	sunder_arena_t* arena = nullptr;
};

struct lightray_mesh_binding_t
{
	u32 transform_index = 0;
	u32 instance_model_index = 0;
};

struct lightray_mesh_binding_offsets_t
{
	u32 current_opaque_instance_model_index = 0;
	u32 last_opaque_instance_model_index = 0;
	u32 current_wireframe_instance_model_index = 0;
	u32 last_wireframe_instance_model_index = 0;
};

struct lightray_mesh_binding_metadata_t
{
	u32 instance_count = 0;
	u32 current_binding_count = 0;
};

struct lightray_scene_t
{
	u32 total_instance_model_count = 0;
	u32 total_mesh_count = 0;
	u32 current_entity_count = 0;
	u32 total_entity_count = 0;

	lightray_entity_t* entity_buffer = nullptr;
	glm::vec3* position_buffer = nullptr;
	glm::vec3* rotation_buffer = nullptr;
	glm::vec3* scale_buffer = nullptr;

	lightray_mesh_binding_offsets_t* mesh_binding_offsets = nullptr; // per mesh
	lightray_mesh_binding_t* mesh_binding_buffer = nullptr; // per instance model
	lightray_mesh_binding_metadata_t* mesh_binding_metadata_buffer = nullptr;
	u32 mesh_binding_count = 0;
	u64 visibility_flags = 0; // 1 bit per instance model
};

void														lightray_get_shader_byte_code(cstring_literal* path, i8* buffer, u64* buffer_size_in_bytes, u64 byte_code_size_limit);

															// returns UINT64_MAX on failure
u64														lightray_get_shader_byte_code_size(cstring_literal* path);
std::filesystem::path								lightray_get_project_root_path();
void														lightray_hide_cursor(GLFWwindow* window);
void														lightray_unhide_cursor(GLFWwindow* window);
bool														lightray_should_tick(GLFWwindow* window);
bool														lightray_is_key_pressed(GLFWwindow* window, i32 key, u8* key_tick_data);
bool														lightray_is_key_down(GLFWwindow* window, i32 key);
bool														lightray_is_mouse_button_pressed(GLFWwindow* window, i32 mouse_button, u8* key_tick_data);
glm::mat4												lightray_construct_projection_matrix(f32 desired_fov, f32 aspect_ratio, f32 near_plane, f32 far_plane);
f32														lightray_compute_aspect_ratio(f32 width, f32 height);
glm::vec3												lightray_to_world_forward_from_euler(const glm::vec3& euler_radians, const glm::vec3& local_forward);
glm::vec3												lightray_to_world_forward_from_model(const glm::mat4& model_matrix, const glm::vec3& local_forward);
glm::vec3												lightray_get_camera_forward(const glm::mat4& camera_view_matrix);
glm::vec3												lightray_get_camera_right(const glm::mat4& camera_view_matrix);
bool														lightray_ray_triangle_intersect(const lightray_ray_t* ray, const glm::vec3* triangle_vertices, f32* t, f32* u, f32* v);
lightray_buffer_index_query_result_t	lightray_query_buffer_index(const u32* buffer, u32 starting_index, u32 ending_index, u32 val_at_index, bool reverse_logic);
bool														lightray_aabbs_intersect(const glm::vec3& position_a, const glm::vec3& scale_a, const glm::vec3& position_b, const glm::vec3& scale_b);
void														lightray_get_raw_vertex_positions(u32 index_buffer_offset, u32 index_count, glm::vec3* buffer, const lightray_vertex_t* vertex_buffer, const u32* index_buffer);
				
															// takes raw vertex positions and computes their positions in model space (expensive, use one of vectorized version) 
void														lightray_compute_projected_vertex_positions(const glm::vec3* raw_vertex_positions, glm::vec3* projected_vertex_positions, u64 vertex_count, const lightray_model_t* model);

glm::vec3												lightray_gjk_find_furthest_point(const glm::vec3* tri, u64 vertex_count, const glm::vec3& direction);
glm::vec3												lightray_gjk_support(const glm::vec3* vertex_positions1, u64 vertex_count1, const glm::vec3* vertex_positions2, u64 vertex_count2, const glm::vec3& direction);
bool														lightray_gjk_same_direction(const glm::vec3& direction, const glm::vec3& ao);
bool														lightray_gjk_line(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction);
bool														lightray_gjk_triangle(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction);
bool														lightray_gjk_tetrahedron(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction);
bool														lightray_gjk_next_simplex(glm::vec3* simplex, u32* simplex_size, glm::vec3* direction);
bool														lightray_gjk_intersect(const glm::vec3* vertex_positions1, u64 vertex_count1, const glm::vec3* vertex_positions2, u64 vertex_count2);

glm::mat4												lightray_assimp_to_glm_mat4(const aiMatrix4x4& mat);
u32														lightray_assimp_get_mesh_index_count(const aiMesh* mesh);
void														lightray_compute_interpolated_bone_positions(aiNodeAnim* animation_channel_buffer, const glm::mat4& parent_transform, glm::mat4* bone_offset_matrix_buffer, f32 delta_time, f64 duration);

u64														lightray_generate_guid64();
u128_t													lightray_generate_guid128();
bool														lightray_raw_mouse_input_supported();
void														lightray_enable_raw_mouse_input(GLFWwindow* window);
void														lightray_disable_raw_mouse_input(GLFWwindow* window);
void														lightray_set_target_fps(f32 desired_fps, f32* frame_duration_s, f32* core_fps);

															// dont need to deallocate anything, because this function just suballocates from specified arena
void														lightray_suballocate_scene(lightray_scene_t* scene, const lightray_scene_suballocation_data_t* suballocation_data);
u64														lightray_compute_scene_suballocation_size(const lightray_scene_suballocation_data_t* suballocation_data, u64 alignment);
lightray_entity_creation_result_t			lightray_create_entity(lightray_scene_t* scene, lightray_entity_kind kind);
lightray_result										lightray_bind_mesh(lightray_scene_t* scene, u32 entity_index, u32 mesh_index, lightray_render_target_kind kind);
void														lightray_hide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene);
void														lightray_unhide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene);
void														lightray_bind_entity(lightray_scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index); // change it so that until mesh's instance is bound to any entity, it wouldn't be rendered
void														lightray_move_entity(lightray_scene_t* scene, u32 self_aabb_index, u32 self_collision_mesh_index, const u32* index_buffer_offsets, const u32* index_count_buffer, const u32* index_buffer, const lightray_vertex_t* vertex_buffer, lightray_model_t* instance_model_buffer, glm::vec3* self_raw_vertex_positions, glm::vec3* self_projected_vertex_positions, glm::vec3* other_raw_vertex_positions, glm::vec3* other_projected_vertex_positions, u32 entity_index, const glm::vec3& direction);
u32														lightray_get_entity_bound_collision_mesh_index_count(const lightray_scene_t* scene, u32 entity_index);
void														lightray_get_entity_bound_collision_mesh_indices(const lightray_scene_t* scene, u32 entity_index, u32* collision_mesh_index_buffer);
u32														lightray_get_entity_bound_aabb_index_count(const lightray_scene_t* scene, u32 collision_mesh_index);
void														lightray_get_entity_bound_aabb_indices(const lightray_scene_t* scene, u32 collision_mesh_index, u32* aabb_index_buffer);
void														lightray_bind_collision_layer();
void														lightray_unbind_collision_layer();
void														lightray_bind_texture();