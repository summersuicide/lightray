#pragma once

#include "lightray_core.h"

namespace lightray
{
	namespace es
	{
		enum result
		{
			RESULT_SUCCESS = 0,
			RESULT_INVALID_MESH_INDEX,
			RESULT_INVALID_ENTITY_INDEX,
			RESULT_UNKNOWN_FAILURE,
			RESULT_ENTITY_BUFFER_OVERFLOW,
			RESULT_TOTAL_ENTITY_COUNT_IS_ZERO,
			RESULT_COLLISION_MESH_COUNT_OF_SPECIFIED_MESH_IS_ZERO,
			RESULT_INSTANCE_COUNT_OF_SPECIFIED_MESH_IS_ZERO,
			RESULT_COUNT_OF_MESH_BINDINGS_FOR_SPECIFIED_MESH_HAS_BEEN_EXCEEDED
		};

		enum undefined_value : u32
		{
			UNDEFINED_VALUE_ENTITY_BINDING_INDEX = UINT16_MAX,
			UNDEFINED_VALUE_MESH_BINDING_INDEX,
			UNDEFINED_VALUE_INSTANCE_MODEL_BINDING_INDEX,
			UNDEFINED_VALUE_CONVERTED_INSTANCE_MODEL_BUFFER_INDEX,
			UNDEFINED_VALUE_AABB_INDEX,
			UNDEFINED_VALUE_COLLISION_MESH_INDEX
		};

		struct entity_creation_result_t
		{
			result result = RESULT_UNKNOWN_FAILURE;
			u32 index = 0;
		};

		enum entity_kind : u32
		{
			ENTITY_KIND_UNDEFINED = 0,
			ENTITY_KIND_LIGHT,
			ENTITY_KIND_STATIC_MESH,
			ENTITY_KIND_SKELETAL_MESH,
			ENTITY_KIND_COLLISION_MESH,
			ENTITY_KIND_AABB,
		};

		enum entity_bits
		{
			ENTITY_BITS_IS_COLLIDING = 0
		};

		struct entity_t
		{
			u128_t guid; // might store that in a separate array for cache efficiency
			entity_kind kind; // might store that in a separate array for cache efficiency
			u32 mesh_binding_index;
			u32 instance_model_binding_index;
			u32 entity_binding_index;
			u32 converted_instance_model_buffer_index;
			u32 light_buffer_index;
			flags32 flags; // might store that in a separate array for cache efficiency
		};

		struct scene_allocation_data_t
		{
			u32 instance_model_count = 0;

			u32 total_entity_count = 0;
			u32 total_mesh_count = 0;
			u32 total_light_count = 0;
			sunder::dynamic_alignment_arena_t* arena = nullptr;
			u32* cpu_side_instance_model_buffer_offsets_per_mesh = nullptr;
			u32* cpu_side_instance_model_count_buffer = nullptr;
			u32* wireframe_mesh_count_buffer = nullptr;
		};

		struct scene_t
		{
			u32 mesh_binding_count = 0;
			u32 instance_model_count = 0;
			u32 total_mesh_count = 0;
			u32 entity_count = 0;
			u32 total_entity_count = 0;
			u32 total_light_count = 0;
			u32 light_count = 0;

			entity_t* entity_buffer = nullptr;
			glm::vec3* position_buffer = nullptr;
			glm::vec3* rotation_buffer = nullptr;
			glm::vec3* scale_buffer = nullptr;
			glm::vec3* light_color_buffer = nullptr;

			u32* entity_transform_index_buffer = 0;
			u32 entity_transform_index_buffer_iter = 0;
			u32* cpu_side_instance_model_count_buffer = nullptr;

			u32* wireframe_mesh_count_buffer = nullptr;
			u32** free_instance_model_offsets_per_mesh_buffers = nullptr;

			u32* converted_instance_model_offset_buffer = nullptr;
			u32* cpu_side_instance_model_buffer_offsets_per_mesh = nullptr;
			u32* instance_model_binding_count_buffer = nullptr;
			index* hidden_instance_model_index_buffer = nullptr;
			flags32 visibility_flags = 0;

			u32* aabb_index_buffer = nullptr;
			u32* collision_mesh_index_buffer = nullptr;
			u32 aabb_count = 0;
			u32 collision_mesh_count = 0;
		};
		
		// dont need to deallocate anything, because this function just suballocates from specified arena
		void allocate_scene(scene_t* scene, scene_allocation_data_t* scene_allocation_data);
		entity_creation_result_t create_entity(scene_t* scene, entity_kind kind);
		result bind_mesh(scene_t* scene, u32 entity_index, u32 mesh_index, render_target_kind kind);
		u32 get_free_hidden_instance_model_index(scene_t* scene);
		void hide_entity(model_t* instance_model_buffer, model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, scene_t* scene);
		void unhide_entity(model_t* instance_model_buffer, model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, scene_t* scene);
		void bind_entity(scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index);
		void move_entity(scene_t* scene, u32 self_aabb_index, u32 self_collision_mesh_index, const u32* index_buffer_offsets, const u32*index_count_buffer, const u32* index_buffer, const vertex_t* vertex_buffer, model_t* instance_model_buffer, glm::vec3* self_raw_vertex_positions, glm::vec3* self_projected_vertex_positions, glm::vec3* other_raw_vertex_positions, glm::vec3* other_projected_vertex_positions, u32 entity_index, const glm::vec3& direction);
		u32 get_entity_bound_collision_mesh_index_count(const scene_t* scene, u32 entity_index);
		void get_entity_bound_collision_mesh_indices(const scene_t* scene, u32 entity_index, u32* collision_mesh_index_buffer);
		u32 get_entity_bound_aabb_index_count(const scene_t* scene, u32 collision_mesh_index);
		void get_entity_bound_aabb_indices(const scene_t* scene, u32 collision_mesh_index, u32* aabb_index_buffer);
		void follow_entity_position();
		void follow_entity_scale();
		void follow_entity_rotation();
		void bind_collision_layer();
		void unbind_collision_layer();
	} // es namespace brace
}// lightray namespace brace

/*
#define IS_BIT_SET(array, i) (((array)[(i) / 64] >> ((i) % 64)) & 1ULL)
#define SET_BIT(array, i)    ((array)[(i) / 64] |= (1ULL << ((i) % 64)))
#define CLEAR_BIT(array, i)  ((array)[(i) / 64] &= ~(1ULL << ((i) % 64)))

inline bool is_bit_set(const u64* array, u64 i) {
	return (array[i / 64] >> (i % 64)) & 1ULL;
}

inline void set_bit(u64* array, u64 i) {
	array[i / 64] |= (1ULL << (i % 64));
}

inline void clear_bit(u64* array, u64 i) {
	array[i / 64] &= ~(1ULL << (i % 64));
}
*/