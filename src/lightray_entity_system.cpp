#include "lightray_entity_system.h"

namespace lightray
{
	namespace es
	{
		void allocate_scene(scene_t* scene, scene_allocation_data_t* scene_allocation_data)
		{
			scene->total_entity_count = scene_allocation_data->total_entity_count;
			scene->total_mesh_count = scene_allocation_data->total_mesh_count;
			scene->cpu_side_instance_model_count_buffer = scene_allocation_data->cpu_side_instance_model_count_buffer;
			scene->instance_model_count = scene_allocation_data->instance_model_count;
			scene->wireframe_mesh_count_buffer = scene_allocation_data->wireframe_mesh_count_buffer;
			//scene->instance_count_buffer = scene_allocation_data->instance_count_buffer;

			sunder::arena_suballocation_result_t entity_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(entity_t), alignof(entity_t));
			scene->entity_buffer = (entity_t*)entity_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t position_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(glm::vec3), alignof(glm::vec3));
			scene->position_buffer = (glm::vec3*)position_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t rotation_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(glm::vec3), alignof(glm::vec3));
			scene->rotation_buffer = (glm::vec3*)rotation_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t scale_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(glm::vec3), alignof(glm::vec3));
			scene->scale_buffer = (glm::vec3*)scale_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t light_color_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_light_count * sizeof(glm::vec3), alignof(glm::vec3));
			scene->light_color_buffer = (glm::vec3*)light_color_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t entity_transform_index_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(u32), alignof(u32));
			scene->entity_transform_index_buffer = (u32*)entity_transform_index_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t converted_instance_model_offset_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->instance_model_count * sizeof(u32), alignof(u32));
			scene->converted_instance_model_offset_buffer = (u32*)converted_instance_model_offset_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t cpu_side_instance_model_buffer_offsets_per_mesh_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_mesh_count * sizeof(u32), alignof(u32));
			scene->cpu_side_instance_model_buffer_offsets_per_mesh = (u32*)cpu_side_instance_model_buffer_offsets_per_mesh_suballoc_result.data;

			sunder::arena_suballocation_result_t instance_model_binding_count_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_mesh_count * sizeof(u32), alignof(u32));
			scene->instance_model_binding_count_buffer = (u32*)instance_model_binding_count_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t hidden_instance_model_index_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->instance_model_count * sizeof(index), alignof(index));
			scene->hidden_instance_model_index_buffer = (index*)hidden_instance_model_index_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t free_instance_model_offsets_per_mesh_buffers_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_mesh_count * sizeof(u32*), alignof(u32*));
			scene->free_instance_model_offsets_per_mesh_buffers = (u32**)free_instance_model_offsets_per_mesh_buffers_suballoc_result.data;

			for (u32 i = 0; i < scene_allocation_data->total_mesh_count; i++)
			{
				sunder::arena_suballocation_result_t free_instance_model_offsets_per_mesh_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, (u32)scene->cpu_side_instance_model_count_buffer[i] * sizeof(u32), alignof(u32));
				scene->free_instance_model_offsets_per_mesh_buffers[i] = (u32*)free_instance_model_offsets_per_mesh_buffer_suballoc_result.data;
			}

			sunder::arena_suballocation_result_t aabb_index_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(u32), alignof(u32));
			scene->aabb_index_buffer = (u32*)aabb_index_buffer_suballoc_result.data;

			sunder::arena_suballocation_result_t collision_mesh_index_buffer_suballoc_result = sunder::suballocate_from_arena(scene_allocation_data->arena, scene_allocation_data->total_entity_count * sizeof(u32), alignof(u32));
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

		entity_creation_result_t create_entity(scene_t* scene, entity_kind kind)
		{
			entity_creation_result_t res;
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

		result bind_mesh(scene_t* scene, u32 entity_index, u32 mesh_index, render_target_kind kind)
		{
			if (mesh_index > scene->total_mesh_count - 1) { return RESULT_INVALID_MESH_INDEX; }
			if (scene->cpu_side_instance_model_count_buffer[mesh_index] == 0) { return RESULT_INSTANCE_COUNT_OF_SPECIFIED_MESH_IS_ZERO; }
			if (scene->instance_model_binding_count_buffer[mesh_index] >= scene->cpu_side_instance_model_count_buffer[mesh_index]) { return RESULT_COUNT_OF_MESH_BINDINGS_FOR_SPECIFIED_MESH_HAS_BEEN_EXCEEDED; }
			if (kind == RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] == 0) { return RESULT_COLLISION_MESH_COUNT_OF_SPECIFIED_MESH_IS_ZERO; }

			scene->entity_buffer[entity_index].mesh_binding_index = (u32)mesh_index;
			buffer_index_query_result_t suitable_index{};

			if (kind == RENDER_TARGET_KIND_OPAQUE_MESH)
			{
				suitable_index = query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], 0, scene->cpu_side_instance_model_count_buffer[mesh_index], UINT32_MAX, true);
			}
			
			else if (kind == RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] < scene->cpu_side_instance_model_count_buffer[mesh_index])
			{
				u32 start = scene->cpu_side_instance_model_count_buffer[mesh_index] - scene->wireframe_mesh_count_buffer[mesh_index];
				u32 elm_count = scene->wireframe_mesh_count_buffer[mesh_index]  + start;

				suitable_index = query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], start, elm_count, UINT32_MAX, true);
			}

			else if (kind == RENDER_TARGET_KIND_WIREFRAME_MESH && scene->wireframe_mesh_count_buffer[mesh_index] == scene->cpu_side_instance_model_count_buffer[mesh_index])
			{
				suitable_index = query_buffer_index(scene->free_instance_model_offsets_per_mesh_buffers[mesh_index], 0, scene->cpu_side_instance_model_count_buffer[mesh_index], UINT32_MAX, true);
			}

			scene->converted_instance_model_offset_buffer[scene->mesh_binding_count] = suitable_index.value_at_index; //scene->suitiable_instance_model_offset_per_mesh_buffer[mesh_index];
			scene->entity_buffer[entity_index].converted_instance_model_buffer_index = scene->mesh_binding_count; //scene->converted_instance_model_offset_buffer[scene->mesh_binding_count];
			scene->entity_buffer[entity_index].instance_model_binding_index = (index)scene->converted_instance_model_offset_buffer[scene->mesh_binding_count];
			scene->instance_model_binding_count_buffer[mesh_index] += 1;
			scene->entity_transform_index_buffer[scene->entity_transform_index_buffer_iter] = entity_index;
			scene->entity_transform_index_buffer_iter += 1;

			scene->free_instance_model_offsets_per_mesh_buffers[mesh_index][suitable_index.return_index] = UINT32_MAX;
			scene->mesh_binding_count++;

			return RESULT_SUCCESS;
 		}

		u32 get_free_hidden_instance_model_index(scene_t* scene)
		{
			for (u32 i = 0; i < scene->instance_model_count; i++)
			{
				if (scene->hidden_instance_model_index_buffer[i] == UINT32_MAX)
				{
					return i;
				}
			}

			return UINT32_MAX;
		}

		void hide_entity(model_t* instance_model_buffer, model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, scene_t* scene)
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

		void unhide_entity(model_t* instance_model_buffer, model_t* hidden_instance_model_buffer, u32* instance_model_to_render_count_buffer, u32* instance_model_buffer_offsets_per_mesh, u32 entity_index, es::scene_t* scene)
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

		void bind_entity(scene_t* scene, u32 entity_to_bind_index, u32 entity_to_bind_to_index)
		{
			scene->entity_buffer[entity_to_bind_index].entity_binding_index = entity_to_bind_to_index;
		}

		void move_entity(scene_t* scene, u32 self_aabb_index, u32 self_collision_mesh_index, const u32* index_buffer_offsets, const u32* index_count_buffer, const u32* index_buffer, const vertex_t* vertex_buffer, model_t* instance_model_buffer, glm::vec3* self_raw_vertex_positions, glm::vec3* self_projected_vertex_positions, glm::vec3* other_raw_vertex_positions, glm::vec3* other_projected_vertex_positions, u32 entity_index, const glm::vec3& direction)
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

		u32 get_entity_bound_collision_mesh_index_count(const scene_t* scene, u32 entity_index)
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

		void get_entity_bound_collision_mesh_indices(const scene_t* scene, u32 entity_index, u32* collision_mesh_index_buffer)
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

		u32 get_entity_bound_aabb_index_count(const scene_t* scene, u32 collision_mesh_index)
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

		void get_entity_bound_aabb_indices(const scene_t* scene, u32 collision_mesh_index, u32* aabb_index_buffer)
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
	}
}