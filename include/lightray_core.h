#pragma once

#include "snd_lib_math.h"
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

// units are in meters (directional_vector(1.0f, 0.0f, 0.0f) * delta_time * speed_in_meters);
// spatial_vector_in_meters(2.0f, 0.0f, 1.6f);


#define LIGHTRAY_LOG(a) std::cout << a
#define LIGHTRAY_WORLD_FORWARD_VECTOR 1.0f, 0.0f, 0.0f
#define LIGHTRAY_WORLD_RIGHT_VECTOR 0.0f, -1.0f, 0.0f
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

#define LIGHTRAY_DEFAULT_CUBE_SCALE_FACTOR 0.1996f

#define LIGHTRAY_CAPSULE_BEYOND_LOWER_CLIPPING_PLANE_VERTEX_PLACEMENT_EPSILON 0.00001f
#define LIGHTRAY_CAPSULE_ONTO_PLANE_PROJECTION_RAY_RECOVERY_EPSILON 0.00001f
#define LIGHTRAY_CAPSULE_ONTO_PLANE_POST_PROJECTION_PLACEMENT_RECOVERY_EPSILON 0.006f

#define LIGHTRAY_MAX_TRACE_STEP_COUNT 8u

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

struct lightray_trace_render_vertex_t
{
	sunder_v3_t normal{};
	f32 padding1 = 0.0f;

	sunder_v2_t uv{};
	u32 trace_render_vertex_index = 0; // relative
};

struct lightray_trace_render_instance_t
{
	sunder_v4_t vertex_color{};
	u32 trace_render_vertex_position_buffer_offset = 0;
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
	LIGHTRAY_RESULT_FAILED_TO_LOAD_COMPILED_SHADER_BYTE_CODE = 13u,
	LIGHTRAY_RESULT_WIREFRAME_MESH_BINDING_COUNT_HAS_BEEN_EXCEEDED = 14u,
	LIGHTRAY_RESULT_OPAQUE_MESH_BINDING_COUNT_HAS_BEEN_EXCEEDED = 15u
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

enum lightray_grid_entity_kind : u32
{
	LIGHTRAY_GRID_ENTITY_KIND_STATIC = 0u,
	LIGHTRAY_GRID_ENTITY_KIND_DYNAMIC = 1u
};

enum lightray_animation_playback_bits : u16
{
	LIGHTRAY_ANIMATION_PLAYBACK_BITS_CULL_PRIMARY_BIT = 0u,
	LIGHTRAY_ANIMATION_PLAYBACK_BITS_LOOP_BIT = 1u,
	LIGHTRAY_ANIMATION_PLAYBACK_BITS_BURST_BIT = 2u
};

enum lightray_animation_blend_mode_e : u8
{
	LIGHTRAY_ANIMATION_BLEND_MODE_NONE = 0u,
	LIGHTRAY_ANIMATION_BLEND_MODE_DEFAULT = 1u,
	LIGHTRAY_ANIMATION_BLEND_MODE_SINGLE_SEGMENTED_FIRST = 2u,
	LIGHTRAY_ANIMATION_BLEND_MODE_SINGLE_SEGMENTED_SECOND = 3u,
	LIGHTRAY_ANIMATION_BLEND_MODE_DOUBLE_SEGMENTED = 4u
};

enum lightray_animation_playback_data_advance_kind_e : u8
{
	LIGHTRAY_ANIMATION_PLAYBACK_DATA_ADVANCE_KIND_LOOPED = 0u,
	LIGHTRAY_ANIMATION_PLAYBACK_DATA_ADVANCE_KIND_FULL = 1u
};

struct lightray_animation_playback_command_push_data_t
{
	u32 playback_command_batch_index = 0;
	u16 skeletal_mesh_index = 0;
	u16 instance_index = 0;
	u16 animation_index = 0;
	u8 flags = 0;
	lightray_animation_blend_mode_e blend_mode = LIGHTRAY_ANIMATION_BLEND_MODE_NONE;
	u8 first_segment_end_frame = 0;
	u8 second_segment_start_frame = 0;
	f32 blend_step = 0.0f;
};

struct lightray_animation_playback_command_t
{
	u16 skeletal_mesh_index = 0;
	u16 instance_index = 0;
	u16 animation_index = 0;
	u8 flags = 0;
	lightray_animation_blend_mode_e blend_mode = LIGHTRAY_ANIMATION_BLEND_MODE_NONE;
	f32 scale = 0.0f;
	f32 time = 0.0f;
	f32 ticks = 0.0f;
	f32 blend = 0.0f;
	f32 blend_step = 0.0f;
	u8 first_segment_end_frame = 0;
	u8 second_segment_start_frame = 0;
};

struct lightray_animation_playback_command_batch_t
{
	u32 command_buffer_offset = 0;
	u32 command_count = 0;
	u16 b_index = 0;
	u16 a_index = 0; 
};

struct lightray_bone_index_pair_t
{
	u32 computed_bone_matrix_buffer_index = 0;
	u32 bone_buffer_index = 0; // relative
};

struct lightray_bone_mask_t
{
	u32 bone_index_pair_buffer_offset = 0;
	u32 pair_count = 0;
};

struct lightray_vertex_t
{
	glm::vec3 position;
	f32 padding0;

	glm::vec3 normal;
	f32 padding1;

	glm::vec2 uv;
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

struct lightray_overlay_element_t
{
	sunder_v3_t color{};
	sunder_v2_t position{};
	u32 texture_index = 0;
	f32 scale = 0.0f;
	f32 theta = 0.0f;
};

struct lightray_overlay_layer_t
{
	u32 parent_layer = 0;
	u32 overlay_element_buffer_offset = 0;
	u32 overlay_element_count = 0;
	u32 overlay_text_element_buffer_offset = 0;
	u32 overlay_text_element_count = 0;
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
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_B32 = 10u,
	LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_STRING = 11u
};

struct lightray_overlay_text_element_t
{
	const void* data = nullptr;
	u64 element_count = 0; // how many elements in 'data
	glm::vec2 position{};
	glm::vec3 color{};
	lightray_overlay_text_element_type type{};
	f32 glyph_size = 0.0f; // in pixels
	u32 precision = 0; // only for floats
};

enum lightray_anchor_e : u8
{
	LIGHTRAY_ANCHOR_TOP_LEFT = 0u,
	LIGHTRAY_ANCHOR_TOP_RIGHT = 1u,
	LIGHTRAY_ANCHOR_BOTTOM_LEFT = 2u,
	LIGHTRAY_ANCHOR_BOTTOM_RIGHT = 3u,
	LIGHTRAY_ANCHOR_CENTER = 4u
};

struct lightray_overlay_core_t
{
	// f32 anchor_buffer[5]{}; // 4 corners of the screen and the center
	// [0] - top left 
	// [1] - top right
	// [2] - bottom left 
	// [3] - bottom right 
	// [4] - center
	
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

struct lightray_camera_attribute_t
{
	u32 entity_index = 0;
	f32 roll = 0;
	f32 pitch = 0;
	f32 yaw = 0;
	f32 sensitivity = 0;
	f32 fov = 0;
	f32 near_clip_plane_distance = 0;
	f32 far_clip_plane_distance = 0;
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
	lightray_animation_playback_command_batch_t* playback_command_batch_buffer = SUNDER_NULLPTR;
	lightray_animation_playback_command_t * playback_command_buffer = SUNDER_NULLPTR;
	u32 animation_playback_command_per_command_batch_count = 0;
	u32 total_playback_command_batch_count = 0;
	u32 current_playback_command_batch_count = 0;
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
	sunder_v3_t origin{};
	sunder_v3_t direction{};
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

enum lightray_entity_kind : u16
{
	LIGHTRAY_ENTITY_KIND_UNDEFINED = 0u,
	LIGHTRAY_ENTITY_KIND_STATIC_MESH = 1u,
	LIGHTRAY_ENTITY_KIND_SKELETAL_MESH = 2u,
	LIGHTRAY_ENTITY_KIND_COLLISION_MESH = 3u,
	LIGHTRAY_ENTITY_KIND_AABB = 4u,
	LIGHTRAY_ENTITY_KIND_CAMERA = 5u,
	LIGHTRAY_ENTITY_KIND_OVERLAP_MESH = 6u,
	LIGHTRAY_ENTITY_KIND_CUSTOM = 7u
};

enum lightray_entity_bits : u16
{
	LIGHTRAY_ENTITY_BITS_OF_COLLIDABLE_KIND_BIT = 0u, // entity that has aabb, or maybe collision mesh entities bound to it. aabb, collision mesh entities ARE NOT considered of collidable kind
	LIGHTRAY_ENTITY_BITS_UNDO_CAMERA_TILT_BIT = 1u,
	LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_TRANSLATION_BIT = 2u,
	LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_ROTATION_BIT = 3u,
	LIGHTRAY_ENTITY_BITS_INHERIT_ENTITY_BINDING_CHAIN_SCALE_BIT = 4u,
	LIGHTRAY_ENTITY_BITS_MESH_BOUND_BIT = 5u,
	LIGHTRAY_ENTITY_BITS_CONTAINS_CAMERA_ATTRIBUTE_BIT = 6u,
	LIGHTRAY_ENTITY_BITS_CONTAINS_LIGHT_ATTRIBUTE_BIT = 7u,
	LIGHTRAY_ENTITY_BITS_CONTAINS_COLLISION_ATTRIBUTE_BIT = 8u,
	LIGHTRAY_ENTITY_BITS_CONTAINS_OVERLAP_ATTRIBUTE_BIT = 9u,
	LIGHTRAY_ENTITY_BITS_NON_DEFAULT_ROTATION_BIT = 10u,
	LIGHTRAY_ENTITY_BITS_NON_UNIT_SCALE_BIT = 11u,
	LIGHTRAY_ENTITY_BITS_BOUND_BIT = 12u
};

struct lightray_entity_t
{
	u16 generation = 0;
	lightray_entity_kind kind{};
	u16 flags = 0;
	u16 global_mesh_index = 0;
	u16 camera_attribute_index = 0;
	u16 light_attribute_index = 0;
	u32 instance_model_binding_index = 0;
	u32 parent_index = 0;
	u32 children_index_count = 0;
	u32 collision_attribute_index = 0;
	u32 bone_binding_index = 0; // computed bone transform matrix buffer index
	u32 binding_chain_index_buffer_index = 0;
	u32 overlap_attribute_index = 0;
	// u32 virtual_entity_binding_index = 0; // not a bad idea, if needed later for some, little bit cleaner implementation of somethings
};

struct lightray_scene_suballocation_data_t
{
	u32 total_instance_model_count = 0;
	u32 total_entity_count = 0;
	u32 total_mesh_count = 0;

	u32 collision_attribute_count = 0;
	u32 grid_cell_index_per_collision_attribute_count = 0;
	u32 collision_mesh_per_batch_count = 0;

	u32 game_side_entity_kind_count = 0;

	const u32* index_buffer = nullptr;
	const lightray_vertex_t* vertex_buffer = nullptr;

	u32 total_raycast_grid_cell_index_count = 0;
	u32 total_raycast_pierce_layer_test_data_count = 0;
	u32 total_per_frame_raycast_count = 0;

	u32 entity_children_index_per_entity_count = 0;

	u32 overlap_attribute_count = 0;
	u32 overlap_mesh_per_batch_count = 0;
	u32 overlap_attribute_grid_cell_row_index_per_attribute_count = 0;

	u32 grid_cell_count = 0;

	u32 overlap_attribute_per_cell_count = 0;
	u32 collision_attribute_per_cell_count = 0;
	u32 overlapped_attribute_index_per_game_side_entity_count = 0;

	sunder_arena_t* arena = nullptr;
};

struct lightray_mesh_binding_t
{
	u32 transform_index = 0;
	u32 instance_model_index = 0;
};

struct lightray_mesh_binding_result_t
{
	lightray_result result{};
	u32 mesh_binding_index = 0;
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
	u32 opaque_instance_count = 0;
	u32 current_opaque_instance_binding_count = 0;
	u32 wireframe_instance_count = 0;
	u32 current_wireframe_instance_binding_count = 0;
};

enum lightray_raycast_bits : u32
{
	LIGHTRAY_RAYCAST_BITS_PIERCE_BIT = 0u,
	LIGHTRAY_RAYCAST_BITS_MAX_PIERCE_THRESHOLD_BIT = 1u,
	LIGHTRAY_RAYCAST_BITS_HIT_ANY_BIT = 2u,
	LIGHTRAY_RAYCAST_BITS_IGNORE_DYNAMIC_COLLISION_ATTRIBUTES_BIT = 3u
};

enum lightray_collision_attribute_bits : u32
{
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_COLLIDED_WITH_BIT = 0u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_COLLIDES_BIT = 1u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_AABB_COLLIDES_BIT = 2u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_COLLISION_MESHES_COLLIDE_BIT = 3u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT = 4u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_STATIC_BIT = 5u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_SAFE_RESOLVE_BIT = 7u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_SINK_RESOLVE_BIT = 8u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_SENTIENT_BIT = 9u,
	LIGHTRAY_COLLISION_ATTRIBUTE_BITS_DEFERRED_BIT = 10u
};

struct lightray_raycast_pierce_layer_t
{
	u16 layer = 0;
	u16 threshold = 0;
};

struct lightray_collision_attribute_t
{
	u64 collision_layer_bitmask = 0;
	u64 cull_collision_layer_bitmask = 0;
	u16 game_side_entity_kind = 0;
	u16 pierce_layer = 0;
	u32 game_side_entity_index = 0;
	u32 has_already_collided_with_bitmask_buffer_offset = 0; // remove
	u32 has_already_failed_to_collide_bit_buffer_offset = 0;
	u32 collision_layer_index_buffer_offset = 0; // remove
	u32 collision_layer_count = 0;
	u32 grid_cell_index_buffer_offset = 0; // remove
	u32 row_count = 0;
	u32 width_per_row = 0;
	u32 aabb_index = 0;
	u32 collision_mesh_batch_index = 0;
	u32 flags = 0;
	u32 entity_index = 0;
	u32 should_reproject_vertices_bitmask = 0; // remove
	u32 has_already_projected_vertices_bitmask = 0; // remove
	u32 trace_step_count = 0;
	f32 trace_step_factor = 0.0f;
	u32 trace_data_buffer_index = 0;
	u32 max_trace_step_count = 0;
};

struct lightray_trace_step_t
{
	sunder_v3_t position{};
	u32 grid_cell_index_count = 0;
	b32 marked = SUNDER_FALSE;
	// u32 flags = 0;  // MARKED_BIT | RESTING_BIT
};

enum lightray_overlap_attribute_bits_e : u32
{
	LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_OVERLAPPED_BIT = 0u,
	LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_OVERLAPS_BIT = 1u,
	LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_AABB_OVERLAPS_BIT = 2u,
	LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_CAN_BE_TRACED_BIT = 3u,
	LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_OBB_OVERLAPS_BIT = 4u,
	LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_HAS_BEEN_TRACED_BIT = 5u,
	LIGHTRAY_OVERLAP_ATTRIBUTE_BITS_STATIC_BIT = 6u,
};

enum lightray_overlap_mesh_bits_e : u32
{
	LIGHTRAY_OVERLAP_MESH_BITS_IS_OBB_BIT = 0u,
};

struct lightray_overlap_attribute_t
{
	u64 overlap_layer_bitmask = 0;
	u64 cull_overlap_layer_bitmask = 0;
	u16 pierce_layer = 0;
	u32 game_side_entity_index = 0;
	u32 overlap_attribute_index_buffer_offset = 0;
	u32 overlap_attribute_index_count = 0;
	u32 has_already_overlapped_with_bitmask_buffer_offset = 0;
	u32 overlap_layer_count = 0;
	u32 overlap_layer_index_buffer_offset = 0;
	u32 grid_cell_index_buffer_offset = 0;
	u32 row_count = 0;
	u32 width_per_row = 0;
	u32 overlap_mesh_batch_index = 0;
	u32 aabb_index = 0;
	u32 entity_index = 0;
	u32 flags = 0;
	u32 should_reproject_vertices_bitmask = 0;
	u32 has_already_projected_vertices_bitmask = 0;
	u32 has_already_failed_to_overlap_bitmask_buffer_offset = 0;
};

struct lightray_overlap_mesh_batch_t
{
	u32 overlap_mesh_buffer_offset = 0;
	u32 overlap_mesh_count = 0;
};

struct lightray_overlap_mesh_t
{
	sunder_v3_t* sentinel_world_space_vertex_position_buffer = SUNDER_NULLPTR;
	const lightray_model_t* model = nullptr;

	u32 sentinel_index_buffer_offset = 0;
	u32 sentinel_local_space_vertex_position_buffer_offset = 0;
	u32 sentinel_vertex_count = 0;
	u32 index_count = 0;
	u32 index_buffer_offset = 0;
	u32 entity_index = 0;
	u16 obb_index = 0;
	u16 obb_owner_index = 0;
	u32 flags = 0;
};

struct lightray_raycast_pierce_layer_test_data_t
{
	sunder_v3_t intersection_point{};
	sunder_v3_t triangle_normal{};
	f32 squared_distance = 0.0f;
	u32 attribute_index = 0;
	u16 previous_hit_pierce_layer = 0;
	u16 previous_hit_applied_threshold = 0;
};

struct lightray_rotation_t
{
	sunder_v3_t axis{};
	f32 angle = 0.0f; // degrees
};

struct lightray_transform_binding_v3_t
{
	sunder_v3_t relative_offset{};
	u32 parent_entity_index = 0;
	u32 entity_index = 0;
};

struct lightray_transform_binding_quat_t
{
	lightray_rotation_t relative_rotation{};
	u32 parent_entity_index = 0;
	u32 entity_index = 0;
};

struct lightray_collision_mesh_t
{
	sunder_v3_t* sentinel_world_space_vertex_position_buffer = SUNDER_NULLPTR;
	sunder_v3_t* trace_step_sentinel_world_space_vertex_position_buffer = SUNDER_NULLPTR;
	u32 funnel_precedence = 0;
	u32 sentinel_index_buffer_offset = 0;
	u32 sentinel_local_space_vertex_position_buffer_offset = 0;
	u32 sentinel_vertex_count = 0;
	u32 max_trace_step_count = 0;

	const lightray_model_t* model = nullptr;
	u32 vertex_count = 0; // should be renamed to index_count
	u32 index_buffer_offset = 0;
	u32 entity_index = 0;
};

struct lightray_collision_mesh_batch_t
{
	u32 collision_mesh_buffer_offset = 0;
	u32 collision_mesh_count = 0;
};

struct lightray_entity_binding_chain_t
{
	u32 entity_binding_chain_index_buffer_offset = 0;
	u32 entity_binding_chain_index_count = 0;
};

struct lightray_gjk_support_point_t
{
	sunder_v3_t p{};
	sunder_v3_t a{};
	sunder_v3_t b{};
};

struct lightray_epa_face_t
{
	u32 i0 = 0;
	u32 i1 = 0;
	u32 i2 = 0;
	sunder_v3_t normal{};
	f32 distance = 0.0f;
	b32 obsolete = SUNDER_TRUE;
};

struct lightray_epa_result_t
{
	b32 hit = SUNDER_FALSE;
	sunder_v3_t normal{};
	f32 depth = 0.0f;
	sunder_v3_t contact_point{};
};

struct lightray_epa_edges_t
{
	i32 first = 0;
	i32 second = 0;
};

struct edge_t
{
	i32 i0 = 0;
	i32 i1 = 0;
};

struct lightray_capsule_t
{
	sunder_v3_t p1{};
	sunder_v3_t p2{};
	f32 r = 0.0f;
};

struct lightray_triangle_t
{
	sunder_v3_t v0{};
	sunder_v3_t v1{};
	sunder_v3_t v2{};
};

struct lightray_m4_cache_t
{
	sunder_v4_t row0{};
	sunder_v4_t row1{};
	sunder_v4_t row2{};
};

struct lightray_collision_vertex_buffers_population_filter_data_t
{
	u32 global_mesh_index = 0;
	b32 is_collision_mesh = SUNDER_FALSE;
};

struct lightray_trace_data_t
{
	b32 should_resolve = SUNDER_FALSE;
	u32 collision_attribute_index = 0;
	sunder_v3_t previous_position{};
	sunder_v3_t movement_this_frame{};
	sunder_v3_t velocity{};
};

struct lightray_trace_instance_t
{
	u32 collision_attribute_index = 0;
	u32 trace_step_global_index = 0;
};

SUNDER_DEFINE_BUFFER(sunder_v3_t, lightray, v3, u32)
SUNDER_DEFINE_BUFFER(lightray_m4_cache_t, lightray, m4_cache, u32)
SUNDER_DEFINE_BUFFER(lightray_raycast_pierce_layer_t, lightray, pierce_layer, u32)
SUNDER_DEFINE_BUFFER(lightray_trace_data_t, lightray, trace_data, u32)

SUNDER_DEFINE_FLUX(lightray_overlap_attribute_t, lightray, overlap_attribute, u32)
SUNDER_DEFINE_FLUX(lightray_overlap_mesh_batch_t, lightray, overlap_mesh_batch, u32)
SUNDER_DEFINE_FLUX(lightray_overlap_mesh_t, lightray, overlap_mesh, u32)
SUNDER_DEFINE_FLUX(lightray_entity_t, lightray, entity, u32)
SUNDER_DEFINE_FLUX(sunder_v3_t, lightray, v3, u32)
SUNDER_DEFINE_FLUX(sunder_quat_t, lightray, quat, u32)
SUNDER_DEFINE_FLUX(lightray_mesh_binding_offsets_t, lightray, mesh_binding_offsets, u32)
SUNDER_DEFINE_FLUX(lightray_mesh_binding_t, lightray, mesh_binding, u32)
SUNDER_DEFINE_FLUX(lightray_mesh_binding_metadata_t, lightray, mesh_binding_metadata, u32)
SUNDER_DEFINE_FLUX(lightray_entity_binding_chain_t, lightray, entity_binding_chain, u32)
SUNDER_DEFINE_FLUX(sunder_m4_t, lightray, m4, u32)
SUNDER_DEFINE_FLUX(lightray_collision_attribute_t, lightray, collision_attribute, u32)
SUNDER_DEFINE_FLUX(lightray_collision_mesh_batch_t, lightray, collision_mesh_batch, u32)
SUNDER_DEFINE_FLUX(lightray_collision_mesh_t, lightray, collision_mesh, u32)
SUNDER_DEFINE_FLUX(lightray_trace_data_t, lightray, trace_data, u32);

SUNDER_DEFINE_SUBARENA(lightray_raycast_pierce_layer_test_data_t, lightray, raycast_pierce_layer_test_data, u32)

SUNDER_DEFINE_UNIFIED_FLUX(lightray_overlap_mesh_batch_t, lightray, overlap_mesh_batch, u32)
SUNDER_DEFINE_UNIFIED_FLUX(lightray_collision_mesh_batch_t, lightray, collision_mesh_batch, u32)
SUNDER_DEFINE_UNIFIED_FLUX(lightray_trace_step_t, lightray, trace_step, u32)
SUNDER_DEFINE_UNIFIED_FLUX(lightray_trace_instance_t, lightray, trace_instance, u32)

struct lightray_raycast_core_t
{
	u32 max_raycast_per_frame_count = 0;

	sunder_subarena_u32_t raycast_grid_cell_index_subarena{};
	lightray_subarena_raycast_pierce_layer_test_data_t pierce_layer_test_data_subarena{};

	sunder_bit_buffer_t has_already_been_traced_collision_attribute_bit_buffer{};
	sunder_bit_buffer_t has_already_been_traced_overlap_attribute_bit_buffer{};
	
	sunder_bit_buffer_t has_already_failed_to_trace_collision_attribute_bit_buffer{};
	sunder_bit_buffer_t has_already_failed_to_trace_overlap_attribute_bit_buffer{};

	sunder_bit_buffer_t has_already_cached_inverse_overlap_mesh_bit_buffer{};
	sunder_bit_buffer_t has_already_cached_inverse_collision_mesh_bit_buffer{};

	lightray_buffer_m4_cache_t m4_inverse_cache_overlap_mesh_buffer{};
	lightray_buffer_m4_cache_t m4_inverse_cache_collision_mesh_buffer{};
};

struct lightray_entity_core_t
{
	lightray_flux_entity_t entity_buffer{};
	lightray_flux_v3_t position_buffer{};
	lightray_flux_v3_t scale_buffer{};
	lightray_flux_quat_t rotation_buffer{};

	lightray_flux_mesh_binding_t mesh_binding_buffer{};
	lightray_flux_mesh_binding_offsets_t mesh_binding_offsets_buffer{};
	lightray_flux_mesh_binding_metadata_t mesh_binding_metadata_buffer{};
	sunder_bit_buffer_t mesh_binding_visibility_bit_buffer{};

	lightray_flux_entity_binding_chain_t entity_binding_chain_buffer{};
	sunder_flux_u32_t chained_entity_index_buffer{};
	sunder_flux_u32_t entity_children_index_buffer{};
	sunder_flux_u32_t binding_chain_depth_buffer{};
	sunder_flux_u32_t entity_binding_chain_index_buffer{};
	lightray_flux_m4_t chained_entity_transform_matrix_buffer{};

	sunder_bit_buffer_t has_already_written_into_binding_chain_bitmask_buffer{};
	sunder_bit_buffer_t already_part_of_other_binding_chain_bitmask_buffer{};
	sunder_bit_buffer_t has_already_assembled_transform_matrix_bit_buffer{};
};

struct lightray_overlap_attribute_core_t
{
	lightray_flux_overlap_attribute_t overlap_attribute_buffer{};
	lightray_unified_flux_overlap_mesh_batch_t overlap_mesh_batch_buffer{};
	lightray_flux_overlap_mesh_t overlap_mesh_buffer{};
	sunder_unified_flux_u32_t overlap_grid_cell_row_index_buffer{};
	sunder_bit_buffer_t has_already_overlapped_with_bit_buffer{};
	sunder_bit_buffer_t has_already_failed_to_overlap_bit_buffer{};
	sunder_buffer_u32_t overlap_layer_index_buffer{};
	sunder_unified_buffer_u32_t overlap_attribute_index_buffer{};
	sunder_flux_u32_t filtered_overlap_mesh_index_buffer{};
};

struct lightray_collision_attribute_core_t
{
	lightray_flux_collision_attribute_t collision_attribute_buffer{};
	lightray_unified_flux_collision_mesh_batch_t collision_mesh_batch_buffer{};
	lightray_flux_collision_mesh_t collision_mesh_buffer{};
	sunder_buffer_u32_t collision_layer_index_buffer{};
	sunder_unified_flux_u32_t collision_grid_cell_row_index_buffer{};
	sunder_bit_buffer_t has_already_collided_with_bit_buffer{};
	sunder_bit_buffer_t has_already_failed_to_collide_bit_buffer{};
	sunder_flux_u32_t filtered_collision_mesh_index_buffer{};

	lightray_flux_trace_data_t trace_data_buffer{};
	lightray_unified_flux_trace_step_t trace_step_buffer{};
	sunder_unified_flux_u32_t trace_step_grid_cell_index_buffer{};
};

// rename this to lightray_core_t
struct lightray_scene_t
{
	const u32* index_buffer = nullptr;
	const lightray_vertex_t* vertex_buffer = nullptr;

	////////////////////////////////////////////////////////////////////
	sunder_v3_t* sentinel_local_space_vertex_position_buffer = SUNDER_NULLPTR;
	u32* sentinel_index_buffer = SUNDER_NULLPTR;

	u32 total_sentinel_local_space_vertex_position_count = 0;
	u32 total_sentinel_index_count = 0;

	u32* capsule_beyond_lower_clipping_plane_sentinel_index_buffer = SUNDER_NULLPTR;
	u32* capsule_beyond_lower_clipping_plane_triangle_index_buffer = SUNDER_NULLPTR;
	u32 capsule_beyond_lower_clipping_plane_sentinel_index_count = 0;
	u32 capsule_beyond_lower_clipping_plane_triangle_index_count = 0;

	u32 capsule_sentinel_local_space_vertex_position_buffer_offset = 0;
	u32 capsule_sentinel_index_buffer_offset = 0;
	////////////////////////////////////////////////////////////////////

	lightray_overlap_attribute_core_t overlap_attribute_core{};
	lightray_collision_attribute_core_t collision_attribute_core{};

	////////////////////////////////////////////////////////////////////
	lightray_collision_vertex_buffers_population_filter_data_t* collision_vertex_buffers_population_filter_data_buffer = SUNDER_NULLPTR;
	u32 collision_vertex_buffers_population_filter_data_count = 0;
	////////////////////////////////////////////////////////////////////

	u32 total_instance_model_count = 0;
	u32 total_mesh_count = 0;
	u32 current_entity_count = 0;
	u32 total_entity_count = 0;

	lightray_entity_t* entity_buffer = nullptr;
	sunder_v3_t* position_buffer = nullptr;
	sunder_v3_t* rotation_buffer = nullptr;
	sunder_v3_t* scale_buffer = nullptr;
	sunder_quat_t* quat_rotation_buffer = nullptr;

	sunder_arena_t collision_mesh_vertex_buffers_arena{}; // rename 
	 
	lightray_raycast_core_t raycast_core{};

	lightray_mesh_binding_offsets_t* mesh_binding_offsets = nullptr; // per mesh
	lightray_mesh_binding_t* mesh_binding_buffer = nullptr; // per instance model
	lightray_mesh_binding_metadata_t* mesh_binding_metadata_buffer = nullptr; // per mesh
	u32 mesh_binding_count = 0;
	u64 visibility_flags = 0;  // change this

	//////////////////////
	u32* chained_entity_index_buffer = nullptr;
	u32 total_chained_entity_count = 0;
	u32 current_chained_entity_count = 0;

	u32* entity_children_index_buffer = nullptr;
	u32 total_entity_children_index_count = 0;
	u32 entity_children_index_per_entity_count = 0;

	u64* has_already_written_into_binding_chain_bitmask_buffer = nullptr;
	u32 total_has_already_written_into_binding_chain_bitmask_count = 0;
	u32 has_already_written_into_binding_chain_bitmask_count_for_total_entity_count = 0;

	u32* binding_chain_depth_buffer = nullptr; // how deep into children (xd)
	u32 total_binding_chain_depth_count = 0;

	u64* already_part_of_other_binding_chain_bitmask_buffer = nullptr;
	u32 total_already_part_of_other_binding_chain_bitmask_count = 0;
	u32 already_part_of_other_binding_chain_bitmask_count_for_total_entity_count = 0; // how many u64 integers i need to have to represent every entity index out there

	lightray_entity_binding_chain_t* entity_binding_chain_buffer = nullptr;
	u32 total_entity_binding_chain_count = 0;
	u32 current_entity_binding_chain_count = 0;

	u32* entity_binding_chain_index_buffer = nullptr;
	u32 total_binding_chain_index_count = 0;
	u32 binding_chain_index_per_entity_count = 0;

	u64* has_already_assembled_transform_matrix_bitmask_buffer = nullptr;
	u32 total_has_already_assembled_transform_matrix_bitmask_count = 0;
	u32 has_already_assembled_transform_matrix_bitmask_per_entity_count = 0;

	sunder_m4_t* chained_entity_transform_matrix_buffer = nullptr;
	u32 total_chained_entity_transform_matrix_count = 0;
	//////////////////////
};

struct lightray_console_t
{
	char* buffer = nullptr;
	u32 current_size = 0;
	u32 current_index = 0;
	u32 capacity = 0;
};

struct lightray_grid_creation_data_t
{
	sunder_arena_t* arena = nullptr;
	lightray_scene_t* scene = nullptr;
	u32 first_column_entity_index = 0;
	u32 first_row_entity_index = 0;
	u32 column_count = 0;
	u32 row_count = 0;
	sunder_v3_t origin{};
	f32 cell_width = 0.0f;
	f32 cell_height = 0.0f;
	u32 collision_attribute_per_cell_count = 0;
	u32 overlap_attribute_per_cell_count = 0;
	u32 collision_attribute_count = 0;
};

struct lightray_grid_cell_t
{
	u32 collision_attribute_index_buffer_offset = 0;
	u32 collision_attribute_index_count = 0;

	u32 overlap_attribute_index_buffer_offset = 0;
	u32 overlap_attribute_index_count = 0;

	u32 trace_instance_count = 0;
	u32 deferred_trace_instance_count = 0;

	sunder_bit_buffer_t collision_attribute_written_bit_buffer{};
};

struct lightray_grid_cell_aabb_coordinates_t
{
	i32 min = 0;
	i32 max = 0;
};

struct lightray_grid_t
{
	lightray_grid_cell_t* cell_buffer = nullptr;
	u32* collision_attribute_index_buffer = nullptr;
	u32* overlap_attribute_index_buffer = SUNDER_NULLPTR;

	lightray_unified_flux_trace_instance_t trace_instance_buffer{};
	lightray_unified_flux_trace_instance_t trace_instance_deferred_buffer{};

	u32 overlap_attribute_index_count = 0;
	u32 overlap_attribute_index_count_per_cell = 0;
	u32 collision_attribute_index_count = 0;
	u32 collision_attribute_index_count_per_cell = 0;
	u32 cell_count = 0;
	sunder_v3_t origin{};
	u32 column_count = 0;
	u32 row_count = 0;
	f32 cell_width = 0.0f;
	f32 cell_height = 0.0f;
};

enum lightray_ray_triangle_face_culling_mode_e : u16
{
	LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_NONE = 0u,
	LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_BACKFACE = 1u,
	LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_FRONTFACE = 2u
};

enum lightray_raycast_intersection_mode_e : u16
{
	LIGHTRAY_RAYCAST_INTERSECTION_MODE_COLLISION_MESH = 0u,
	LIGHTRAY_RAYCAST_INTERSECTION_MODE_OVERLAP_MESH = 1u
};

struct lightray_raycast_memory_reserve_result_t
{
	u32 grid_cell_index_per_ray_count = 0;
	u32 pierce_layer_test_data_per_ray_count = 0;

	u32 grid_cell_index_subarena_offset = 0;
	u32 pierce_layer_test_data_subarena_offset = 0;

	u32 has_already_been_traced_attribute_bit_buffer_offset = 0;
	u32 has_already_failed_to_trace_attribute_bit_buffer_offset = 0;

	b32 success = SUNDER_TRUE;
};

struct lightray_raycast_data_t
{
	lightray_buffer_pierce_layer_t pierce_layer_buffer{};
	u64 layer_bitmask = 0;
	u64 cull_layer_bitmask = 0;
	sunder_buffer_u32_t layer_index_buffer{};
	lightray_raycast_memory_reserve_result_t memory_reserve_result{};
	lightray_ray_t ray{};
	const lightray_grid_t* grid = SUNDER_NULLPTR;
	u32 flags = 0;
	u32 cube_entity_index = 0;
	lightray_raycast_intersection_mode_e intersection_mode = LIGHTRAY_RAYCAST_INTERSECTION_MODE_OVERLAP_MESH;
	lightray_ray_triangle_face_culling_mode_e culling_mode = LIGHTRAY_RAY_TRIANGLE_FACE_CULLING_MODE_BACKFACE;
};

struct lightray_raycast_result_t
{
	u32 hit_count = 0;
	b32 hit_any = SUNDER_FALSE;
};

struct lightray_gjk_closest_hit_t
{
	sunder_v3_t closest_a{};
	sunder_v3_t closest_b{};
	sunder_v3_t normal{};
	f32 distance = 0.0f;
	b32 valid = SUNDER_FALSE;
};

struct lightray_capsule_vertex_segments_t
{
	sunder_v3_t* v0v17 = SUNDER_NULLPTR;
	sunder_v3_t* v8v15 = SUNDER_NULLPTR;

	sunder_v3_t* v0v8 = SUNDER_NULLPTR;
	sunder_v3_t* v17v15 = SUNDER_NULLPTR;

	sunder_v3_t* v0v1 = SUNDER_NULLPTR;
	sunder_v3_t* v1v17 = SUNDER_NULLPTR;
	sunder_v3_t* v8v10 = SUNDER_NULLPTR;
	sunder_v3_t* v10v15 = SUNDER_NULLPTR;
};

struct lightray_capsule_to_plane_distance_computation_data_t
{
	lightray_capsule_vertex_segments_t* segments = SUNDER_NULLPTR;
	u64 capsule_vertex_count = 0;
	const sunder_v3_t* capsule_local_space_vertex_position_buffer = SUNDER_NULLPTR;
	const sunder_v3_t* plane_local_space_vertex_position_buffer = SUNDER_NULLPTR;
	const sunder_m4_t* capsule_model = SUNDER_NULLPTR;
	const sunder_m4_t* capsule_inverse = SUNDER_NULLPTR;
	sunder_v3_t* ray_cluster_origin_buffer = SUNDER_NULLPTR;
	const u16* lower_clipping_plane_vertex_index_buffer = SUNDER_NULLPTR;
	sunder_v3_t* ray_capsule_intersection_point_buffer = SUNDER_NULLPTR;
	sunder_v3_t* ray_plane_intersection_point_buffer = SUNDER_NULLPTR;
	u64 plane_vertex_count = 0;
	const sunder_m4_t* plane_model = SUNDER_NULLPTR;
	const sunder_m4_t* plane_inverse = SUNDER_NULLPTR;
	u64* capsule_hit_bitmask_buffer = SUNDER_NULLPTR;
	u64* plane_hit_bitmask_buffer = SUNDER_NULLPTR;
	u16* hit_mapping_buffer = SUNDER_NULLPTR;
	sunder_v3_t capsule_position{};
	sunder_v3_t cluster_direction{};
};

struct lightray_capsule_onto_plane_projection_data_t
{
	lightray_capsule_vertex_segments_t* segments = SUNDER_NULLPTR;
	sunder_v3_t* ray_cluster_origin_buffer = SUNDER_NULLPTR;
	const u16* lower_clipping_plane_vertex_index_buffer = SUNDER_NULLPTR;
	sunder_v3_t* ray_capsule_intersection_point_buffer = SUNDER_NULLPTR;
	sunder_v3_t* ray_plane_intersection_point_buffer = SUNDER_NULLPTR;

	u64 collision_layer_bitmask = 0;
	u64 cull_collision_layer_bitmask = 0;
	u32* collision_layer_index_buffer = SUNDER_NULLPTR;
	lightray_raycast_pierce_layer_t* pierce_layer_buffer = SUNDER_NULLPTR;
	u32 pierce_layer_count = 0;
	u32 collision_layer_index_count = 0;
	sunder_v3_t entry_ray_origin{};
	f32 entry_ray_distance = 0.0f;
	u32 raycast_flags = 0;
	u32 cube_entity_index = 0;
	u32 line_trace_entity_index = 0;
	u32 raycast_grid_cell_index_count = 0;
	u32 raycast_pierce_layer_test_data_count = 0;
};

enum lightray_step_bits_e : u32
{
	LIGHTRAY_STEP_BITS_INCLUDE_DYNAMIC_BIT = 0u,
	LIGHTRAY_STEP_BITS_SOLVE_EPA_BIT = 1u
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
b32														lightray_ray_triangle_intersect(const lightray_ray_t* ray, const sunder_v3_t* triangle_vertices, f32* out_t, f32* out_u, f32* out_v, lightray_ray_triangle_face_culling_mode_e culling_mode, sunder_v3_t* hit_v0, sunder_v3_t* hit_v1, sunder_v3_t* hit_v2);
void														lightray_compute_aabb_min_max(const sunder_v3_t& position, const sunder_v3_t& scale, sunder_v3_t* min, sunder_v3_t* max);
b32														lightray_aabbs_intersect(const sunder_v3_t& position_a, const sunder_v3_t& scale_a, const sunder_v3_t& position_b, const sunder_v3_t& scale_b);
b32														lightray_ray_aabb_intersect(const lightray_ray_t* ray, const sunder_v3_t& aabb_position, const sunder_v3_t& aabb_scale, f32* t_hit, sunder_v3_t* normal);
b32														lightray_ray_aabb_intersect_precomputed(const lightray_ray_t* ray, const sunder_v3_t& aabb_min, const sunder_v3_t& aabb_max, f32* t_hit, sunder_v3_t* normal);
void														lightray_get_raw_vertex_positions(u32 index_buffer_offset, u32 index_count, sunder_v3_t* buffer, const lightray_vertex_t* vertex_buffer, const u32* index_buffer);
				
void														lightray_compute_sentinel_world_space_vertex_positions(const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_m4_t& m);
void														lightray_compute_sentinel_world_space_vertex_positions_via_index_buffer(const u32* sentinel_index_buffer, const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_m4_t& m);
void														lightray_compute_sentinel_world_space_vertex_positions_ts(const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_v3_t& t, const sunder_v3_t& s);
void														lightray_compute_sentinel_world_space_vertex_positions_t(const sunder_v3_t* sentinel_local_space_vertex_position_buffer, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, const sunder_v3_t& t);

sunder_v3_t											lightray_gjk_find_furthest_point(const sunder_v3_t* tri, u32 vertex_count, const sunder_v3_t& direction);
lightray_gjk_support_point_t				lightray_gjk_support(const sunder_v3_t* vertex_positions1, u32 vertex_count1, const sunder_v3_t* vertex_positions2, u32 vertex_count2, const sunder_v3_t& direction);
b32														lightray_gjk_same_direction(const sunder_v3_t& direction, sunder_v3_t& ao);
b32														lightray_gjk_line(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction);
b32														lightray_gjk_triangle(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction);
b32														lightray_gjk_tetrahedron(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction);
b32														lightray_gjk_next_simplex(lightray_gjk_support_point_t* simplex, u32* simplex_size, sunder_v3_t* direction);
b32														lightray_gjk_intersect(const sunder_v3_t* vertex_positions1, u32 vertex_count1, const sunder_v3_t* vertex_positions2, u32 vertex_count2, lightray_gjk_support_point_t* out_simplex, u32* out_simplex_size);
void														lightray_gjk_compute_closest_point(lightray_gjk_support_point_t* simplex, u32 simplex_size, sunder_v3_t* closest, f32* bary);
lightray_gjk_closest_hit_t						lightray_gjk_distance(const sunder_v3_t* vertices_a, u32 vertex_count_a, const sunder_v3_t* vertices_b, u32 vertex_count_b, const sunder_v3_t& search_direction);

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
lightray_entity_creation_result_t			lightray_create_entity(lightray_scene_t* scene, lightray_entity_kind kind, u16 flags);
lightray_mesh_binding_result_t			lightray_bind_mesh(lightray_scene_t* scene, u32 entity_index, u16 mesh_index, lightray_render_target_kind kind);
void														lightray_hide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene);
void														lightray_unhide_entity(lightray_model_t* instance_model_buffer, lightray_model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, lightray_scene_t* scene);
void														lightray_bind_entity(lightray_scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index);
u32														lightray_get_entity_bound_collision_mesh_index_count(const lightray_scene_t* scene, u32 entity_index);
void														lightray_get_entity_bound_collision_mesh_indices(const lightray_scene_t* scene, u32 entity_index, u32* collision_mesh_index_buffer);
u32														lightray_get_entity_bound_aabb_index_count(const lightray_scene_t* scene, u32 collision_mesh_index);
void														lightray_get_entity_bound_aabb_indices(const lightray_scene_t* scene, u32 collision_mesh_index, u32* aabb_index_buffer);
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
void														lightray_compute_interpolated_skeleton_transform(lightray_animation_core_t* animation_core, u32 animation_playback_index);
void														lightray_blend_skeleton_transform(lightray_animation_core_t* animation_core, u32 animation_playback_index_a, u32 animation_playback_index_b, f32 blend_factor);
void														lightray_blend_skeleton_transform_additive(lightray_animation_core_t* animation_core, u32 animation_playback_index_a, u32 animation_playback_index_b, f32 blend_factor);

u32														lightray_compute_skeletal_mesh_bone_count_with_respect_to_instance_count(u32 bone_count, u32 instance_count);
u32														lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(u32 instance_index, u32 bone_count, u32 base_offset);

void														lightray_assimp_get_node_hierarchy_metadata(const aiNode* node, u32* total_node_count, u64* aligned_name_byte_code_size, u32 alignment);
void														lightray_populate_node_related_string_upon_suballocation(sunder_arena_t* arena, sunder_string_t* host, cstring_literal* string, u32 length);
void														lightray_assimp_execute_first_node_buffer_population_pass(const aiNode* node, lightray_node_t* node_buffer, sunder_arena_t* arena, sunder_string_t* names, u32* current_index);
void														lightray_assimp_execute_second_node_buffer_population_pass(const aiNode* node, lightray_node_t* node_buffer, u32 node_count, const sunder_string_t* names, u32* current_index, u32 node_buffer_offset);

u32														lightray_get_skeletal_mesh_global_mesh_index(u32 skeletal_mesh_index, u32 static_mesh_count);
glm::vec2												lightray_get_cursor_position(GLFWwindow* window);
void														lightray_load_json_garbage();
i32														lightray_get_file_size(cstring_literal* path);

void														lightray_create_grid(lightray_grid_t* grid, const lightray_grid_creation_data_t* creation_data);
b32														lightray_get_grid_cell_coordinates(const lightray_grid_t* grid, const sunder_v3_t* world_position, u32* row_index, u32* column_index);
void														lightray_get_grid_cell_coordinates_aabb(const lightray_grid_t* grid, const sunder_v3_t* aabb_position, const sunder_v3_t* aabb_scale, lightray_grid_cell_aabb_coordinates_t* row_coordinates, lightray_grid_cell_aabb_coordinates_t* column_coordinates);

u32														lightray_push_collision_attribute(lightray_scene_t* scene, u32 entity_index, u32 flags, u32 max_trace_step_count);
void														lightray_bind_aabb(lightray_scene_t* scene, u32 entity_index, u32 aabb_index);
u32														lightray_get_grid_cell_index(const lightray_grid_t* grid, u32 row_index, u32 column_index);
sunder_v3_t											lightray_get_grid_cell_center(const lightray_grid_t* grid, u32 row_index, u32 column_index);

void														lightray_log_glm_matrix(const glm::mat4& m);
glm::mat4												lightray_copy_sunder_m4_to_glm(const sunder_m4_t& m);
sunder_m4_t										lightray_copy_glm_mat4_to_sunder(const glm::mat4& m);

void														lightray_bind_position(lightray_scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index, const sunder_v3_t& relative_offset);

u32														lightray_push_collision_mesh_batch(lightray_scene_t* scene); // 0

															// by vertex count, the total vertex count (before merging) is meant (index count should contain the total vertex count value) / returns relative to chosen batch index
u32														lightray_push_collision_mesh(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_mesh_index, u32 vertex_count, u32 index_buffer_offset, const lightray_model_t* model); // 1
void														lightray_bind_collision_mesh_batch(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_attribute_index); // 2
void														lightray_allocate_collision_mesh_vertex_buffers(lightray_scene_t* scene, u32 capsule_mesh_batch_index, u32 capsule_mesh_index, b32 capsule_is_collision_mesh); // 3
void														lightray_free_collision_mesh_vertex_buffers(lightray_scene_t* scene); // 5

void														lightray_initialize_collision_mesh_sentinel_world_space_vertex_buffer(lightray_scene_t* scene, u32 collision_mesh_batch_index, u32 collision_mesh_index); //4

															// all 4 of these below expect a filled out bitmask (1ull << bit0) | (1ull << bit1) ..
void														lightray_add_collision_layer(lightray_scene_t* scene, u32 collision_attribute_index, u64 collision_layer);
void														lightray_remove_collision_layer(lightray_scene_t* scene, u32 collision_attribute_index, u64 collision_layer);
void														lightray_add_collision_layer_exception(lightray_scene_t* scene, u32 collision_attribute_index, u64 exception_layer);
void														lightray_remove_collision_layer_exception(lightray_scene_t* scene, u32 collision_attribute_index, u64 exception_layer);

void														lightray_enable_collision(lightray_scene_t* scene, u32 collision_attribute_index);
void														lightray_disable_collision(lightray_scene_t* scene, u32 collision_attribute_index);
void														lightray_enable_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 collision_attribute_index, u32 collision_mesh_index);
void														lightray_disable_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 collision_attribute_index, u32 collision_mesh_index);
void														lightray_add_collision_attribute_flags(lightray_scene_t* scene, u32 collision_attribute_index, u32 flags);
void														lightray_set_pierce_layer(lightray_scene_t* scene, u32 collision_attribute_index, u16 layer);

b32														lightray_collision_attribute_collides(const lightray_scene_t* scene, u32 collision_mesh_attribute_index);
b32														lightray_collision_attribute_aabb_collides(const lightray_scene_t* scene, u32 collision_mesh_attribute_index);
b32														lightray_collision_attribute_collision_meshes_collide(const lightray_scene_t* scene, u32 collision_mesh_attribute_index);

lightray_raycast_result_t						lightray_cast_ray(lightray_scene_t* scene, const lightray_raycast_data_t* data, const lightray_render_instance_t* render_instance_buffer);

lightray_raycast_memory_reserve_result_t	lightray_reserve_raycast_memory(lightray_scene_t* scene, u32 raycast_grid_cell_index_per_ray_count, u32 raycast_pierce_layer_test_data_per_ray_count, lightray_raycast_intersection_mode_e intersection_mode);
void														lightray_reclaim_raycast_memory(lightray_scene_t* scene);

void														lightray_set_collision_attribute_entity_kind(lightray_scene_t* scene, u32 collision_mesh_attribute_index, u32 game_side_entity_kind);

lightray_ray_t										lightray_ray(const sunder_v3_t& origin, const sunder_v3_t& direction, f32 distance);

f32														lightray_get_default_cube_grid_scale_x(const lightray_grid_t* grid);
f32														lightray_get_default_cube_grid_scale_y(const lightray_grid_t* grid);
u32														lightray_traverse_grid(u32 index, i32 direction);

void														lightray_handle_grid_traversal_aftermath(lightray_scene_t* scene, u32 cube_entity_index, const lightray_grid_t* grid, const lightray_ray_t* aabb_intersection_ray, u32 row_index, u32 column_index, u32 previous_grid_cell_index, u32* next_row_index, u32* next_column_index, u32* written_grid_cell_index_count, u32 max_grid_cell_index_count, u32 raycast_grid_cell_index_subarena_offset);

bool														lightray_quick_sort_compare_raycast_pierce_layer_test_data_squared_distance_less(const lightray_raycast_pierce_layer_test_data_t* i, const lightray_raycast_pierce_layer_test_data_t* j);

SUNDER_DEFINE_QUICK_SORT_PARTITION_FUNCTION(lightray_raycast_pierce_layer_test_data_t, raycast_pierce_layer_test_data, lightray)
SUNDER_DEFINE_QUICK_SORT_FUNCTION(lightray_raycast_pierce_layer_test_data_t, raycast_pierce_layer_test_data, lightray)

u16														lightray_get_pierce_layer_hit_count(u16 initial_threshold, u16 applied_at_hit);

u32														lightray_get_bone_computed_transform_matrix_buffer_index(const lightray_animation_core_t* animation_core, cstring_literal* bone_name, u32 skeleton_index, u32 instance_index);
sunder_m4_t										lightray_get_bone_m4(const lightray_animation_core_t* animation_core, u32 bone_computed_transform_matrix_buffer_index);

void														lightray_bind_entity_to_camera(lightray_scene_t* scene, u32 self_entity_index, u32 camera_index);
void														lightray_bind_entity_to_bone(lightray_scene_t* scene, u32 self_entity_index, u32 bone_owner_entity_index, u32 bone_computed_transform_matrix_buffer_index);
u32														lightray_get_entity_children_index_buffer_offset(const lightray_scene_t* scene, u32 entity_index);
void														lightray_traverse_entity_binding_chain(lightray_scene_t* scene, u32* current_binding_chain_entity_index, u32 entity_binding_chain_index_buffer_offset, u32* entity_binding_chain_index_count);

void														lightray_add_entity_flags(lightray_scene_t* scene, u32 entity_index, u16 flags);
void														lightray_remove_entity_flags(lightray_scene_t* scene, u32 entity_index, u16 flags);



void														lightray_epa_add_face(lightray_epa_face_t* f, u32 face_count, const lightray_gjk_support_point_t* verts, u32 vert_count);
lightray_epa_result_t							lightray_solve_epa(lightray_gjk_support_point_t* simplex, u32 simplex_size, const sunder_v3_t* vertices_a, u32 vertex_count_a, const sunder_v3_t* vertices_b, u32 vertex_count_b);

sunder_v3_t											lightray_add_impulse(const sunder_v3_t& velocity, const sunder_v3_t& impulse);

b32														lightray_aabb_swept(const sunder_v3_t& movement_direction, const sunder_v3_t& aabb_position_a, const sunder_v3_t& aabb_scale_a, const sunder_v3_t& aabb_position_b, const sunder_v3_t& aabb_scale_b, f32* time);
b32														lightray_capsule_triangle_swept(const lightray_capsule_t* capsule, const sunder_v3_t* tri, sunder_v3_t* normal, f32* t);

void														lightray_cast_ray_cluster(u64 capsule_vertex_count, const u32* capsule_index_buffer, const sunder_v3_t* capsule_local_space_vertex_position_buffer, const sunder_m4_t& capsule_model, const sunder_m4_t& capsule_inverse, sunder_v3_t* capsule_intersection_point_buffer, u64 plane_vertex_count, const sunder_v3_t* plane_local_space_vertex_position_buffer, const sunder_m4_t& plane_model, const sunder_m4_t& plane_inverse, sunder_v3_t* plane_intersection_point_buffer, u32 ray_count, const sunder_v3_t* ray_origin_buffer, const sunder_v3_t& cluster_direction, u64* hit_bitmask_buffer_capsule, u64* hit_bitmask_buffer_plane);
sunder_v3_t											lightray_compute_segment_point(const sunder_v3_t& A, const sunder_v3_t& B, u32 index, f32 step, f32 z_alignment, b32 align_z);
f32														lightray_compute_capsule_to_plane_distance(lightray_capsule_to_plane_distance_computation_data_t* data);

sunder_v3_t											lightray_glm_vec3_to_sunder(const glm::vec3& v);

b32														lightray_cast_convex_hull(lightray_scene_t* scene, u32 overlap_mesh_batch_index, u32 overlap_mesh_index);

void														lightray_advance_animation_playback_data(lightray_animation_core_t* core, u32 animation_playback_command_index, f32 delta_time, lightray_animation_playback_data_advance_kind_e advance_kind);
f32														lightray_compute_animation_blend_step(f32 seconds);

u32														lightray_push_overlap_attribute(lightray_scene_t* scene, u32 entity_index, u32 flags);
u32														lightray_push_overlap_mesh_batch(lightray_scene_t* scene);
void														lightray_bind_overlap_mesh_batch(lightray_scene_t* scene, u32 overlap_mesh_batch_index, u32 overlap_attribute_index);
u32														lightray_push_overlap_mesh(lightray_scene_t* scene, u32 entity_index, u32 overlap_mesh_batch_index, u32 mesh_index_count, u32 mesh_index_buffer_offset, const lightray_model_t* model, u32 flags);
void														lightray_bind_overlap_obb(lightray_scene_t* scene, u32 overlap_mesh_batch_index, u16 obb_overlap_mesh_index, u16 target_overlap_mesh_index);
void														lightray_bind_overlap_aabb(lightray_scene_t* scene, u32 parent_entity_index, u32 aabb_entity_index);

b32														lightray_ray_mesh_intersect(const lightray_ray_t* transformed_ray, f32* t_out, lightray_ray_triangle_face_culling_mode_e culling_mode, sunder_v3_t* tri_out, u32 index_count, const u32* sentinel_index_buffer, u32 sentinel_index_buffer_offset, const sunder_v3_t* sentinel_local_space_vertex_position_buffer, u32 sentinel_local_space_vertex_position_buffer_offset);
lightray_ray_t										lightray_transform_ray_to_ray_space(const lightray_ray_t* ray, lightray_buffer_m4_cache_t* inverse_cache, sunder_bit_buffer_t* inverse_cache_bit_buffer, const sunder_m4_t& model, const sunder_v3_t& scale, u32 mesh_global_index, b32 non_unit_scale, b32 non_default_rotation, b32 attribute_non_default_rotation);
void														lightray_compute_ray_triangle_intersection_metadata(const lightray_ray_t* transformed_ray, const lightray_ray_t* ray, f32 t, const sunder_m4_t& model, const sunder_v3_t& scale, sunder_v3_t* tri, sunder_v3_t* intersection_point, sunder_v3_t* triangle_normal, f32* squared_distance, b32 non_unit_scale, b32 non_default_rotation, b32 attribute_non_default_rotation);
void														lightray_write_hit_attribute(lightray_scene_t* scene, const lightray_raycast_memory_reserve_result_t* memory_reserve_result, f32 squared_distance, u32 attribute_index, const sunder_v3_t& intersection_point, const sunder_v3_t& triangle_normal, u32* written_attribute_count, f32* closest_squared_distance, u32* closest_attribute_index, sunder_v3_t* closest_intersection_point, sunder_v3_t* closest_triangle_normal);

void														lightray_add_overlap_layer(lightray_scene_t* scene, u32 overlap_attribute_index, u32 layer);
void														lightray_set_overlap_attribute_flags(lightray_scene_t* scene, u32 overlap_attribute_index, u32 flags);
void														lightray_enable_overlap_mesh_per_frame_vertex_reprojection(lightray_scene_t* scene, u32 overlap_attribute_index, u32 overlap_mesh_index);

b32														lightray_verify_overlap_mesh(lightray_scene_t* scene, u32 overlap_mesh_flags, u32 overlap_mesh_buffer_offset, u32 overlap_mesh_index, u16 bitmask, u16* chosen_index, b32* is_raw_mesh);
void														lightray_handle_overlap_mesh_vertex_projection(lightray_scene_t* scene, const sunder_m4_t& overlap_attribute_entity_transform, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, u32 overlap_mesh_global_index, b32 overlap_attribute_non_default_rotation);
void														lightray_handle_collision_mesh_vertex_projection(lightray_scene_t* scene, const sunder_m4_t& collision_attribute_entity_transform, sunder_v3_t* sentinel_world_space_vertex_position_buffer, u32 sentinel_vertex_count, u32 collision_mesh_global_index, b32 collision_attribute_non_default_rotation);

b32														lightray_cast_convex_hull(lightray_scene_t* scene, lightray_grid_t* grid, const lightray_render_instance_t* render_instance_buffer, u32 overlap_attribute_index);

void														lightray_push_dynamic_collision_attribute_trace_data(lightray_scene_t* scene, u32 collision_attribute_index, const sunder_v3_t& velocity, const sunder_v3_t& movement_this_frame);
void														lightray_trace_dynamic_collision_attributes(lightray_scene_t* scene, lightray_grid_t* grid);

															// rename to  step xdxdd
b32														lightray_step(lightray_scene_t* scene, lightray_grid_t* grid, u32 collision_attribute_index, const sunder_v3_t& step_position, u32 flags, f32* penetration_depth);
															// only accepts 1 mesh per batch, it being capsule
void														lightray_resolve_sentient_collision(lightray_scene_t* scene, lightray_grid_t* grid);
void														lightray_resolve_primal_collision(lightray_scene_t* scene, lightray_grid_t* grid);

void														lightray_pre_compute_collision_and_overlap_static_meshes_data(lightray_scene_t* scene, lightray_grid_t* grid);

void														lightray_set_entity_position(lightray_scene_t* scene, u32 entity_index, const sunder_v3_t& position);
void														lightray_set_entity_scale(lightray_scene_t* scene, u32 entity_index, const sunder_v3_t& scale);
void														lightray_set_entity_rotation(lightray_scene_t* scene, u32 entity_index, const sunder_quat_t& rotation);
void														lightray_clear_entity_position(lightray_scene_t* scene, u32 entity_index);
void														lightray_clear_entity_scale(lightray_scene_t* scene, u32 entity_index);
void														lightray_clear_entity_rotation(lightray_scene_t* scene, u32 entity_index);

b32														lightray_aabb_in_grid_bounds(const lightray_grid_t* grid, const lightray_grid_cell_aabb_coordinates_t* row_coordinates, const lightray_grid_cell_aabb_coordinates_t* column_coordinates);

void														lightray_extract_transform(const sunder_m4_t& transform, sunder_v3_t* translation, sunder_quat_t* rotation, sunder_v3_t* scale, b32 non_unit_scale, b32 non_default_rotation);
sunder_m4_t										lightray_compute_inherited_transform(const sunder_m4_t& parent_transform, const sunder_v3_t& extracted_translation, const sunder_quat_t& extracted_rotation, const sunder_v3_t& extracted_scale, b32 inherits_translation, b32 inherits_rotation, b32 inherits_scale, b32* identity);
void														lightray_cache_m4(lightray_buffer_m4_cache_t* m4_cache, sunder_bit_buffer_t* bit_buffer, const sunder_m4_t& inverse, u32 mesh_global_index);
sunder_m4_t										lightray_reconstruct_cached_m4(const lightray_m4_cache_t* cache);

sunder_v3_t											lightray_project_capsule_onto_plane(lightray_scene_t* scene, lightray_grid_t* grid, u32 capsule_collision_attribute_index, const sunder_v3_t& pre_raycast_capsule_position, u32 cube_entity_index, const lightray_render_instance_t* render_instance_buffer);