#pragma once

#include "snd_lib.h"
#define GLFW_STATIC
#include "glfw3.h"
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
#include "cJSON.h"

#define LIGHTRAY_LOG(a) std::cout << a
#define LIGHTRAY_WORLD_FORWARD_VECTOR 0.0f, 1.0f, 0.0f
#define LIGHTRAY_WORLD_RIGHT_VECTOR 1.0f, 0.0f, 0.0f
#define LIGHTRAY_WORLD_UP_VECTOR 0.0f, 0.0f, 1.0f
// based
#define plex struct

#define LIGHTRAY_INVALID_BONE_PARENT_INDEX 65790u
#define LIGHTRAY_INVALID_COMPUTED_BONE_TRANSFORM_MATRIX_BUFFER_OFFSET UINT16_MAX
#define LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT 900'000ull
#define LIGHTRAY_INVALID_NODE_BONE_BUFFER_INDEX 68790u
#define LIGHTRAY_INVALID_NODE_ANIMATION_CHANNEL_BUFFER_INDEX 72790u
#define LIGHTRAY_INVALID_NODE_PARENT_INDEX 76790u
#define LIGHTRAY_DEFAULT_ANIMATION_TICKRATE 24.0f
#define LIGHTRAY_MAIN_3D_CAMERA_INDEX 0u

#define LIGHTRAY_KEY_BOUND(key) ((key) != LIGHTRAY_KEY_BINDING_UNBOUND)

enum lightray_key_binding : i32
{
	LIGHTRAY_KEY_BINDING_UNBOUND = INT32_MAX,
	LIGHTRAY_KEY_BINDING_MOUSE_BUTTON_LEFT = 0,
	LIGHTRAY_KEY_BINDING_MOUSE_BUTTON_RIGHT = 1,
	LIGHTRAY_KEY_BINDING_MOUSE_BUTTON_MIDDLE = 2,
	LIGHTRAY_KEY_BINDING_SPACE = 32,
	LIGHTRAY_KEY_BINDING_APOSTROPHE = 39,
	LIGHTRAY_KEY_BINDING_COMMA = 44,
	LIGHTRAY_KEY_BINDING_MINUS = 45,
	LIGHTRAY_KEY_BINDING_PERIOD = 46,
	LIGHTRAY_KEY_BINDING_SLASH = 47,
	LIGHTRAY_KEY_BINDING_0 = 48,
	LIGHTRAY_KEY_BINDING_1 = 49,
	LIGHTRAY_KEY_BINDING_2 = 50,
	LIGHTRAY_KEY_BINDING_3 = 51,
	LIGHTRAY_KEY_BINDING_4 = 52,
	LIGHTRAY_KEY_BINDING_5 = 53,
	LIGHTRAY_KEY_BINDING_6 = 54,
	LIGHTRAY_KEY_BINDING_7 = 55,
	LIGHTRAY_KEY_BINDING_8 = 56,
	LIGHTRAY_KEY_BINDING_9 = 57,
	LIGHTRAY_KEY_BINDING_SEMICOLON = 59,
	LIGHTRAY_KEY_BINDING_EQUAL = 61,
	LIGHTRAY_KEY_BINDING_A = 65,
	LIGHTRAY_KEY_BINDING_B = 66,
	LIGHTRAY_KEY_BINDING_C = 67,
	LIGHTRAY_KEY_BINDING_D = 68,
	LIGHTRAY_KEY_BINDING_E = 69,
	LIGHTRAY_KEY_BINDING_F = 70,
	LIGHTRAY_KEY_BINDING_G = 71,
	LIGHTRAY_KEY_BINDING_H = 72,
	LIGHTRAY_KEY_BINDING_I = 73,
	LIGHTRAY_KEY_BINDING_J = 74,
	LIGHTRAY_KEY_BINDING_K = 75,
	LIGHTRAY_KEY_BINDING_L = 76,
	LIGHTRAY_KEY_BINDING_M = 77,
	LIGHTRAY_KEY_BINDING_N = 78,
	LIGHTRAY_KEY_BINDING_O = 79,
	LIGHTRAY_KEY_BINDING_P = 80,
	LIGHTRAY_KEY_BINDING_Q = 81,
	LIGHTRAY_KEY_BINDING_R = 82,
	LIGHTRAY_KEY_BINDING_S = 83,
	LIGHTRAY_KEY_BINDING_T = 84,
	LIGHTRAY_KEY_BINDING_U = 85,
	LIGHTRAY_KEY_BINDING_V = 86,
	LIGHTRAY_KEY_BINDING_W = 87,
	LIGHTRAY_KEY_BINDING_X = 88,
	LIGHTRAY_KEY_BINDING_Y = 89,
	LIGHTRAY_KEY_BINDING_Z = 90,
	LIGHTRAY_KEY_BINDING_LEFT_BRACKET = 91, 
	LIGHTRAY_KEY_BINDING_BACKSLASH = 92,
	LIGHTRAY_KEY_BINDING_RIGHT_BRACKET = 93, 
	LIGHTRAY_KEY_BINDING_GRAVE_ACCENT = 96,
	LIGHTRAY_KEY_BINDING_ESCAPE = 256,
	LIGHTRAY_KEY_BINDING_ENTER = 257,
	LIGHTRAY_KEY_BINDING_TAB = 258,
	LIGHTRAY_KEY_BINDING_BACKSPACE = 259,
	LIGHTRAY_KEY_BINDING_INSERT = 260,
	LIGHTRAY_KEY_BINDING_DELETE = 261,
	LIGHTRAY_KEY_BINDING_RIGHT = 262,
	LIGHTRAY_KEY_BINDING_LEFT = 263,
	LIGHTRAY_KEY_BINDING_DOWN = 264,
	LIGHTRAY_KEY_BINDING_UP = 265,
	LIGHTRAY_KEY_BINDING_PAGE_UP = 266,
	LIGHTRAY_KEY_BINDING_PAGE_DOWN = 267,
	LIGHTRAY_KEY_BINDING_HOME = 268,
	LIGHTRAY_KEY_BINDING_END = 269,
	LIGHTRAY_KEY_BINDING_CAPS_LOCK = 280,
	LIGHTRAY_KEY_BINDING_SCROLL_LOCK = 281,
	LIGHTRAY_KEY_BINDING_NUM_LOCK = 282,
	LIGHTRAY_KEY_BINDING_PRINT_SCREEN = 283,
	LIGHTRAY_KEY_BINDING_PAUSE = 284,
	LIGHTRAY_KEY_BINDING_F1 = 290,
	LIGHTRAY_KEY_BINDING_F2 = 291,
	LIGHTRAY_KEY_BINDING_F3 = 292,
	LIGHTRAY_KEY_BINDING_F4 = 293,
	LIGHTRAY_KEY_BINDING_F5 = 294,
	LIGHTRAY_KEY_BINDING_F6 = 295,
	LIGHTRAY_KEY_BINDING_F7 = 296,
	LIGHTRAY_KEY_BINDING_F8 = 297,
	LIGHTRAY_KEY_BINDING_F9 = 298,
	LIGHTRAY_KEY_BINDING_F10 = 299,
	LIGHTRAY_KEY_BINDING_F11 = 300,
	LIGHTRAY_KEY_BINDING_F12 = 301,
	LIGHTRAY_KEY_BINDING_F13 = 302,
	LIGHTRAY_KEY_BINDING_F14 = 303,
	LIGHTRAY_KEY_BINDING_F15 = 304,
	LIGHTRAY_KEY_BINDING_F16 = 305,
	LIGHTRAY_KEY_BINDING_F17 = 306,
	LIGHTRAY_KEY_BINDING_F18 = 307,
	LIGHTRAY_KEY_BINDING_F19 = 308,
	LIGHTRAY_KEY_BINDING_F20 = 309,
	LIGHTRAY_KEY_BINDING_F21 = 310,
	LIGHTRAY_KEY_BINDING_F22 = 311,
	LIGHTRAY_KEY_BINDING_F23 = 312,
	LIGHTRAY_KEY_BINDING_F24 = 313,
	LIGHTRAY_KEY_BINDING_F25 = 314,
	LIGHTRAY_KEY_BINDING_KP_0 = 320,
	LIGHTRAY_KEY_BINDING_KP_1 = 321,
	LIGHTRAY_KEY_BINDING_KP_2 = 322,
	LIGHTRAY_KEY_BINDING_KP_3 = 323,
	LIGHTRAY_KEY_BINDING_KP_4 = 324,
	LIGHTRAY_KEY_BINDING_KP_5 = 325,
	LIGHTRAY_KEY_BINDING_KP_6 = 326,
	LIGHTRAY_KEY_BINDING_KP_7 = 327,
	LIGHTRAY_KEY_BINDING_KP_8 = 328,
	LIGHTRAY_KEY_BINDING_KP_9 = 329,
	LIGHTRAY_KEY_BINDING_KP_DECIMAL = 330,
	LIGHTRAY_KEY_BINDING_KP_DIVIDE = 331,
	LIGHTRAY_KEY_BINDING_KP_MULTIPLY = 332,
	LIGHTRAY_KEY_BINDING_KP_SUBTRACT = 333,
	LIGHTRAY_KEY_BINDING_KP_ADD = 334,
	LIGHTRAY_KEY_BINDING_KP_ENTER = 335,
	LIGHTRAY_KEY_BINDING_KP_EQUAL = 336,
	LIGHTRAY_KEY_BINDING_LEFT_SHIFT = 340,
	LIGHTRAY_KEY_BINDING_LEFT_CONTROL = 341,
	LIGHTRAY_KEY_BINDING_LEFT_ALT = 342,
	LIGHTRAY_KEY_BINDING_LEFT_SUPER = 343,
	LIGHTRAY_KEY_BINDING_RIGHT_SHIFT = 344,
	LIGHTRAY_KEY_BINDING_RIGHT_CONTROL = 345,
	LIGHTRAY_KEY_BINDING_RIGHT_ALT = 346,
	LIGHTRAY_KEY_BINDING_RIGHT_SUPER = 347,
	LIGHTRAY_KEY_BINDING_MENU = 348
};

struct u128_t
{
	u64 high64 = 0;
	u64 low64 = 0;

	bool operator==(const u128_t& other) const
	{
		return (high64 == other.high64) && (low64 == other.low64);
	}
};

enum lightray_asset_kind : u32
{
	LIGHTRAY_ASSET_KIND_UNDEFINED = 0u,
	LIGHTRAY_ASSET_KIND_STATIC_MESH = 1u,
	LIGHTRAY_ASSET_KIND_SKELETAL_MESH = 2u,
	LIGHTRAY_ASSET_KIND_SHADER = 3u,
	LIGHTRAY_ASSET_KIND_TEXTURE = 4u,
	LIGHTRAY_ASSET_KIND_ANIMATION = 5u,
	LIGHTRAY_ASSET_KIND_SOUND = 6u
};

// add that as an argument to any function that accepts asset related indices to load, bind, play, apply, whatever the fuck 
enum lightray_asset_index_kind : u32
{
	LIGHTRAY_ASSET_INDEX_KIND_SETUP_TIME = 0u,
	LIGHTRAY_ASSET_INDEX_KIND_RUNTIME = 1u
};

struct lightray_runtime_asset_loading_indices_t
{
	u32 skeletal_mesh_index;
	u32 static_mesh_index;
	u32 texture_index;
	u32 shader_index;
	u32 animation_index;
	u32 sound_index;
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
	LIGHTRAY_RESULT_COUNT_OF_MESH_BINDINGS_FOR_SPECIFIED_MESH_HAS_BEEN_EXCEEDED = 9u,
	LIGHTRAY_RESULT_MESH_BINDING_OVERFLOW = 10u,
	LIGHTRAY_RESULT_FAILED_TO_OPEN_COMPILED_SHADER_BYTE_CODE = 11u,
	LIGHTRAY_RESULT_COMPILED_SHADER_BYTE_CODE_SIZE_LIMIT_HAS_BEEN_EXCEEDED = 12u,
	LIGHTRAY_RESULT_FAILED_TO_LOAD_COMPILED_SHADER_BYTE_CODE = 13u
};

enum lightray_render_target_kind : u32
{
	LIGHTRAY_RENDER_TARGET_KIND_UNDEFINED = 0u,
	LIGHTRAY_RENDER_TARGET_KIND_OPAQUE_MESH = 1u,
	LIGHTRAY_RENDER_TARGET_KIND_WIREFRAME_MESH = 2u 
};

enum lightray_bits : u64
{
	LIGHTRAY_BITS_KEY_CAN_BE_PRESSED_BIT = 0ull,
	LIGHTRAY_BITS_USER_CHOSEN_PRESENT_MODE_BIT = 1ull,
	LIGHTRAY_BITS_PRESENT_MODE_FIFO_SUPPORTED_BIT = 2ull,
	LIGHTRAY_BITS_PRESENT_MODE_MAILBOX_SUPPORTED_BIT = 3ull,
	LIGHTRAY_BITS_PRESENT_MODE_IMMEDIATE_SUPPORTED_BIT = 4ull,
	LIGHTRAY_BITS_FPS_CAPPED_BIT = 5ull
};

enum lightray_spatial_partitioning_grid_entity_kind : u32
{
	LIGHTRAY_SPATIAL_PARTITIONING_GRID_ENTITY_KIND_STATIC = 0u,
	LIGHTRAY_SPATIAL_PARTITIONING_GRID_ENTITY_KIND_DYNAMIC = 1u
};

enum lightray_animation_status : u32
{
	LIGHTRAY_ANIMATION_STATUS_BIND_POSE = 0u,
	LIGHTRAY_ANIMATION_STATUS_ACTIVE = 1u,
	LIGHTRAY_ANIMATION_STATUS_FROZEN = 2u
};

enum lightray_animation_bits : u32
{
	LIGHTRAY_ANIMATION_BITS_SHOULD_PLAY_BIT = 0u,
	LIGHTRAY_ANIMATION_BITS_SHOULD_RESTART_BIT = 1u
};

enum lightray_animation_playback_bits : u32
{
	LIGHTRAY_ANIMATION_PLAYBACK_BITS_LOOP_BIT = 0u
};

struct lightray_animation_playback_command_t
{
	u32 instance_index = 0;
	u32 skeletal_mesh_index = 0;
	u32 animation_index = 0;
	f32 scale = 0.0f;
	f32 time = 0.0f;
	f32 ticks = 0.0f;
};

struct lightray_spatial_partitioning_grid_t
{

};

struct lightray_vertex_t
{
	glm::vec3 position;
	f32 padding0;

	glm::vec3 normal;
	f32 padding1;

	glm::vec2 uv; // probably remove that
	glm::u8vec4 bone_indices;
	glm::u8vec4 weights;
};

struct lightray_model_t
{
	glm::mat4 model{};
};

struct lightray_render_instance_t
{
	lightray_model_t model{};
	f32 layer_index = 0.0f;
	u32 computed_bone_transform_matrix_buffer_offset_with_respect_to_instance = 0;
	// glm::vec2 uv; ???
};

struct lightray_orthographic_bounds_t
{
	f64 left;
	f64 right;
	f64 top;
	f64 bottom;
};

struct lightray_glyph_t
{
	f64 advance;
	lightray_orthographic_bounds_t plane_bounds;
	lightray_orthographic_bounds_t atlas_bounds;
	i32 index;
	bool has_bounds;
};

struct lightray_render_instance_glyph_t
{
	glm::mat4 model_matrix;
	glm::vec2 uv_min;
	glm::vec2 uv_max;
	f32 padding;
	glm::vec3 color;
};

struct lightray_font_atlas_t
{
	f64 ascender;
	f64 descender;
	f64 base_line_height;
	u32 width;
	u32 height;
	u32 glyph_count;
	u32 glyph_buffer_offset;
	u32 glyph_size;
	f32 em_size;
};

enum lightray_overlay_text_element_type : u32
{
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_U8 = 0u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_U16 = 1u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_U32 = 2u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_U64 = 3u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_I8 = 4u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_I16 = 5u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_I32 = 6u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_I64 = 7u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_F32 = 8u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_F64 = 9u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_B32 = 10,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_STRING = 11u
};

struct lightray_overlay_text_element_t
{
	const void* data = nullptr;
	u64 element_count = 0; // how many elements in 'data
	glm::vec2 position{};
	glm::vec3 color{};
	lightray_overlay_text_element_type type{};
	f32 glyph_size = 0; // in pixels
	u32 precision = 0; // only for floats
	// u32 sprite_index
};

struct lightray_overlay_core_t
{
	lightray_font_atlas_t* font_atlas_buffer;
	lightray_glyph_t* glyph_buffer;
	lightray_render_instance_glyph_t* render_instance_glyph_buffer;
	lightray_overlay_text_element_t* text_element_buffer;
	u32 total_text_element_count;
	u32 text_element_count;
	u32 total_font_atlas_count;
	u32 total_glyph_count;
	u32 current_glyph_render_instance_count;
};

// camera's view, projection matrices
struct lightray_cvp_t
{
	glm::mat4 view;
	glm::mat4 projection;
};

struct lightray_cursor_t
{
	glm::vec2 last_position;
	glm::vec2 current_position;
};

enum lightray_camera_projection_kind : u16
{
	LIGHTRAY_CAMERA_PROJECTION_KIND_UNDEFINED = 0,
	LIGHTRAY_CAMERA_PROJECTION_KIND_PERSPECTIVE = 1,
	LIGHTRAY_CAMERA_PROJECTION_KIND_ORTHOGRAPHIC = 2
};

struct lightray_camera_t
{
	glm::vec3 position{};
	glm::vec2 rotation{}; // yaw - x , pitch - y 
	f32 sensitivity = 0;
	f32 fov = 0;
	f32 near_clip_plane_distance = 0;
	f32 far_clip_plane_distance = 0;
	f32 left = 0.0f;
	f32 right = 0.0f;
	f32 bottom = 0.0f;
	f32 top = 0.0f;
	lightray_camera_projection_kind projection_kind = LIGHTRAY_CAMERA_PROJECTION_KIND_UNDEFINED;
	bool first_tick = true;
	bool vulkan_y_flip = true;
};

struct lightray_camera_initialization_data_t
{
	glm::vec3 position{};
	glm::vec2 rotation{}; // yaw - x , pitch - y 
	f32 sensitivity = 0;
	f32 fov = 0;
	f32 near_clip_plane_distance = 0;
	f32 far_clip_plane_distance = 0;
	f32 left = 0.0f;
	f32 right = 0.0f;
	f32 bottom = 0.0f;
	f32 top = 0.0f;
	u32 camera_index = 0;
	lightray_camera_projection_kind projection_kind = LIGHTRAY_CAMERA_PROJECTION_KIND_UNDEFINED;
	bool first_tick = true;
	bool vulkan_y_flip = true;
};

struct lightray_node_t
{
	u32 parent_index = 0; // relative
	u32 bone_buffer_index = 0; // relative
	u32 animation_channel_buffer_index = 0; // relative
	glm::mat4 local_transform_matrix{};
	glm::mat4 global_transform_matrix;
};

struct lightray_bone_t
{
	glm::mat4 inverse_bind_pose_matrix{};
};

struct lightray_animation_key_vec3_t
{
	glm::vec3 vec;
	f32 time;
};

struct lightray_animation_key_quat_t
{
	glm::quat quat;
	f32 time;
};

struct lightray_animation_channel_t
{
	u32 position_key_buffer_offset;
	u32 position_key_count;

	u32 rotation_key_buffer_offset;
	u32 rotation_key_count;

	u32 scale_key_buffer_offset;
	u32 scale_key_count;
};

struct lightray_animation_t
{
	u32 channel_buffer_offset;
	u32 channel_count;
	f32 tickrate;
	f32 duration;
};

struct lightray_skeleton_t
{
	u32 computed_bone_transform_matrix_buffer_offset;
	u32 computed_bone_transform_matrix_buffer_bone_count;

	u32 bone_buffer_offset;
	u32 bone_count;

	u32 node_buffer_offset;
	u32 node_count;

	u32 instance_count;
};

struct lightray_animation_core_t
{
	lightray_animation_playback_command_t* playback_command_buffer;
	u32 total_playback_command_count;
	u32 playback_command_count;
	u32 total_skeletal_mesh_instance_count;
	u64 playback_flags; // 1 bit per skeletal mesh / layer maybe change that to an array depending on skeletal mesh instance count
	u64 looped_playback_flags;  // 1 bit per skeletal mesh / layer maybe change that to an array depending on skeletal mesh instance count

	lightray_node_t* node_buffer;
	u32 total_node_count;

	sunder_string_t* node_names;
	sunder_string_t* animation_channel_names;
	sunder_string_t* bone_names;

	glm::mat4* global_root_inverse_matrix_buffer;
	glm::mat4* bone_bind_pose_matrix_buffer;
	glm::mat4* computed_bone_matrix_buffer; // sent to the gpu in a storage buffer
	u32 largest_skeleton_bone_count; // obsolete
	u32 total_computed_bone_transform_matrix_buffer_bone_count;

	lightray_bone_t* bone_buffer; // change that to just inverse bind pose matrix buffer
	lightray_skeleton_t* skeleton_buffer;
	u32 total_bone_count;
	u32 total_skeleton_count;

	lightray_animation_key_vec3_t* animation_position_key_buffer;
	lightray_animation_key_quat_t* animation_rotation_key_buffer;
	lightray_animation_key_vec3_t* animation_scale_key_buffer;
	u32 total_animation_position_key_count;
	u32 total_animation_rotation_key_count;
	u32 total_animation_scale_key_count;

	lightray_animation_channel_t* animation_channel_buffer;
	lightray_animation_t* animation_buffer;
	u32 total_animation_channel_count;
	u32 total_animation_count;
};

struct lightray_ray_t
{
	glm::vec3 origin{};
	glm::vec3 direction{};
	f32 distance = 0;
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

// rename this to entity_core_t
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
	lightray_mesh_binding_metadata_t* mesh_binding_metadata_buffer = nullptr; // per mesh
	u32 mesh_binding_count = 0;
	u64 visibility_flags = 0; // 1 bit per entity / should be an array of u64
};

struct lightray_console_t
{
	char* buffer = nullptr;
	u32 current_size = 0;
	u32 current_index = 0;
	u32 capacity = 0;
};

/*
struct lightray_world_view_t
{
};

lightray_result										lightray_load_world_view();
lightray_result										lightray_push_world_view();
lightray_result										lightray_flush_world_view();
lightray_result										lightray_save_world_view();
lightray_result										lightray_borrow_world_view(borrow_data); // this is called before the push to preserve ("borrow") some data from the previously pushed world view / maybe should rename this to "preserve_world_view"
lightray_result										lightray_flush_world_view_borrow_buffer();
u64														lightray_compute_world_view_cache_suballocation_size();
lightray_result										lightray_suballocate_world_view_cache();
lightray_result										lightray_cache_world_view();
lightray_result										lightray_invalidate_world_view();

*/

lightray_result										lightray_load_shader_byte_code(cstring_literal* path, i8* buffer, u64* buffer_size_in_bytes, u64 byte_code_size_limit);

															// returns UINT64_MAX on failure
u64														lightray_get_shader_byte_code_size(cstring_literal* path);
void														lightray_hide_cursor(GLFWwindow* window);
void														lightray_unhide_cursor(GLFWwindow* window);
bool														lightray_should_tick(GLFWwindow* window);
bool														lightray_key_pressed(GLFWwindow* window, lightray_key_binding key, u8* key_tick_data);
bool														lightray_key_down(GLFWwindow* window, lightray_key_binding key);
bool														lightray_mouse_button_pressed(GLFWwindow* window, lightray_key_binding mouse_button, u8* key_tick_data);
glm::mat4												lightray_construct_perspective_projection_matrix(f32 desired_fov, f32 aspect_ratio, f32 near_plane, f32 far_plane, bool vulkan_y_flip);
glm::mat4												lightray_construct_orthographic_projection_matrix(f32 left, f32 right, f32 bottom, f32 top, f32 near_plane, f32 far_plane, bool vulkan_y_flip);
f32														lightray_compute_aspect_ratio(f32 width, f32 height);
glm::vec3												lightray_to_world_forward_from_euler(const glm::vec3& euler_radians, const glm::vec3& local_forward);
glm::vec3												lightray_to_world_forward_from_model(const glm::mat4& model_matrix, const glm::vec3& local_forward);
glm::vec3												lightray_get_camera_forward(const glm::mat4& camera_view_matrix);
glm::vec3												lightray_get_camera_right(const glm::mat4& camera_view_matrix);
bool														lightray_ray_triangle_intersect(const lightray_ray_t* ray, const glm::vec3* triangle_vertices, f32* t, f32* u, f32* v);
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
u32														lightray_assimp_get_mesh_index_count_unsafe(const aiMesh* mesh);

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
void														lightray_log_mesh_binding_offset_buffer(const lightray_scene_t* scene);
void														lightray_log_mesh_binding_metadata_buffer(const lightray_scene_t* scene);
void														lightray_log_mesh_binding_buffer(const lightray_scene_t* scene);

															// 0.45 -> 115
u8															lightray_pack_f32_to_u8(f32 f);
															// 115 -> 0.45 with some garbage
f32														lightray_unpack_u8_to_f32(u8 v);

glm::vec3												lightray_assimp_to_glm_vec3(const aiVector3D& vec);
glm::quat												lightray_assimp_to_glm_quat(const aiQuaternion& quat);

void														lightray_log_animation_playback_frame_time(f32 current_frame_time, f32 animation_duration);
u32														lightray_get_suitable_scale_key_index(const lightray_animation_key_vec3_t* scale_key_buffer, u32 scale_key_buffer_offset, u32 scale_key_count, f32 animation_in_ticks);
u32														lightray_get_suitable_rotation_key_index(const lightray_animation_key_quat_t* rotation_key_buffer, u32 rotation_key_buffer_offset, u32 rotation_key_count, f32 animation_in_ticks);
u32														lightray_get_suitable_position_key_index(const lightray_animation_key_vec3_t* position_key_buffer, u32 position_key_buffer_offset, u32 position_key_count, f32 animation_in_ticks);
glm::vec3												lightray_compute_interpolated_animation_channel_scale_key(const lightray_animation_channel_t* channel, const lightray_animation_key_vec3_t* scale_key_buffer, f32 animation_in_ticks);
glm::quat												lightray_compute_interpolated_animation_channel_rotation_key(const lightray_animation_channel_t* channel, const lightray_animation_key_quat_t* rotation_key_buffer, f32 animation_in_ticks);
glm::vec3												lightray_compute_interpolated_animation_channel_position_key(const lightray_animation_channel_t* channel, const lightray_animation_key_vec3_t* position_key_buffer, f32 animation_in_ticks);
void														lightray_compute_interpolated_skeleton_transform(lightray_animation_core_t* animation_core, u32 animation_index, u32 skeleton_index, u32 instance_index);

u32														lightray_compute_skeletal_mesh_bone_count_with_respect_to_instance_count(u32 bone_count, u32 instance_count);
u32														lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(u32 instance_index, u32 bone_count, u32 base_offset);

void														lightray_assimp_get_node_hierarchy_metadata(const aiNode* node, u32* total_node_count, u64* aligned_name_byte_code_size, u32 alignment);
void														lightray_populate_node_related_string_upon_suballocation(sunder_arena_t* arena, sunder_string_t* host, cstring_literal* string, u32 length);
void														lightray_assimp_execute_first_node_buffer_population_pass(const aiNode* node, lightray_node_t* node_buffer, sunder_arena_t* arena, sunder_string_t* names, u32* current_index);
void														lightray_assimp_execute_second_node_buffer_population_pass(const aiNode* node, lightray_node_t* node_buffer, u32 node_count, const sunder_string_t* names, u32* current_index);

u32														lightray_get_skeletal_mesh_global_mesh_index(u32 skeletal_mesh_index, u32 static_mesh_count);
glm::vec2												lightray_get_cursor_position(GLFWwindow* window);
void														lightray_load_json_garbage();
i32														lightray_get_file_size(cstring_literal* path);