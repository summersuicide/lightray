#include "lightray_vulkan_core.h"
#include "sys/stat.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void lightray_vulkan_initialize_core(lightray_vulkan_core_t* core, const lightray_vulkan_core_initialization_data_t* initialization_data)
{
	const u32 core_setup_flags = 0;
	// set the flags depending on the setup data and later during the setup act accordingly

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, SUNDER_TRUE);

	const i32 file_size = lightray_get_file_size("D:/lightray/lightray_entry_point/resources/ShareTech-Regular_layout.json");

	FILE* file = nullptr;
	fopen_s(&file, "D:/lightray/lightray_entry_point/resources/ShareTech-Regular_layout.json", "rb");

	i8* json_buffer = nullptr;
	cJSON* font_root = nullptr;
	const cJSON* atlas = nullptr;
	const cJSON* metrics = nullptr;
	const cJSON* glyphs = nullptr;
	u32 glyph_count = 0;

	if (file != nullptr)
	{
		fseek(file, 0, SEEK_END);

		const u64 file_size = ftell(file);
		rewind(file);

		json_buffer = (i8*)sunder_aligned_halloc(file_size + 1, 8);

		const u64 bytes_read = fread(json_buffer, 1, file_size, file);
		json_buffer[file_size] = 0;

		fclose(file);

		font_root = cJSON_Parse(json_buffer);
		atlas = cJSON_GetObjectItem(font_root, "atlas");
		metrics = cJSON_GetObjectItem(font_root, "metrics");
		glyphs = cJSON_GetObjectItem(font_root, "glyphs");

		const i32 glyph_count_local = cJSON_GetArraySize(glyphs);
		glyph_count = glyph_count_local;
	}

	u32 supported_vulkan_api_version = VK_API_VERSION_1_0;
	const VkResult vulkan_api_version_query_result = vkEnumerateInstanceVersion(&supported_vulkan_api_version);

	const u32 vulkan_api_version_major = VK_API_VERSION_MAJOR(supported_vulkan_api_version);
	const u32 vulkan_api_version_minor = VK_API_VERSION_MINOR(supported_vulkan_api_version);
	const u32 vulkan_api_version_patch = VK_API_VERSION_PATCH(supported_vulkan_api_version);

	cstring_literal* compiled_static_mesh_vertex_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_static_mesh_vertex_shader.vert.spv";
	cstring_literal* compiled_static_mesh_fragment_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_static_mesh_fragment_shader.frag.spv";
	cstring_literal* compiled_textured_static_mesh_vertex_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_textured_static_mesh_vertex_shader.vert.spv";
	cstring_literal* compiled_textured_static_mesh_fragment_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_textured_static_mesh_fragment_shader.frag.spv";
	cstring_literal* compiled_skeletal_mesh_vertex_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_skeletal_mesh_vertex_shader.vert.spv";
	cstring_literal* compiled_skeletal_mesh_fragment_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_skeletal_mesh_fragment_shader.frag.spv";
	cstring_literal* compiled_overlay_vertex_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_overlay_vertex_shader.vert.spv";
	cstring_literal* compiled_overlay_fragment_shader_path = "D:/lightray/lightray_entry_point/shaders/lightray_overlay_fragment_shader.frag.spv";

	const u64 compiled_static_mesh_vertex_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_vertex_shader_path);
	const u64 compiled_static_mesh_fragment_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_fragment_shader_path);
	const u64 compiled_textured_static_mesh_vertex_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_textured_static_mesh_vertex_shader_path);
	const u64 compiled_textured_static_mesh_fragment_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_textured_static_mesh_fragment_shader_path);
	const u64 compiled_skeletal_mesh_vertex_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_skeletal_mesh_vertex_shader_path);
	const u64 compiled_skeletal_mesh_fragment_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_skeletal_mesh_fragment_shader_path);
	const u64 compiled_overlay_vertex_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_overlay_vertex_shader_path);
	const u64 compiled_overlay_fragment_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_overlay_fragment_shader_path);

	core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size = compiled_static_mesh_vertex_shader_byte_code_size;
	core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size = compiled_static_mesh_fragment_shader_byte_code_size;
	core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size = compiled_textured_static_mesh_vertex_shader_byte_code_size;
	core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size = compiled_textured_static_mesh_fragment_shader_byte_code_size;
	core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_VERTEX_SHADER_INDEX].byte_code_size = compiled_skeletal_mesh_vertex_shader_byte_code_size;
	core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_FRAGMENT_SHADER_INDEX].byte_code_size = compiled_skeletal_mesh_fragment_shader_byte_code_size;
	core->shaders[6].byte_code_size = compiled_overlay_vertex_shader_byte_code_size;
	core->shaders[7].byte_code_size = compiled_overlay_fragment_shader_byte_code_size;

	core->monitor = glfwGetPrimaryMonitor();
	core->video_mode = glfwGetVideoMode(core->monitor);

	if (initialization_data->fullscreen_mode)
	{
		core->window = glfwCreateWindow(core->video_mode->width, core->video_mode->height, initialization_data->window_title, core->monitor, nullptr);
	}

	else
	{
		core->window = glfwCreateWindow(initialization_data->window_width, initialization_data->window_height, initialization_data->window_title, nullptr, nullptr);
	}

	core->required_instance_extensions = glfwGetRequiredInstanceExtensions(&core->required_instance_extension_count);

	SUNDER_LOG("\n\n");
	for (u32 i = 0; i < core->required_instance_extension_count; i++)
	{
		SUNDER_LOG(core->required_instance_extensions[i]);
		SUNDER_LOG("\n");
	}

	core->enabled_instance_layers[0] = "VK_LAYER_KHRONOS_validation";

	core->application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	core->application_info.apiVersion = VK_API_VERSION_1_4;
	core->application_info.applicationVersion = VK_API_VERSION_1_4;
	core->application_info.engineVersion = VK_API_VERSION_1_4;
	core->application_info.pApplicationName = "lightray_core";
	core->application_info.pEngineName = "lightray";
	core->application_info.pNext = nullptr;

	core->instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	core->instance_info.ppEnabledExtensionNames = core->required_instance_extensions;
	core->instance_info.enabledExtensionCount = core->required_instance_extension_count;
	core->instance_info.ppEnabledLayerNames = core->enabled_instance_layers;
	core->instance_info.enabledLayerCount = 1;
	core->instance_info.flags = 0;
	core->instance_info.pApplicationInfo = &core->application_info;
	core->instance_info.pNext = nullptr;

	const VkResult instance_creation_result = vkCreateInstance(&core->instance_info, nullptr, &core->instance);
	const VkResult surface_creation_result = glfwCreateWindowSurface(core->instance, core->window, nullptr, &core->surface);

	core->gpu_count = 1;
	const VkResult gpu_enumeration_result = vkEnumeratePhysicalDevices(core->instance, &core->gpu_count, core->gpus);
	vkGetPhysicalDeviceProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->gpu_properties);
	vkGetPhysicalDeviceFeatures(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->gpu_features);
	vkGetPhysicalDeviceMemoryProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->gpu_vram_properties);
	const VkResult surface_capabilities_retrieaval_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->surface_capabilities);

	const u32 storage_buffer_required_alignment = SUNDER_CAST2(u32)core->gpu_properties.limits.minStorageBufferOffsetAlignment;
	const u32 uniform_buffer_required_alignment = SUNDER_CAST2(u32)core->gpu_properties.limits.minUniformBufferOffsetAlignment;

	vkGetPhysicalDeviceQueueFamilyProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->supported_queue_family_property_count, nullptr);
	vkGetPhysicalDeviceSurfaceFormatsKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_surface_format_count, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_swapchain_present_mode_count, nullptr);
	vkEnumerateDeviceExtensionProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], nullptr, &core->available_device_extension_count, nullptr);

	core->swapchain_image_in_use_count = lightray_vulkan_get_supported_swapchain_image_count(&core->surface_capabilities);

	VkBool32 queue_family_supports_swapchain_presentation = VK_FALSE;
	SUNDER_LOG("\n\n");

	for (u32 i = 0; i < core->supported_queue_family_property_count; i++)
	{
		const VkResult queue_family_support_query_result = vkGetPhysicalDeviceSurfaceSupportKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], i, core->surface, &queue_family_supports_swapchain_presentation);

		if (queue_family_support_query_result == VK_SUCCESS && queue_family_supports_swapchain_presentation == VK_TRUE)
		{
			core->swapchain_presentation_supported_queue_family_index_count++;
			queue_family_supports_swapchain_presentation = VK_FALSE;
			SUNDER_LOG(i);
			SUNDER_LOG(" ");
		}
	}

	queue_family_supports_swapchain_presentation = VK_FALSE;

	u64 total_vertex_count = 0;
	u32 total_index_count = 0;
	u32 total_instance_model_count = 0;

	core->total_texture_count = initialization_data->texture_count + initialization_data->msdf_font_atlas_count;
	core->total_texture_count_including_depth_texture = core->total_texture_count + 1;
	core->depth_texture_index = core->total_texture_count;
	core->total_static_mesh_count = initialization_data->static_mesh_count;
	core->total_skeletal_mesh_count = initialization_data->skeletal_mesh_count;
	core->animation_core.total_skeleton_count = initialization_data->skeletal_mesh_count;
	core->total_mesh_count = initialization_data->static_mesh_count + initialization_data->skeletal_mesh_count;
	core->animation_core.total_animation_count = initialization_data->animation_count;
	core->total_camera_count = initialization_data->camera_count;
	core->animation_core.total_playback_command_count = initialization_data->animation_playback_command_count;
	core->overlay_core.total_font_atlas_count = initialization_data->msdf_font_atlas_count;
	core->overlay_core.total_glyph_count = glyph_count;
	core->overlay_core.total_text_element_count = 10;

	const u32 default_textures_range = initialization_data->texture_count;

	const u32 msdf_font_atlases_starting_offset = default_textures_range;
	const u32 msdf_font_atlases_range = default_textures_range + initialization_data->msdf_font_atlas_count;

	const u32 skeletal_mesh_starting_offset = core->total_static_mesh_count;
	const u32 skeletal_mesh_range = core->total_static_mesh_count + core->total_skeletal_mesh_count;

	const u32 animation_starting_offset = core->total_static_mesh_count + core->total_skeletal_mesh_count;
	const u32 animation_range = animation_starting_offset + initialization_data->animation_count;

	const u32 device_local_vram_arena_allocation_count = 2u; // vertex buffer / index buffer
	const u32 host_visible_vram_arena_allocation_count = 4u + core->total_texture_count + core->total_camera_count; // cvp_buffer[..], vertex_buffer, index_buffer, render_instance_buffer, render_instance_glyph_buffer, texture_staging_buffer[..]
	const u32 host_visible_storage_vram_arena_allocation_count = 1u;

	core->device_local_vram_arena_suballocation_starting_offset_count = device_local_vram_arena_allocation_count;
	core->host_visible_vram_arena_suballocation_starting_offset_count = host_visible_vram_arena_allocation_count;

	Assimp::Importer* importers = nullptr;
	const aiScene** scenes = nullptr;
	const aiMesh** meshes = nullptr;
	VkMemoryRequirements* texture_vram_requirements_buffer = nullptr;
	u32* vram_type_index_buffer = nullptr;
	u32* vram_type_index_filter_buffer = nullptr;
	lightray_vulkan_texture_sampler_metadata_t* texture_sampler_metadata_buffer = nullptr;
	lightray_vulkan_texture_sampler_metadata_t* texture_sampler_metadata_filter_buffer = nullptr;
	u64* aligned_allocation_size_buffer = nullptr;

	{
		const sunder_arena_suballocation_result_t importers_suballocation_result = sunder_suballocate_from_arena_debug(initialization_data->arena, sunder_compute_array_size_in_bytes(8, core->total_mesh_count + core->animation_core.total_animation_count), 8);
		importers = SUNDER_CAST(Assimp::Importer*, importers_suballocation_result.data);

		const sunder_arena_suballocation_result_t scenes_suballocation_result = sunder_suballocate_from_arena_debug(initialization_data->arena, sunder_compute_array_size_in_bytes(8, core->total_mesh_count + core->animation_core.total_animation_count), 8);
		scenes = SUNDER_CAST(const aiScene**, scenes_suballocation_result.data);

		const sunder_arena_suballocation_result_t meshes_suballocation_result = sunder_suballocate_from_arena_debug(initialization_data->arena, sunder_compute_array_size_in_bytes(8, core->total_mesh_count), 8);
		meshes = SUNDER_CAST(const aiMesh**, meshes_suballocation_result.data);

		const sunder_arena_suballocation_result_t aligned_allocation_size_buffer_suballocation_result = sunder_suballocate_from_arena_debug(initialization_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), LIGHTRAY_VULKAN_ALIGNED_ALLOCATION_SIZE_BUFFER_LENGTH), alignof(u64));
		aligned_allocation_size_buffer = SUNDER_CAST(u64*, aligned_allocation_size_buffer_suballocation_result.data);
	}

	for (u32 i = 0; i < core->total_mesh_count + core->animation_core.total_animation_count; i++)
	{
		new(&importers[i]) Assimp::Importer();
	}

	for (u32 i = 0; i < initialization_data->static_mesh_count; i++)
	{
		scenes[i] = importers[i].ReadFile(initialization_data->static_mesh_metadata_buffer[i].path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
		meshes[i] = scenes[i]->mMeshes[0];
	}

	u32 skeletal_mesh_import_iter = 0;

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		scenes[i] = importers[i].ReadFile(initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_import_iter].path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_LimitBoneWeights | aiProcess_PopulateArmatureData);
		meshes[i] = scenes[i]->mMeshes[0];
		skeletal_mesh_import_iter++;
	}

	u32 animation_path_buffer_iter = 0;

	for (u32 i = animation_starting_offset; i < animation_range; i++)
	{
		scenes[i] = importers[i].ReadFile(initialization_data->animation_path_buffer[animation_path_buffer_iter], aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals | aiProcess_LimitBoneWeights | aiProcess_PopulateArmatureData);
		animation_path_buffer_iter++;
	}

	SUNDER_LOG("\n\nstatic mesh vertex data\n");

	u32 static_mesh_index_count_iter = 0;

	for (u32 i = 0; i < core->total_static_mesh_count; i++)
	{
		static_mesh_index_count_iter = lightray_assimp_get_mesh_index_count(meshes[i]);

		SUNDER_LOG("\nindex count: ");
		SUNDER_LOG(static_mesh_index_count_iter);
		SUNDER_LOG("\nvertex count: ");
		SUNDER_LOG(meshes[i]->mNumVertices);
		SUNDER_LOG("\n\n");

		total_index_count += static_mesh_index_count_iter;
		total_vertex_count += meshes[i]->mNumVertices;
		total_instance_model_count += initialization_data->static_mesh_metadata_buffer[i].instance_count;

		static_mesh_index_count_iter = 0;
	}

	const u32 skeletal_mesh_instance_starting_offset = total_instance_model_count;

	SUNDER_LOG("\n\nskeletal mesh vertex data\n");

	u32 skeletal_mesh_metadata_buffer_iter = 0;
	u32 skeletal_mesh_index_count_iter = 0;

	u32 largest_skeleton_bone_count = 0;

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		skeletal_mesh_index_count_iter = lightray_assimp_get_mesh_index_count(meshes[i]);

		SUNDER_LOG("\nindex count: ");
		SUNDER_LOG(skeletal_mesh_index_count_iter);
		SUNDER_LOG("\nvertex count: ");
		SUNDER_LOG(meshes[i]->mNumVertices);
		SUNDER_LOG("\n\n");

		total_index_count += skeletal_mesh_index_count_iter;
		total_vertex_count += meshes[i]->mNumVertices;

		total_instance_model_count += initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count;

		core->animation_core.total_skeletal_mesh_instance_count += initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count;
		const u32 bone_count_per_skeleton_mesh_with_respect_to_instance_count = lightray_compute_skeletal_mesh_bone_count_with_respect_to_instance_count(meshes[i]->mNumBones, initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count);
		core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count += bone_count_per_skeleton_mesh_with_respect_to_instance_count;

		if (largest_skeleton_bone_count < meshes[i]->mNumBones)
		{
			largest_skeleton_bone_count = meshes[i]->mNumBones;
		}

		core->animation_core.total_bone_count += meshes[i]->mNumBones;
		skeletal_mesh_metadata_buffer_iter++;

		skeletal_mesh_index_count_iter = 0;
	}

	total_index_count += 6;
	total_vertex_count += 4;

	core->animation_core.largest_skeleton_bone_count = largest_skeleton_bone_count;

	u64 total_string_byte_code_size = 0;

	for (u32 i = animation_starting_offset; i < animation_range; i++)
	{
		const aiAnimation* current_animation = scenes[i]->mAnimations[0];
		core->animation_core.total_animation_channel_count += current_animation->mNumChannels;

		for (u32 j = 0; j < current_animation->mNumChannels; j++)
		{
			total_string_byte_code_size = sunder_update_aligned_value_u64(total_string_byte_code_size, current_animation->mChannels[j]->mNodeName.length + 1, initialization_data->arena_alignment);

			SUNDER_LOG(current_animation->mChannels[j]->mNodeName.C_Str());
			SUNDER_LOG("\n");
			core->animation_core.total_animation_position_key_count += current_animation->mChannels[j]->mNumPositionKeys;
			core->animation_core.total_animation_rotation_key_count += current_animation->mChannels[j]->mNumRotationKeys;
			core->animation_core.total_animation_scale_key_count += current_animation->mChannels[j]->mNumScalingKeys;
		}
	}

	u32 total_node_count = 0;

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		lightray_assimp_get_node_hierarchy_metadata(scenes[i]->mRootNode, &total_node_count, &total_string_byte_code_size, initialization_data->arena_alignment);
	}

	core->animation_core.total_node_count = total_node_count;

	for (u32 sm = skeletal_mesh_starting_offset; sm < skeletal_mesh_range; sm++)
	{
		const u32 current_bone_count = meshes[sm]->mNumBones;

		for (u32 b = 0; b < current_bone_count; b++)
		{
			total_string_byte_code_size = sunder_update_aligned_value_u64(total_string_byte_code_size, meshes[sm]->mBones[b]->mName.length + 1, initialization_data->arena_alignment);
		}
	}

	total_string_byte_code_size = sunder_align64(total_string_byte_code_size, initialization_data->arena_alignment);
	core->cpu_side_instance_count = total_instance_model_count;

	{
		aligned_allocation_size_buffer[0]	= sunder_compute_aligned_allocation_size(sizeof(lightray_camera_t), core->total_camera_count, initialization_data->arena_alignment); // camera_buffer
		aligned_allocation_size_buffer[1] = sunder_compute_aligned_allocation_size(sizeof(lightray_cvp_t), core->total_camera_count, initialization_data->arena_alignment); // cvp_buffer
		aligned_allocation_size_buffer[2] = sunder_compute_aligned_allocation_size(sizeof(lightray_animation_playback_command_t), initialization_data->animation_playback_command_count, initialization_data->arena_alignment); // animaiton_playback_command_buffer
		aligned_allocation_size_buffer[3] = sunder_compute_aligned_allocation_size(sizeof(lightray_node_t), total_node_count, initialization_data->arena_alignment); // node_buffer
		aligned_allocation_size_buffer[4] = sunder_compute_aligned_allocation_size(sizeof(lightray_bone_t), core->animation_core.total_bone_count, initialization_data->arena_alignment); // bone_buffer
		aligned_allocation_size_buffer[5] = sunder_compute_aligned_allocation_size(sizeof(sunder_string_t), core->animation_core.total_bone_count, initialization_data->arena_alignment); // bone_names
		aligned_allocation_size_buffer[6] = sunder_compute_aligned_allocation_size(sizeof(sunder_string_t), core->animation_core.total_animation_channel_count, initialization_data->arena_alignment); // animation_channel_names
		aligned_allocation_size_buffer[7] = sunder_compute_aligned_allocation_size(sizeof(sunder_string_t), total_node_count, initialization_data->arena_alignment); // node_names
		aligned_allocation_size_buffer[8] = sunder_compute_aligned_allocation_size(sizeof(glm::mat4), core->animation_core.total_skeleton_count, initialization_data->arena_alignment); // global_root_inverse_matrix_buffer
		aligned_allocation_size_buffer[9] = sunder_compute_aligned_allocation_size(sizeof(glm::mat4), core->animation_core.total_bone_count, initialization_data->arena_alignment); // bone_bind_pose_matrix_buffer
		aligned_allocation_size_buffer[10] = sunder_compute_aligned_allocation_size(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count, initialization_data->arena_alignment); // computed_bone_matrix_buffer
		aligned_allocation_size_buffer[11] = sunder_compute_aligned_allocation_size(sizeof(lightray_skeleton_t), core->animation_core.total_skeleton_count, initialization_data->arena_alignment); // skeleton_buffer
		aligned_allocation_size_buffer[12] = sunder_compute_aligned_allocation_size(sizeof(lightray_animation_channel_t), core->animation_core.total_animation_channel_count, initialization_data->arena_alignment); // animation_channel_buffer
		aligned_allocation_size_buffer[13] = sunder_compute_aligned_allocation_size(sizeof(lightray_animation_t), core->animation_core.total_animation_count, initialization_data->arena_alignment); // animation_buffer
		aligned_allocation_size_buffer[14] = sunder_compute_aligned_allocation_size(sizeof(lightray_animation_key_vec3_t), core->animation_core.total_animation_position_key_count, initialization_data->arena_alignment); // animation_position_key_buffer
		aligned_allocation_size_buffer[15] = sunder_compute_aligned_allocation_size(sizeof(lightray_animation_key_quat_t), core->animation_core.total_animation_rotation_key_count, initialization_data->arena_alignment); // animation_rotation_key_buffer
		aligned_allocation_size_buffer[16] = sunder_compute_aligned_allocation_size(sizeof(lightray_animation_key_vec3_t), core->animation_core.total_animation_scale_key_count, initialization_data->arena_alignment); // animation_scale_key_buffer
		aligned_allocation_size_buffer[17] = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, initialization_data->arena_alignment); // cpu_side_vertex buffer 
		aligned_allocation_size_buffer[18] = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, initialization_data->arena_alignment); // cpu_side_index_buffer
		aligned_allocation_size_buffer[19] = sunder_compute_aligned_allocation_size(sizeof(lightray_render_instance_t), total_instance_model_count, initialization_data->arena_alignment); // render_instance_buffer
		aligned_allocation_size_buffer[20] = sunder_compute_aligned_allocation_size(sizeof(u64), device_local_vram_arena_allocation_count, initialization_data->arena_alignment); // device_local_vram_arena_starting_offsets
		aligned_allocation_size_buffer[21] = sunder_compute_aligned_allocation_size(sizeof(VkExtensionProperties), core->available_device_extension_count, initialization_data->arena_alignment); // available_device_extensions
		aligned_allocation_size_buffer[22] = sunder_compute_aligned_allocation_size(sizeof(u32), core->swapchain_presentation_supported_queue_family_index_count, initialization_data->arena_alignment); // swapchain_presentation_supported_queue_family_indices
		aligned_allocation_size_buffer[23] = sunder_compute_aligned_allocation_size(sizeof(VkImage), core->swapchain_image_in_use_count * 7, initialization_data->arena_alignment); // swapchain_images / swapchain_image_related_data / 7 is the number of swapchain image related fields that needs to be the count of swapchain_image_in_use_count

		u64 aligned_shader_byte_code_allocation_size_buffer[LIGHTRAY_VULKAN_MANDATORY_SHADER_COUNT]{};
		aligned_shader_byte_code_allocation_size_buffer[0] = compiled_static_mesh_vertex_shader_byte_code_size;
		aligned_shader_byte_code_allocation_size_buffer[1] = compiled_static_mesh_fragment_shader_byte_code_size;
		aligned_shader_byte_code_allocation_size_buffer[2] = compiled_textured_static_mesh_vertex_shader_byte_code_size;
		aligned_shader_byte_code_allocation_size_buffer[3] = compiled_textured_static_mesh_fragment_shader_byte_code_size;
		aligned_shader_byte_code_allocation_size_buffer[4] = compiled_skeletal_mesh_vertex_shader_byte_code_size;
		aligned_shader_byte_code_allocation_size_buffer[5] = compiled_skeletal_mesh_fragment_shader_byte_code_size;
		aligned_shader_byte_code_allocation_size_buffer[6] = compiled_overlay_vertex_shader_byte_code_size;
		aligned_shader_byte_code_allocation_size_buffer[7] = compiled_overlay_fragment_shader_byte_code_size;

		aligned_allocation_size_buffer[24] = sunder_accumulate_aligned_allocation_size(aligned_shader_byte_code_allocation_size_buffer, LIGHTRAY_VULKAN_MANDATORY_SHADER_COUNT, initialization_data->arena_alignment); // shader_byte_code_combined
		aligned_allocation_size_buffer[25] = sunder_compute_aligned_allocation_size(sizeof(VkQueueFamilyProperties), core->supported_queue_family_property_count, initialization_data->arena_alignment); // queue_family_properties
		aligned_allocation_size_buffer[26] = sunder_compute_aligned_allocation_size(sizeof(VkSurfaceFormatKHR), core->supported_surface_format_count, initialization_data->arena_alignment); // supported_surface_formats
		aligned_allocation_size_buffer[27] = sunder_compute_aligned_allocation_size(sizeof(VkPresentModeKHR), core->supported_swapchain_present_mode_count, initialization_data->arena_alignment); // supported_swapchain_present_modes
		aligned_allocation_size_buffer[28] = sunder_compute_aligned_allocation_size(sizeof(lightray_vulkan_mesh_render_pass_data_t), core->total_mesh_count, initialization_data->arena_alignment); // mesh_render_pass_data_buffer
		aligned_allocation_size_buffer[29] = sunder_compute_aligned_allocation_size(sizeof(u32), core->total_mesh_count, initialization_data->arena_alignment); // mesh_render_pass_data_mapping_buffer
		aligned_allocation_size_buffer[30] = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(host_visible_vram_arena_allocation_count, initialization_data->arena_alignment); // host_visible_vram_arena_metadata
		aligned_allocation_size_buffer[31] = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(device_local_vram_arena_allocation_count, initialization_data->arena_alignment); // device_local_vram_arena_metadata
		aligned_allocation_size_buffer[32] = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(host_visible_storage_vram_arena_allocation_count, initialization_data->arena_alignment); // host_visible_storage_vram_arena_metadata
		aligned_allocation_size_buffer[33] = total_string_byte_code_size;
		aligned_allocation_size_buffer[34] = sunder_compute_aligned_allocation_size(sizeof(lightray_font_atlas_t), 1, initialization_data->arena_alignment); // font_atlas buffer
		aligned_allocation_size_buffer[35] = sunder_compute_aligned_allocation_size(sizeof(lightray_glyph_t), 256, initialization_data->arena_alignment); // glyph buffer
		aligned_allocation_size_buffer[36] = sunder_compute_aligned_allocation_size(sizeof(lightray_render_instance_glyph_t), 256, initialization_data->arena_alignment); // render instance glyph buffer
		aligned_allocation_size_buffer[37] = sunder_compute_aligned_allocation_size(sizeof(lightray_vulkan_texture_t), core->total_texture_count_including_depth_texture, initialization_data->arena_alignment); // texture buffer
		aligned_allocation_size_buffer[38] = sunder_compute_aligned_allocation_size(sizeof(VkMemoryRequirements), core->total_texture_count_including_depth_texture, initialization_data->arena_alignment); // vram requirements buffer
		aligned_allocation_size_buffer[39] = sunder_compute_aligned_allocation_size(sizeof(u32), core->total_texture_count_including_depth_texture, initialization_data->arena_alignment); // vram type index buffer
		aligned_allocation_size_buffer[40] = sunder_compute_aligned_allocation_size(sizeof(u32), core->total_texture_count_including_depth_texture, initialization_data->arena_alignment); // vram type index filter buffer
		aligned_allocation_size_buffer[41] = sunder_compute_aligned_allocation_size(sizeof(lightray_vulkan_texture_sampler_metadata_t), core->total_texture_count * 2, initialization_data->arena_alignment); // texture sampler metadata, filter buffers
		aligned_allocation_size_buffer[42] = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorSet), core->total_texture_count, initialization_data->arena_alignment); // descriptor sets
		aligned_allocation_size_buffer[43] = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorSetLayout), core->total_texture_count, initialization_data->arena_alignment); // copy descriptor set layouts
		aligned_allocation_size_buffer[44] = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorImageInfo), core->total_texture_count, initialization_data->arena_alignment); // combined image sampler info buffer
		aligned_allocation_size_buffer[45] = sunder_compute_aligned_allocation_size(sizeof(u64), host_visible_vram_arena_allocation_count, initialization_data->arena_alignment); // host_visible_vram_arena_suballocation_starting_offsets 
		aligned_allocation_size_buffer[46] = sunder_compute_aligned_allocation_size(sizeof(u32), core->total_mesh_count, initialization_data->arena_alignment); // mesh_render_pass_data_reordering_helper_buffer 
		aligned_allocation_size_buffer[47] = sunder_compute_aligned_allocation_size(sizeof(lightray_overlay_text_element_t), core->overlay_core.total_text_element_count, initialization_data->arena_alignment); // overlay text element buffer

		SUNDER_LOG("\n\naligned_allocation_size_buffer: \n");
		for (u32 i = 0; i < LIGHTRAY_VULKAN_ALIGNED_ALLOCATION_SIZE_BUFFER_LENGTH; i++)
		{
			SUNDER_LOG(aligned_allocation_size_buffer[i]);
			SUNDER_LOG("\n");
		}

		const u64 general_purpose_ram_arena_allocation_size = sunder_accumulate_aligned_allocation_size(aligned_allocation_size_buffer, LIGHTRAY_VULKAN_ALIGNED_ALLOCATION_SIZE_BUFFER_LENGTH, initialization_data->arena_alignment);

		const sunder_arena_result general_purpose_ram_arena_allocation_result = sunder_allocate_arena(&core->general_purpose_ram_arena, general_purpose_ram_arena_allocation_size, initialization_data->arena_alignment);
	
		const sunder_arena_suballocation_result_t available_device_extensions_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkExtensionProperties), core->available_device_extension_count), alignof(VkExtensionProperties));
		core->available_device_extensions = SUNDER_CAST(VkExtensionProperties*, available_device_extensions_suballocation_result.data);

		const sunder_arena_suballocation_result_t  swapchain_presentation_supported_queue_family_indices_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->swapchain_presentation_supported_queue_family_index_count), alignof(u32));
		core->swapchain_presentation_supported_queue_family_indices = SUNDER_CAST(u32*, swapchain_presentation_supported_queue_family_indices_suballocation_result.data);

		const sunder_arena_suballocation_result_t swapchain_images_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkImage), core->swapchain_image_in_use_count), alignof(VkImage));
		core->swapchain_images = SUNDER_CAST(VkImage*, swapchain_images_suballocation_result.data);

		const sunder_arena_suballocation_result_t swapchain_image_views_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkImageView), core->swapchain_image_in_use_count), alignof(VkImageView));
		core->swapchain_image_views = SUNDER_CAST(VkImageView*, swapchain_image_views_suballocation_result.data);

		const sunder_arena_suballocation_result_t swapchain_framebuffers_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkFramebuffer), core->swapchain_image_in_use_count), alignof(VkFramebuffer));
		core->swapchain_framebuffers = SUNDER_CAST(VkFramebuffer*, swapchain_framebuffers_suballocation_result.data);

		const sunder_arena_suballocation_result_t image_finished_rendering_semaphores_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkSemaphore), core->swapchain_image_in_use_count), alignof(VkSemaphore));
		core->image_finished_rendering_semaphores = SUNDER_CAST(VkSemaphore*, image_finished_rendering_semaphores_suballocation_result.data);

		const sunder_arena_suballocation_result_t image_available_for_rendering_semaphores_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkSemaphore), core->swapchain_image_in_use_count), alignof(VkSemaphore));
		core->image_available_for_rendering_semaphores = SUNDER_CAST(VkSemaphore*, image_available_for_rendering_semaphores_suballocation_result.data);

		const sunder_arena_suballocation_result_t inflight_fences_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkFence), core->swapchain_image_in_use_count), alignof(VkFence));
		core->inflight_fences = SUNDER_CAST(VkFence*, inflight_fences_suballocation_result.data);

		const sunder_arena_suballocation_result_t render_command_buffers_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkCommandBuffer), core->swapchain_image_in_use_count), alignof(VkCommandBuffer));
		core->render_command_buffers = SUNDER_CAST(VkCommandBuffer*, render_command_buffers_suballocation_result.data);

		const sunder_arena_suballocation_result_t supported_queue_family_properties_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkQueueFamilyProperties), core->supported_queue_family_property_count), alignof(VkQueueFamilyProperties));
		core->supported_queue_family_properties = SUNDER_CAST(VkQueueFamilyProperties*, supported_queue_family_properties_suballocation_result.data);

		const sunder_arena_suballocation_result_t supported_surface_formats_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkSurfaceFormatKHR), core->supported_surface_format_count), alignof(VkSurfaceFormatKHR));
		core->supported_surface_formats = SUNDER_CAST(VkSurfaceFormatKHR*, supported_surface_formats_suballocation_result.data);

		const sunder_arena_suballocation_result_t swapchain_present_modes_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkPresentModeKHR), core->supported_swapchain_present_mode_count), alignof(VkPresentModeKHR));
		core->supported_swapchain_present_modes = SUNDER_CAST(VkPresentModeKHR*, swapchain_present_modes_suballocation_result.data);

		const sunder_arena_suballocation_result_t static_mesh_vertex_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_static_mesh_vertex_shader_byte_code_size, alignof(i8));
		core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code = SUNDER_CAST(i8*, static_mesh_vertex_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t static_mesh_fragment_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_static_mesh_fragment_shader_byte_code_size, alignof(i8));
		core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code = SUNDER_CAST(i8*, static_mesh_fragment_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t textured_static_mesh_vertex_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_textured_static_mesh_vertex_shader_byte_code_size, alignof(i8));
		core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code = SUNDER_CAST(i8*, textured_static_mesh_vertex_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t textured_static_mesh_fragment_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_textured_static_mesh_fragment_shader_byte_code_size, alignof(i8));
		core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code = SUNDER_CAST(i8*, textured_static_mesh_fragment_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t skeletal_mesh_vertex_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_skeletal_mesh_vertex_shader_byte_code_size, alignof(i8));
		core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_VERTEX_SHADER_INDEX].byte_code = SUNDER_CAST(i8*, skeletal_mesh_vertex_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t skeletal_mesh_fragment_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_skeletal_mesh_fragment_shader_byte_code_size, alignof(i8));
		core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_FRAGMENT_SHADER_INDEX].byte_code = SUNDER_CAST(i8*, skeletal_mesh_fragment_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t compiled_overlay_vertex_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_overlay_vertex_shader_byte_code_size, alignof(i8));
		core->shaders[6].byte_code = SUNDER_CAST(i8*, compiled_overlay_vertex_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t compiled_overlay_fragment_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_overlay_fragment_shader_byte_code_size, alignof(i8));
		core->shaders[7].byte_code = SUNDER_CAST(i8*, compiled_overlay_fragment_shader_byte_code_suballocation_result.data);

		const sunder_arena_suballocation_result_t cpu_side_vertex_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count), alignof(lightray_vertex_t));
		core->cpu_side_vertex_buffer = SUNDER_CAST(lightray_vertex_t*, cpu_side_vertex_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t cpu_side_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count), alignof(u32));
		core->cpu_side_index_buffer = SUNDER_CAST(u32*, cpu_side_index_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t instance_render_instance_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), total_instance_model_count), alignof(lightray_render_instance_t));
		core->cpu_side_render_instance_buffer = SUNDER_CAST(lightray_render_instance_t*, instance_render_instance_suballocation_result.data);

		const sunder_arena_suballocation_result_t mesh_render_pass_data_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vulkan_mesh_render_pass_data_t), core->total_mesh_count), alignof(lightray_vulkan_mesh_render_pass_data_t));
		core->mesh_render_pass_data_buffer = SUNDER_CAST(lightray_vulkan_mesh_render_pass_data_t*, mesh_render_pass_data_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t mesh_render_pass_data_mapping_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->total_mesh_count), alignof(u32));
		core->mesh_render_pass_data_mapping_buffer = SUNDER_CAST(u32*, mesh_render_pass_data_mapping_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t device_local_vram_arena_suballocation_starting_offsets_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u64), device_local_vram_arena_allocation_count), alignof(u64));
		core->device_local_vram_arena_suballocation_starting_offsets = SUNDER_CAST(u64*, device_local_vram_arena_suballocation_starting_offsets_suballocation_result.data);

		const sunder_arena_suballocation_result_t node_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_node_t), core->animation_core.total_node_count), alignof(lightray_node_t));
		core->animation_core.node_buffer = SUNDER_CAST(lightray_node_t*, node_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t animation_playback_command_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_animation_playback_command_t), initialization_data->animation_playback_command_count), alignof(lightray_animation_playback_command_t));
		core->animation_core.playback_command_buffer = SUNDER_CAST(lightray_animation_playback_command_t*, animation_playback_command_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t computed_bone_matrix_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count), alignof(glm::mat4));
		core->animation_core.computed_bone_matrix_buffer = SUNDER_CAST(glm::mat4*, computed_bone_matrix_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t global_root_inverse_matrix_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_skeleton_count), alignof(glm::mat4));
		core->animation_core.global_root_inverse_matrix_buffer = SUNDER_CAST(glm::mat4*, global_root_inverse_matrix_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t bone_bind_pose_matrix_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_bone_count), alignof(glm::mat4));
		core->animation_core.bone_bind_pose_matrix_buffer = SUNDER_CAST(glm::mat4*, bone_bind_pose_matrix_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t bone_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_bone_t), core->animation_core.total_bone_count), alignof(lightray_bone_t));
		core->animation_core.bone_buffer = SUNDER_CAST(lightray_bone_t*, bone_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t skeleton_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_skeleton_t), core->animation_core.total_skeleton_count), alignof(lightray_skeleton_t));
		core->animation_core.skeleton_buffer = SUNDER_CAST(lightray_skeleton_t*, skeleton_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t none_names_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(sunder_string_t), total_node_count), alignof(sunder_string_t));
		core->animation_core.node_names = SUNDER_CAST(sunder_string_t*, none_names_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t bone_names_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(sunder_string_t), core->animation_core.total_bone_count), alignof(sunder_string_t));
		core->animation_core.bone_names = SUNDER_CAST(sunder_string_t*, bone_names_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t animation_channel_names_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(sunder_string_t), core->animation_core.total_animation_channel_count), alignof(sunder_string_t));
		core->animation_core.animation_channel_names = SUNDER_CAST(sunder_string_t*, animation_channel_names_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t animation_position_key_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_animation_key_vec3_t), core->animation_core.total_animation_position_key_count), alignof(lightray_animation_key_vec3_t));
		core->animation_core.animation_position_key_buffer = SUNDER_CAST(lightray_animation_key_vec3_t*, animation_position_key_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t animation_rotation_key_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_animation_key_quat_t), core->animation_core.total_animation_rotation_key_count), alignof(lightray_animation_key_quat_t));
		core->animation_core.animation_rotation_key_buffer = SUNDER_CAST(lightray_animation_key_quat_t*, animation_rotation_key_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t animation_scale_key_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_animation_key_vec3_t), core->animation_core.total_animation_scale_key_count), alignof(lightray_animation_key_vec3_t));
		core->animation_core.animation_scale_key_buffer = SUNDER_CAST(lightray_animation_key_vec3_t*, animation_scale_key_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t animation_channel_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_animation_channel_t), core->animation_core.total_animation_channel_count), alignof(lightray_animation_channel_t));
		core->animation_core.animation_channel_buffer = SUNDER_CAST(lightray_animation_channel_t*, animation_channel_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t animation_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_animation_t), core->animation_core.total_animation_count), alignof(lightray_animation_t));
		core->animation_core.animation_buffer = SUNDER_CAST(lightray_animation_t*, animation_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t camera_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_camera_t), core->total_camera_count), alignof(lightray_camera_t));
		core->camera_buffer = SUNDER_CAST(lightray_camera_t*, camera_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t cvp_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_cvp_t), core->total_camera_count), alignof(lightray_cvp_t));
		core->cvp_buffer = SUNDER_CAST(lightray_cvp_t*, cvp_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t font_atlas_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_font_atlas_t), 1), alignof(lightray_font_atlas_t));
		core->overlay_core.font_atlas_buffer = SUNDER_CAST(lightray_font_atlas_t*, font_atlas_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t glyph_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_glyph_t), 256), alignof(lightray_glyph_t));
		core->overlay_core.glyph_buffer = SUNDER_CAST(lightray_glyph_t*, glyph_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t render_instance_glyph_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), 256), alignof(lightray_render_instance_glyph_t));
		core->overlay_core.render_instance_glyph_buffer = SUNDER_CAST(lightray_render_instance_glyph_t*, render_instance_glyph_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t overlay_text_element_buffer_suballocatinon_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_overlay_text_element_t), core->overlay_core.total_text_element_count), alignof(lightray_overlay_text_element_t));
		core->overlay_core.text_element_buffer = SUNDER_CAST(lightray_overlay_text_element_t*, overlay_text_element_buffer_suballocatinon_result.data);

		const sunder_arena_suballocation_result_t texture_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vulkan_texture_t), core->total_texture_count_including_depth_texture), alignof(lightray_vulkan_texture_t));
		core->texture_buffer = SUNDER_CAST(lightray_vulkan_texture_t*, texture_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t texture_vram_requirements_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkMemoryRequirements), core->total_texture_count_including_depth_texture), alignof(VkMemoryRequirements));
		texture_vram_requirements_buffer = SUNDER_CAST(VkMemoryRequirements*, texture_vram_requirements_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t vram_type_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->total_texture_count_including_depth_texture), alignof(u32));
		vram_type_index_buffer = SUNDER_CAST(u32*, vram_type_index_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t vram_type_index_filter_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->total_texture_count_including_depth_texture), alignof(u32));
		vram_type_index_filter_buffer = SUNDER_CAST(u32*, vram_type_index_filter_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t texture_sampler_metadata_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vulkan_texture_sampler_metadata_t), core->total_texture_count), alignof(lightray_vulkan_texture_sampler_metadata_t));
		texture_sampler_metadata_buffer = SUNDER_CAST(lightray_vulkan_texture_sampler_metadata_t*, texture_sampler_metadata_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t texture_sampler_metadata_filter_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vulkan_texture_sampler_metadata_t), core->total_texture_count), alignof(lightray_vulkan_texture_sampler_metadata_t));
		texture_sampler_metadata_filter_buffer = SUNDER_CAST(lightray_vulkan_texture_sampler_metadata_t*, texture_sampler_metadata_filter_buffer_suballocation_result.data);

		const sunder_arena_suballocation_result_t descriptor_sets_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkDescriptorSet), core->total_texture_count), alignof(VkDescriptorSet));
		core->descriptor_sets = SUNDER_CAST(VkDescriptorSet*, descriptor_sets_suballocation_result.data);

		const sunder_arena_suballocation_result_t copy_descriptor_set_layouts_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkDescriptorSetLayout), core->total_texture_count), alignof(VkDescriptorSetLayout));
		core->copy_descriptor_set_layouts = SUNDER_CAST(VkDescriptorSetLayout*, copy_descriptor_set_layouts_suballocation_result.data);

		const sunder_arena_suballocation_result_t descriptor_combined_sampler_infos_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkDescriptorImageInfo), core->total_texture_count), alignof(VkDescriptorImageInfo));
		core->descriptor_combined_sampler_infos = SUNDER_CAST(VkDescriptorImageInfo*, descriptor_combined_sampler_infos_suballocation_result.data);

		const sunder_arena_suballocation_result_t host_visible_vram_arena_suballocation_starting_offsets_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u64), host_visible_vram_arena_allocation_count), alignof(u64));
		core->host_visible_vram_arena_suballocation_starting_offsets = SUNDER_CAST(u64*, host_visible_vram_arena_suballocation_starting_offsets_suballocation_result.data);

		const sunder_arena_suballocation_result_t mesh_render_pass_data_reordering_helper_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->total_mesh_count), alignof(u32));
		core->mesh_render_pass_data_reordering_helper_buffer = SUNDER_CAST(u32*, mesh_render_pass_data_reordering_helper_buffer_suballocation_result.data);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// parse json font metadata, map glyphs - done
	// log them - done
	// load font atlas png - done
	// suballocate gpu side render instance glyph subbuffer - done
	// adjust the vertex inputs for overlay - pending
	// write cpu side pen advancment and other shennenigans - pending
	// write vertex, fragment shader for overlay - pending
	// profit?? - pending

	core->overlay_core.font_atlas_buffer[0].em_size = SUNDER_CAST2(f32)cJSON_GetObjectItem(metrics, "emSize")->valuedouble;
	core->overlay_core.font_atlas_buffer[0].base_line_height = cJSON_GetObjectItem(metrics, "lineHeight")->valuedouble;
	core->overlay_core.font_atlas_buffer[0].ascender = cJSON_GetObjectItem(metrics, "ascender")->valuedouble;
	core->overlay_core.font_atlas_buffer[0].descender = cJSON_GetObjectItem(metrics, "descender")->valuedouble;
	core->overlay_core.font_atlas_buffer[0].width = cJSON_GetObjectItem(atlas, "width")->valueint;
	core->overlay_core.font_atlas_buffer[0].height = cJSON_GetObjectItem(atlas, "height")->valueint;
	core->overlay_core.font_atlas_buffer[0].glyph_size = cJSON_GetObjectItem(atlas, "size")->valueint;

	for (u32 i = 0; i < glyph_count; i++)
	{
		const cJSON* glyph = cJSON_GetArrayItem(glyphs, i);
		const i32 unicode_index = cJSON_GetObjectItem(glyph, "unicode")->valueint;
		const f64 advance = cJSON_GetObjectItem(glyph, "advance")->valuedouble;

		const cJSON* plane_bounds = cJSON_GetObjectItem(glyph, "planeBounds");
		const cJSON* atlas_bounds = cJSON_GetObjectItem(glyph, "atlasBounds");

		core->overlay_core.glyph_buffer[unicode_index].advance = advance;
		core->overlay_core.glyph_buffer[unicode_index].index = unicode_index;
		core->overlay_core.glyph_buffer[unicode_index].has_bounds = false;

		if (plane_bounds != nullptr)
		{
			core->overlay_core.glyph_buffer[unicode_index].has_bounds = true;

			core->overlay_core.glyph_buffer[unicode_index].plane_bounds.left = cJSON_GetObjectItem(plane_bounds, "left")->valuedouble;
			core->overlay_core.glyph_buffer[unicode_index].plane_bounds.right = cJSON_GetObjectItem(plane_bounds, "right")->valuedouble;
			core->overlay_core.glyph_buffer[unicode_index].plane_bounds.top = cJSON_GetObjectItem(plane_bounds, "top")->valuedouble;
			core->overlay_core.glyph_buffer[unicode_index].plane_bounds.bottom = cJSON_GetObjectItem(plane_bounds, "bottom")->valuedouble;
		}

		if (atlas_bounds != nullptr)
		{
			core->overlay_core.glyph_buffer[unicode_index].has_bounds = true;

			core->overlay_core.glyph_buffer[unicode_index].atlas_bounds.left = cJSON_GetObjectItem(atlas_bounds, "left")->valuedouble;
			core->overlay_core.glyph_buffer[unicode_index].atlas_bounds.right = cJSON_GetObjectItem(atlas_bounds, "right")->valuedouble;
			core->overlay_core.glyph_buffer[unicode_index].atlas_bounds.top = cJSON_GetObjectItem(atlas_bounds, "top")->valuedouble;
			core->overlay_core.glyph_buffer[unicode_index].atlas_bounds.bottom = cJSON_GetObjectItem(atlas_bounds, "bottom")->valuedouble;
		}
	}

	/*
	SUNDER_LOG("\n=========================================");
	for (u32 i = 0; i < 256; i++)
	{
		SUNDER_LOG("\nindex: ");
		SUNDER_LOG(core->overlay_core.glyph_buffer[i].index);
		SUNDER_LOG("\nadvance: ");
		SUNDER_LOG(core->overlay_core.glyph_buffer[i].advance);

		if (core->overlay_core.glyph_buffer[i].has_bounds)
		{
			SUNDER_LOG("\n\nplane_bounds");
			SUNDER_LOG("\nleft: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].plane_bounds.left);
			SUNDER_LOG("\nright: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].plane_bounds.right);
			SUNDER_LOG("\ntop: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].plane_bounds.top);
			SUNDER_LOG("\nbottom: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].plane_bounds.bottom);

			SUNDER_LOG("\n\natlas_bounds");
			SUNDER_LOG("\nleft: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].atlas_bounds.left);
			SUNDER_LOG("\nright: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].atlas_bounds.right);
			SUNDER_LOG("\ntop: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].atlas_bounds.top);
			SUNDER_LOG("\nbottom: ");
			SUNDER_LOG(core->overlay_core.glyph_buffer[i].atlas_bounds.bottom);
		}
		
		SUNDER_LOG("\n=========================================\n");
	}
	*/

	cJSON_Delete(font_root);
	sunder_aligned_free(SUNDER_PRE_FREE_CAST(json_buffer));

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	vkGetPhysicalDeviceQueueFamilyProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->supported_queue_family_property_count, core->supported_queue_family_properties);
	const VkResult available_device_extensions_enumeration_result = vkEnumerateDeviceExtensionProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], nullptr, &core->available_device_extension_count, core->available_device_extensions);
	const VkResult surface_formats_retrieval_result = vkGetPhysicalDeviceSurfaceFormatsKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_surface_format_count, core->supported_surface_formats);
	const VkResult swapchain_present_modes_retrieval_result = vkGetPhysicalDeviceSurfacePresentModesKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_swapchain_present_mode_count, core->supported_swapchain_present_modes);

	const u32 possible_image_format_count = 4u;
	const VkFormat possible_image_formats[possible_image_format_count] { VK_FORMAT_R8G8B8A8_UNORM , VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB };
	const VkFormat supported_image_format = lightray_vulkan_get_supported_image_format(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], possible_image_formats, possible_image_format_count, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_TYPE_2D, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_SRGB);

	const u32 possible_msdf_font_atlas_format_count = 2u;
	const VkFormat possible_msdf_font_atlas_formats[possible_msdf_font_atlas_format_count] { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8_UNORM };
	const VkFormat supported_msdf_font_atlas_format = lightray_vulkan_get_supported_image_format(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], possible_msdf_font_atlas_formats, possible_msdf_font_atlas_format_count, VK_IMAGE_TILING_LINEAR, VK_IMAGE_TYPE_2D, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_UNORM);

	SUNDER_LOG("\n\n");
	for (u32 i = 0; i < core->supported_surface_format_count; i++)
	{
		SUNDER_LOG(core->supported_surface_formats[i].format);
		SUNDER_LOG(" ");
		SUNDER_LOG(core->supported_surface_formats[i].colorSpace);
		SUNDER_LOG("\n");
	}

	// finding suitable surface format for the swapchain 
	for (u32 i = 0; i < core->supported_surface_format_count; i++)
	{
		if (core->supported_surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && core->supported_surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			core->chosen_surface_format = core->supported_surface_formats[i];
			break;
		}

		core->chosen_surface_format = core->supported_surface_formats[0];
	}

	// finding suitable swapchain present mode
	for (u32 i = 0; i < core->supported_swapchain_present_mode_count; i++)
	{
		if (core->supported_swapchain_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			SUNDER_SET_BIT(core->flags, LIGHTRAY_BITS_PRESENT_MODE_MAILBOX_SUPPORTED_BIT, 1U);
		}

		else if (core->supported_swapchain_present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
		{
			SUNDER_SET_BIT(core->flags, LIGHTRAY_BITS_PRESENT_MODE_FIFO_SUPPORTED_BIT, 1U);
		}

		else if (core->supported_swapchain_present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			SUNDER_SET_BIT(core->flags, LIGHTRAY_BITS_PRESENT_MODE_IMMEDIATE_SUPPORTED_BIT, 1U);
		}
	}

	if (SUNDER_IS_ANY_BIT_SET(core->flags, LIGHTRAY_BITS_PRESENT_MODE_MAILBOX_SUPPORTED_BIT, 1U) && core->swapchain_image_in_use_count == 3 || core->swapchain_image_in_use_count == 2)
	{
		core->chosen_swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
	}

	else if (SUNDER_IS_ANY_BIT_SET(core->flags, LIGHTRAY_BITS_PRESENT_MODE_IMMEDIATE_SUPPORTED_BIT, 1U) && core->chosen_swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		core->chosen_swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	else
	{
		core->chosen_swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	}

	u32 swapchain_presentation_supported_queue_family_indices_iter = 0;

	for (u32 i = 0; i < core->supported_queue_family_property_count; i++)
	{
		const VkResult queue_family_support_query_result = vkGetPhysicalDeviceSurfaceSupportKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], i, core->surface, &queue_family_supports_swapchain_presentation);

		if (queue_family_support_query_result == VK_SUCCESS && queue_family_supports_swapchain_presentation == VK_TRUE)
		{
			core->swapchain_presentation_supported_queue_family_indices[swapchain_presentation_supported_queue_family_indices_iter] = i;
			swapchain_presentation_supported_queue_family_indices_iter++;
			queue_family_supports_swapchain_presentation = VK_FALSE;
		}
	}

	SUNDER_LOG("\n\nswapchain_presentation_supported_queue_family_indices\n");

	for (u32 i = 0; i < core->swapchain_presentation_supported_queue_family_index_count; i++)
	{
		SUNDER_LOG(core->swapchain_presentation_supported_queue_family_indices[i]);
		SUNDER_LOG(" ");
	}


	SUNDER_LOG("\n\n");

	for (u32 i = 0; i < core->available_device_extension_count; i++)
	{
		SUNDER_LOG(core->available_device_extensions[i].extensionName);
		SUNDER_LOG("\n");
	}

	u32 swapchain_device_extension_index = UINT32_MAX;

	for (u32 i = 0; i < core->available_device_extension_count; i++)
	{
		if (strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, core->available_device_extensions[i].extensionName) == 0)
		{
			swapchain_device_extension_index = i;
			break;
		}
	}

	core->enabled_device_extensions[0] = core->available_device_extensions[swapchain_device_extension_index].extensionName;
	core->enabled_device_extension_count = 1;

	const u32 graphics_queue_family_index = lightray_vulkan_get_graphics_queue_family_index(core->supported_queue_family_properties, core->swapchain_presentation_supported_queue_family_indices, core->swapchain_presentation_supported_queue_family_index_count);
	const u32 present_queue_family_index = lightray_vulkan_get_present_queue_family_index(core->supported_queue_family_properties, core->swapchain_presentation_supported_queue_family_indices, core->swapchain_presentation_supported_queue_family_index_count);

	core->queue_priority = 1.0f;

	core->graphics_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	core->graphics_queue_info.queueFamilyIndex = graphics_queue_family_index;
	core->graphics_queue_info.queueCount = 1;
	core->graphics_queue_info.pQueuePriorities = &core->queue_priority;
	core->graphics_queue_info.flags = 0;
	core->graphics_queue_info.pNext = nullptr;

	core->present_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	core->present_queue_info.queueFamilyIndex = present_queue_family_index;
	core->present_queue_info.queueCount = 1;
	core->present_queue_info.pQueuePriorities = &core->queue_priority;
	core->present_queue_info.flags = 0;
	core->present_queue_info.pNext = nullptr;

	core->queues_in_use_infos[0] = core->graphics_queue_info;
	core->queues_in_use_infos[1] = core->present_queue_info;
	core->queues_in_use_info_count = 2;

	core->logical_device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	core->logical_device_info.pQueueCreateInfos = core->queues_in_use_infos;
	core->logical_device_info.queueCreateInfoCount = core->queues_in_use_info_count;;
	core->logical_device_info.pEnabledFeatures = &core->gpu_features;
	core->logical_device_info.ppEnabledExtensionNames = core->enabled_device_extensions;
	core->logical_device_info.enabledExtensionCount = core->enabled_device_extension_count;
	core->logical_device_info.flags = 0;
	core->logical_device_info.pNext = nullptr;

	const VkResult logical_device_creation_result = vkCreateDevice(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->logical_device_info, nullptr, &core->logical_device);
	vkGetDeviceQueue(core->logical_device, graphics_queue_family_index, 0, &core->graphics_queue);
	vkGetDeviceQueue(core->logical_device, present_queue_family_index, 0, &core->present_queue);

	const u8 texture_creation_mask = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_IMAGE_BIT }, 1);
	u64 host_visible_vram_arena_staging_texture_buffer_allocation_size = 0;

	for (u32 i = 0; i < default_textures_range; i++)
	{
		const lightray_vulkan_result texture_loading_result = lightray_vulkan_load_texture(initialization_data->texture_metadata_buffer[i].path, &core->texture_buffer[i]);

		lightray_vulkan_texture_creation_data_t texture_creation_data{};
		texture_creation_data.type = VK_IMAGE_TYPE_2D;
		texture_creation_data.width = core->texture_buffer[i].width;
		texture_creation_data.height = core->texture_buffer[i].height;
		texture_creation_data.format = supported_image_format;
		texture_creation_data.tiling = VK_IMAGE_TILING_OPTIMAL;
		texture_creation_data.usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		texture_creation_data.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
		texture_creation_data.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		texture_creation_data.filter = initialization_data->texture_metadata_buffer[i].filter;
		texture_creation_data.layer_count = initialization_data->texture_metadata_buffer[i].layer_count;
		texture_creation_data.creation_flags = texture_creation_mask;
		texture_creation_data.kind = LIGHTRAY_VULKAN_TEXTURE_KIND_MESH_TEXTURE;

		const lightray_vulkan_result texture_image_creation_result = lightray_vulkan_create_texture(core, &core->texture_buffer[i], &texture_creation_data);
		host_visible_vram_arena_staging_texture_buffer_allocation_size += sunder_align64(core->texture_buffer[i].size, 256);
		host_visible_vram_arena_staging_texture_buffer_allocation_size = sunder_align64(host_visible_vram_arena_staging_texture_buffer_allocation_size, 256);

		vkGetImageMemoryRequirements(core->logical_device, core->texture_buffer[i].image, &texture_vram_requirements_buffer[i]);
		vram_type_index_buffer[i] = lightray_vulkan_get_vram_type_index(core, texture_vram_requirements_buffer[i].memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		texture_sampler_metadata_buffer[i].filter = initialization_data->texture_metadata_buffer[i].filter;
		texture_sampler_metadata_buffer[i].address_mode = initialization_data->texture_metadata_buffer[i].address_mode;
	}

	/////////////////////////// depth texture creation ///////////////////////////
	const u32 possible_depth_format_count = 3u;
	const VkFormat possible_depth_formats[possible_depth_format_count] { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

	const VkFormat supported_depth_fomat = lightray_vulkan_get_supported_image_format(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], possible_depth_formats, possible_depth_format_count, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_TYPE_2D, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT);

	lightray_vulkan_texture_creation_data_t depth_texture_creation_data{};
	depth_texture_creation_data.type = VK_IMAGE_TYPE_2D;
	depth_texture_creation_data.width = core->video_mode->width;
	depth_texture_creation_data.height = core->video_mode->height;
	depth_texture_creation_data.format = supported_depth_fomat;
	depth_texture_creation_data.tiling = VK_IMAGE_TILING_OPTIMAL;
	depth_texture_creation_data.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depth_texture_creation_data.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	depth_texture_creation_data.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_texture_creation_data.filter = VK_FILTER_LINEAR;
	depth_texture_creation_data.layer_count = 1;
	depth_texture_creation_data.creation_flags = texture_creation_mask;
	depth_texture_creation_data.kind = LIGHTRAY_VULKAN_TEXTURE_KIND_DEPTH_TEXTURE;

	const lightray_vulkan_result depth_texture_image_creation_result = lightray_vulkan_create_texture(core, &core->texture_buffer[core->depth_texture_index], &depth_texture_creation_data);
	vkGetImageMemoryRequirements(core->logical_device, core->texture_buffer[core->depth_texture_index].image, &texture_vram_requirements_buffer[core->depth_texture_index]);
	vram_type_index_buffer[core->depth_texture_index] = lightray_vulkan_get_vram_type_index(core, texture_vram_requirements_buffer[core->depth_texture_index].memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	/////////////////////////// depth texture creation ///////////////////////////

	////////////////////////// msdf_font_atlases texture creation //////////////////////////

	u32 msdf_font_atlas_buffer_iter = 0;

	for (u32 i = msdf_font_atlases_starting_offset; i < msdf_font_atlases_range; i++)
	{
		const lightray_vulkan_result msdf_font_atlas_texture_loading_result = lightray_vulkan_load_texture(initialization_data->msdf_font_atlas_path_buffer[msdf_font_atlas_buffer_iter], &core->texture_buffer[i]);

		lightray_vulkan_texture_creation_data_t msdf_font_atlas_texture_creation_data{};
		msdf_font_atlas_texture_creation_data.type = VK_IMAGE_TYPE_2D;
		msdf_font_atlas_texture_creation_data.width = core->texture_buffer[i].width;
		msdf_font_atlas_texture_creation_data.height = core->texture_buffer[i].height;
		msdf_font_atlas_texture_creation_data.format = supported_msdf_font_atlas_format;
		msdf_font_atlas_texture_creation_data.tiling = VK_IMAGE_TILING_OPTIMAL;
		msdf_font_atlas_texture_creation_data.usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		msdf_font_atlas_texture_creation_data.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
		msdf_font_atlas_texture_creation_data.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		msdf_font_atlas_texture_creation_data.filter = VK_FILTER_LINEAR;
		msdf_font_atlas_texture_creation_data.layer_count = 1;
		msdf_font_atlas_texture_creation_data.creation_flags = texture_creation_mask;
		msdf_font_atlas_texture_creation_data.kind = LIGHTRAY_VULKAN_TEXTURE_KIND_MSDF_FONT_ATLAS_TEXTURE;

		const lightray_vulkan_result msdf_font_atlas_texture_creation_result = lightray_vulkan_create_texture(core, &core->texture_buffer[i], &msdf_font_atlas_texture_creation_data);
		host_visible_vram_arena_staging_texture_buffer_allocation_size += sunder_align64(core->texture_buffer[i].size, 256);
		host_visible_vram_arena_staging_texture_buffer_allocation_size = sunder_align64(host_visible_vram_arena_staging_texture_buffer_allocation_size, 256);

		vkGetImageMemoryRequirements(core->logical_device, core->texture_buffer[i].image, &texture_vram_requirements_buffer[i]);
		vram_type_index_buffer[i] = lightray_vulkan_get_vram_type_index(core, texture_vram_requirements_buffer[i].memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		texture_sampler_metadata_buffer[i].filter = VK_FILTER_LINEAR;
		texture_sampler_metadata_buffer[i].address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		msdf_font_atlas_buffer_iter++;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////

	SUNDER_LOG("\n\nvram type index buffer : ");
	for (u32 i = 0; i < core->total_texture_count_including_depth_texture; i++)
	{
		SUNDER_LOG(vram_type_index_buffer[i]);
		SUNDER_LOG(" ");
	}

	SUNDER_LOG("\n\ntexture samler metadata buffer: ");
	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		SUNDER_LOG("\nfilter: ");
		SUNDER_LOG(SUNDER_CAST2(u32)texture_sampler_metadata_buffer[i].filter);
		SUNDER_LOG(" address_mode: ");
		SUNDER_LOG(SUNDER_CAST2(u32)texture_sampler_metadata_buffer[i].address_mode);
		SUNDER_LOG("\n");
	}

	for (u32 i = 0; i < core->total_texture_count_including_depth_texture; i++)
	{
		vram_type_index_filter_buffer[i] = 256; // maybe make it a define later or something 
	}

	// filtering out duplicated vram type indices, so that the final filtered buffer looks like : [7, 8] and matches vram texture arena count, instead of [7, 7, 8, 8] and being the count of textures
	u32 vram_type_index_filter_buffer_size = 0;

	for (u32 i = 0; i < core->total_texture_count_including_depth_texture; i++)
	{
		const bool vram_type_index_has_already_been_encountered = sunder_exists_u32(vram_type_index_filter_buffer, vram_type_index_filter_buffer_size, vram_type_index_buffer[i]);

		if (!vram_type_index_has_already_been_encountered)
		{
			vram_type_index_filter_buffer[vram_type_index_filter_buffer_size] = vram_type_index_buffer[i];
			vram_type_index_filter_buffer_size++;
		}
	}

	core->total_vram_texture_arena_count = vram_type_index_filter_buffer_size;
	const u64 vram_texture_arena_buffer_suballocation_size = sunder_compute_array_size_in_bytes(sizeof(lightray_vulkan_vram_texture_arena_t), core->total_vram_texture_arena_count);

	const sunder_arena_suballocation_result_t vram_texture_arena_buffer_suballocation_result = sunder_suballocate_from_arena_debug(initialization_data->arena, vram_texture_arena_buffer_suballocation_size, alignof(lightray_vulkan_vram_texture_arena_t));
	core->vram_texture_arena_buffer = (lightray_vulkan_vram_texture_arena_t*)vram_texture_arena_buffer_suballocation_result.data;

	SUNDER_LOG("\n\nvram type index filter buffer : ");
	for (u32 i = 0; i < core->total_vram_texture_arena_count; i++)
	{
		SUNDER_LOG(vram_type_index_filter_buffer[i]);
		SUNDER_LOG(" ");
	}

	// properly allocating vram texture arenas
	for (u32 i = 0; i < core->total_vram_texture_arena_count; i++)
	{
		u64 current_vram_texture_arena_allocation_size = 0;
		u32 current_alignment = 0;

		for (u32 j = 0; j < core->total_texture_count_including_depth_texture; j++)
		{
			if (vram_type_index_filter_buffer[i] == vram_type_index_buffer[j])
			{
				const u32 possible_new_alignment = SUNDER_CAST(u32, texture_vram_requirements_buffer[j].alignment);

				if (current_alignment < possible_new_alignment)
				{
					current_alignment = possible_new_alignment;
					current_vram_texture_arena_allocation_size = sunder_align64(current_vram_texture_arena_allocation_size, current_alignment);
				}

				current_vram_texture_arena_allocation_size += sunder_align64(texture_vram_requirements_buffer[j].size, current_alignment);
			}
		}

		current_vram_texture_arena_allocation_size = sunder_align64(current_vram_texture_arena_allocation_size, current_alignment);
		const lightray_vulkan_result vram_texture_arena_allocation_result = lightray_vulkan_allocate_vram_texture_arena_debug(core, &core->vram_texture_arena_buffer[i], current_vram_texture_arena_allocation_size, current_alignment, vram_type_index_filter_buffer[i]);
	}

	// change this to just an enum, no need for a bitfield (vulkan yells at you if you create both of them right after another, for image not being bound to any device memory beforehand)
	const u8 texture_view_creation_flags = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_VIEW_BIT }, 1);
	lightray_vulkan_texture_creation_data_t texture_view_creation_data{};
	texture_view_creation_data.creation_flags = texture_view_creation_flags;
	texture_view_creation_data.view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	// texture suballocation / view creation
	for (u32 i = 0; i < core->total_texture_count_including_depth_texture; i++)
	{
		const sunder_buffer_index_query_result_u32_t res = sunder_query_buffer_index_u32(vram_type_index_filter_buffer, 0, core->total_vram_texture_arena_count, vram_type_index_buffer[i], false);
		core->texture_buffer[i].vram_texture_arena_index = res.return_index;
		const lightray_vulkan_result texture_suballocation_result = lightray_vulkan_suballocate_texture(core, &core->vram_texture_arena_buffer[res.return_index], &core->texture_buffer[i], texture_vram_requirements_buffer[i].size);
		const lightray_vulkan_result texture_view_creation_result = lightray_vulkan_create_texture(core, &core->texture_buffer[i], &texture_view_creation_data);
	}

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		texture_sampler_metadata_filter_buffer[i].filter = SUNDER_CAST(VkFilter, 512);
		texture_sampler_metadata_filter_buffer[i].address_mode = SUNDER_CAST(VkSamplerAddressMode, 512);
	}

	// filtering out duplicated sampler filtering of each texture so that the final filter buffer looks like : [VK_FILTER_NEAREST, VK_FILTER_LINEAR] and matches the sampler count, instead of [VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_FILTER_LINEAR] and being the texture count
	u32 texture_sampler_filter_buffer_size = 0;

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		const bool texture_sampler_metadata_has_already_been_encountered = lightray_vulkan_texture_sampler_metadata_exists(texture_sampler_metadata_filter_buffer, texture_sampler_filter_buffer_size, &texture_sampler_metadata_buffer[i]);

		if (!texture_sampler_metadata_has_already_been_encountered)
		{
			texture_sampler_metadata_filter_buffer[texture_sampler_filter_buffer_size] = texture_sampler_metadata_buffer[i];
			texture_sampler_filter_buffer_size++;
		}
	}

	core->total_sampler_count = texture_sampler_filter_buffer_size;
	const u64 sampler_buffer_suballocation_size = sunder_compute_array_size_in_bytes(sizeof(VkSampler), core->total_sampler_count);

	const sunder_arena_suballocation_result_t sampler_buffer_suballocation_result = sunder_suballocate_from_arena_debug(initialization_data->arena, sampler_buffer_suballocation_size, alignof(VkSampler));
	core->sampler_buffer = SUNDER_CAST(VkSampler*, sampler_buffer_suballocation_result.data);

	SUNDER_LOG("\n\ntexture filtering filter buffer: ");
	for (u32 i = 0; i < core->total_sampler_count; i++)
	{
		SUNDER_LOG("\nfilter: ");
		SUNDER_LOG(SUNDER_CAST2(u32)texture_sampler_metadata_filter_buffer[i].filter);
		SUNDER_LOG(" address_mode: ");
		SUNDER_LOG(SUNDER_CAST2(u32)texture_sampler_metadata_filter_buffer[i].address_mode);
		SUNDER_LOG("\n");
	}

	// samplers creation
	for (u32 i = 0; i < core->total_sampler_count; i++)
	{
		VkSamplerCreateInfo sampler_create_info{};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter = texture_sampler_metadata_filter_buffer[i].filter;
		sampler_create_info.minFilter = texture_sampler_metadata_filter_buffer[i].filter;
		sampler_create_info.anisotropyEnable = core->gpu_features.samplerAnisotropy;
		sampler_create_info.maxAnisotropy = core->gpu_properties.limits.maxSamplerAnisotropy;
		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_create_info.unnormalizedCoordinates = VK_FALSE;
		sampler_create_info.compareEnable = VK_FALSE;
		sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f;
		sampler_create_info.flags = 0;
		sampler_create_info.pNext = nullptr;
		sampler_create_info.addressModeU = texture_sampler_metadata_filter_buffer[i].address_mode;
		sampler_create_info.addressModeV = texture_sampler_metadata_filter_buffer[i].address_mode;
		sampler_create_info.addressModeW = texture_sampler_metadata_filter_buffer[i].address_mode;

		const VkResult sampler_creation_result = vkCreateSampler(core->logical_device, &sampler_create_info, nullptr, &core->sampler_buffer[i]);
		SUNDER_LOG("\nsampler_creation_result at ");
		SUNDER_LOG(i);
		SUNDER_LOG(" is ");
		SUNDER_LOG(SUNDER_CAST(u32, sampler_creation_result));
	}

	{
		u64 host_visible_vram_arena_allocation_size_buffer[6]{};
		host_visible_vram_arena_allocation_size_buffer[0] = sunder_compute_aligned_array_allocation_size(sizeof(lightray_cvp_t), core->total_camera_count, uniform_buffer_required_alignment); // cvp_buffer
		host_visible_vram_arena_allocation_size_buffer[1] = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, uniform_buffer_required_alignment); // vertex_buffer
		host_visible_vram_arena_allocation_size_buffer[2] = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, uniform_buffer_required_alignment); // index_buffer
		host_visible_vram_arena_allocation_size_buffer[3] = sunder_compute_aligned_allocation_size(sizeof(lightray_render_instance_t), total_instance_model_count, uniform_buffer_required_alignment); // render_instance_buffer
		host_visible_vram_arena_allocation_size_buffer[4] = sunder_compute_aligned_allocation_size(sizeof(lightray_render_instance_glyph_t), glyph_count, uniform_buffer_required_alignment);
		host_visible_vram_arena_allocation_size_buffer[5] = host_visible_vram_arena_staging_texture_buffer_allocation_size;
		
		const u64 host_visible_vram_arena_metadata_suballocation_size = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(host_visible_vram_arena_allocation_count, initialization_data->arena_alignment);
		const u64 host_visible_vram_arena_allocation_size = sunder_accumulate_aligned_allocation_size(host_visible_vram_arena_allocation_size_buffer, 6, uniform_buffer_required_alignment);

		lightray_vulkan_vram_arena_allocation_data_t host_visible_vram_arena_allocation_data{};
		host_visible_vram_arena_allocation_data.allocation_size = host_visible_vram_arena_allocation_size; //sunder_align64(host_visible_vram_arena_allocation_size + sunder_align64(host_visible_vram_arena_staging_texture_buffer_allocation_size, uniform_buffer_required_alignment), uniform_buffer_required_alignment);
		host_visible_vram_arena_allocation_data.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		host_visible_vram_arena_allocation_data.metadata_arena = &core->general_purpose_ram_arena;
		host_visible_vram_arena_allocation_data.suballocation_count = host_visible_vram_arena_allocation_count;
		host_visible_vram_arena_allocation_data.metadata_allocation_size = SUNDER_CAST(u32, host_visible_vram_arena_metadata_suballocation_size);
		host_visible_vram_arena_allocation_data.vram_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		u64 device_local_vram_arena_allocation_size_buffer[2]{};
		device_local_vram_arena_allocation_size_buffer[0] = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);
		device_local_vram_arena_allocation_size_buffer[1] = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);

		const u64 device_local_vram_arena_metadata_suballocation_size = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(device_local_vram_arena_allocation_count, initialization_data->arena_alignment);
		const u64 device_local_vram_arena_allocation_size = sunder_accumulate_aligned_allocation_size(device_local_vram_arena_allocation_size_buffer, 2, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);

		lightray_vulkan_vram_arena_allocation_data_t device_local_vram_arena_allocation_data{};
		device_local_vram_arena_allocation_data.allocation_size = device_local_vram_arena_allocation_size;
		device_local_vram_arena_allocation_data.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		device_local_vram_arena_allocation_data.metadata_arena = &core->general_purpose_ram_arena;
		device_local_vram_arena_allocation_data.suballocation_count = device_local_vram_arena_allocation_count;
		device_local_vram_arena_allocation_data.metadata_allocation_size = SUNDER_CAST(u32, device_local_vram_arena_metadata_suballocation_size);
		device_local_vram_arena_allocation_data.vram_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		const u64 host_visible_storage_vram_arena_metadata_suballocation_size = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(host_visible_storage_vram_arena_allocation_count, initialization_data->arena_alignment);
		const u64 host_visible_storage_vram_arena_allocation_size = sunder_compute_aligned_allocation_size(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count, storage_buffer_required_alignment);

		lightray_vulkan_vram_arena_allocation_data_t host_visible_storage_vram_arena_allocation_data{};
		host_visible_storage_vram_arena_allocation_data.allocation_size = host_visible_storage_vram_arena_allocation_size;
		host_visible_storage_vram_arena_allocation_data.usage_flags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		host_visible_storage_vram_arena_allocation_data.metadata_arena = &core->general_purpose_ram_arena;
		host_visible_storage_vram_arena_allocation_data.suballocation_count = host_visible_storage_vram_arena_allocation_count;
		host_visible_storage_vram_arena_allocation_data.metadata_allocation_size = SUNDER_CAST(u32, host_visible_storage_vram_arena_metadata_suballocation_size);
		host_visible_storage_vram_arena_allocation_data.vram_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		const lightray_vulkan_result host_visible_vram_arena_allocation_result = lightray_vulkan_allocate_vram_arena_debug(core, &core->host_visible_vram_arena, &host_visible_vram_arena_allocation_data);
		const lightray_vulkan_result device_local_vram_arena_allocation_result = lightray_vulkan_allocate_vram_arena_debug(core, &core->device_local_vram_arena, &device_local_vram_arena_allocation_data);
		const lightray_vulkan_result host_visible_storage_vram_arena_allocation_resilt = lightray_vulkan_allocate_vram_arena_debug(core, &core->host_visible_storage_vram_arena, &host_visible_storage_vram_arena_allocation_data);
	}

	// account for glyph render instance buffer 
	u32 host_visible_vram_arena_suballocation_starting_indices_iter = LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_CVP_BUFFER_STARTING_INDEX;

	for (u32 i = 0; i < core->total_camera_count; i++)
	{
		const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_cvp_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sizeof(lightray_cvp_t));
		core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_suballocation_starting_indices_iter] = host_visible_vram_arena_cvp_suballocation_result.starting_offset;
		host_visible_vram_arena_suballocation_starting_indices_iter++;
	}

	core->host_visible_vram_arena_vertex_buffer_starting_index = host_visible_vram_arena_suballocation_starting_indices_iter;

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_vertex_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count));
	core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_suballocation_starting_indices_iter] = host_visible_vram_arena_vertex_buffer_suballocation_result.starting_offset;
	host_visible_vram_arena_suballocation_starting_indices_iter++;

	core->host_visible_vram_arena_index_buffer_starting_index = host_visible_vram_arena_suballocation_starting_indices_iter;

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_index_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count));
	core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_suballocation_starting_indices_iter] = host_visible_vram_arena_index_buffer_suballocation_result.starting_offset;
	host_visible_vram_arena_suballocation_starting_indices_iter++;

	core->host_visible_vram_arena_render_instance_buffer_starting_index = host_visible_vram_arena_suballocation_starting_indices_iter;

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_render_instance_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), total_instance_model_count));
	core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_buffer_starting_index] = host_visible_vram_arena_render_instance_buffer_suballocation_result.starting_offset;
	host_visible_vram_arena_suballocation_starting_indices_iter++;

	core->host_visible_vram_arena_render_instance_glyph_buffer_starting_index = host_visible_vram_arena_suballocation_starting_indices_iter;

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_render_instance_glyph_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), glyph_count));
	core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_glyph_buffer_starting_index] = host_visible_vram_arena_render_instance_glyph_buffer_suballocation_result.starting_offset;
	host_visible_vram_arena_suballocation_starting_indices_iter++;

	core->host_visible_vram_arena_staging_texture_buffer_starting_index = host_visible_vram_arena_suballocation_starting_indices_iter;

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		const lightray_vulkan_vram_arena_suballocation_result_t raw_texture_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, core->texture_buffer[i].size);
		core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_suballocation_starting_indices_iter] = raw_texture_suballocation_result.starting_offset;
		host_visible_vram_arena_suballocation_starting_indices_iter++;
	}

	SUNDER_LOG("\n\nhost visible vram arena suballocation starting offsets : ");
	for (u32 i = 0; i < core->host_visible_vram_arena_suballocation_starting_offset_count; i++)
	{
		SUNDER_LOG(core->host_visible_vram_arena_suballocation_starting_offsets[i]);
		SUNDER_LOG(" ");
	}

	const VkResult host_visible_vram_arena_vram_mapping_result = vkMapMemory(core->logical_device, core->host_visible_vram_arena.device_memory, 0, core->host_visible_vram_arena.capacity, 0, &core->cpu_side_host_visible_vram_arena_view);

	const lightray_vulkan_vram_arena_suballocation_result_t device_local_vram_arena_vertex_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->device_local_vram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count));
	core->device_local_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_VERTEX_BUFFER_INDEX] = device_local_vram_arena_vertex_buffer_suballocation_result.starting_offset;

	const lightray_vulkan_vram_arena_suballocation_result_t device_local_vram_arena_index_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->device_local_vram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count));
	core->device_local_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_INDEX_BUFFER_INDEX] = device_local_vram_arena_index_buffer_suballocation_result.starting_offset;

	SUNDER_LOG("\n\ndevice local vram arena suballocation starting offsets : ");
	for (u32 i = 0; i < core->device_local_vram_arena_suballocation_starting_offset_count; i++)
	{
		SUNDER_LOG(core->device_local_vram_arena_suballocation_starting_offsets[i]);
		SUNDER_LOG(" ");
	}

	const lightray_vulkan_vram_arena_suballocation_result_t gpu_side_bone_transform_matrix_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_storage_vram_arena, sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count));
	const VkResult host_visible_storage_vram_arena_mapping_result = vkMapMemory(core->logical_device, core->host_visible_storage_vram_arena.device_memory, 0, core->host_visible_storage_vram_arena.capacity, 0, &core->cpu_side_host_visible_storage_vram_arena_view);

	lightray_vulkan_create_swapchain(core);

	{
		const lightray_result compiled_static_mesh_vertex_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_static_mesh_vertex_shader_path, core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
		const lightray_result compiled_static_mesh_fragment_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_static_mesh_fragment_shader_path, core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
		const lightray_result compiled_textured_static_mesh_vertex_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_textured_static_mesh_vertex_shader_path, core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
		const lightray_result compiled_textured_static_mesh_fragment_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_textured_static_mesh_fragment_shader_path, core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
		const lightray_result compiled_skeletal_mesh_vertex_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_skeletal_mesh_vertex_shader_path, core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_VERTEX_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_VERTEX_SHADER_INDEX].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
		const lightray_result compiled_skeletal_mesh_fragment_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_skeletal_mesh_fragment_shader_path, core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_FRAGMENT_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_FRAGMENT_SHADER_INDEX].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
		const lightray_result compiled_overlay_vertex_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_overlay_vertex_shader_path, core->shaders[6].byte_code, &core->shaders[6].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
		const lightray_result compiled_overlay_fragment_shader_byte_code_loading_result = lightray_load_shader_byte_code(compiled_overlay_fragment_shader_path, core->shaders[7].byte_code, &core->shaders[7].byte_code_size, LIGHTRAY_SHADER_BYTE_CODE_SIZE_LIMIT);
	}

	SUNDER_LOG("\n\n");
	SUNDER_LOG(core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size);
	SUNDER_LOG(", ");
	SUNDER_LOG(core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size);
	SUNDER_LOG(", ");
	SUNDER_LOG(core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size);
	SUNDER_LOG(", ");
	SUNDER_LOG(core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size);
	SUNDER_LOG(", ");
	SUNDER_LOG(core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_VERTEX_SHADER_INDEX].byte_code_size);
	SUNDER_LOG(", ");
	SUNDER_LOG(core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_FRAGMENT_SHADER_INDEX].byte_code_size);

	{
		const lightray_vulkan_result static_mesh_vertex_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX], VK_SHADER_STAGE_VERTEX_BIT);
		const lightray_vulkan_result static_mesh_fragment_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX], VK_SHADER_STAGE_FRAGMENT_BIT);
		const lightray_vulkan_result textured_static_mesh_vertex_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_VERTEX_SHADER_INDEX], VK_SHADER_STAGE_VERTEX_BIT);
		const lightray_vulkan_result textured_static_mesh_fragment_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_FRAGMENT_SHADER_INDEX], VK_SHADER_STAGE_FRAGMENT_BIT);
		const lightray_vulkan_result skeletal_mesh_vertex_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_VERTEX_SHADER_INDEX], VK_SHADER_STAGE_VERTEX_BIT);
		const lightray_vulkan_result skeletal_mesh_fragment_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_FRAGMENT_SHADER_INDEX], VK_SHADER_STAGE_FRAGMENT_BIT);
		const lightray_vulkan_result overlay_vertex_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[6], VK_SHADER_STAGE_VERTEX_BIT);
		const lightray_vulkan_result overlay_fragment_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[7], VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	// offset into computed bone buffer per skeletal mesh = skeletal instance index * skeleton bone count
	// computed bone buffer per skeletal mesh offet with respect to bone indices + offset into computed bone buffer per skeletal mesh + bone index
	 
	// 0 + (0 * 3) = 0;
	// 3 + (0 * 3) = 3;
	// 6 + (0 * 3) = 6;
	 
	// 4 bone indices applied
	// 6 + 0 = 6;
	// 6 + 1 = 7;
	// 6 + 2 = 8;
	// 6 + 0 = 6;

	core->descriptor_set_layout_bindings[0].binding = 0;
	core->descriptor_set_layout_bindings[0].descriptorCount = 1;
	core->descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	core->descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	core->descriptor_set_layout_bindings[0].pImmutableSamplers = nullptr;

	core->descriptor_set_layout_bindings[1].binding = 1;
	core->descriptor_set_layout_bindings[1].descriptorCount = 1;
	core->descriptor_set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	core->descriptor_set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	core->descriptor_set_layout_bindings[1].pImmutableSamplers = nullptr;

	core->descriptor_set_layout_bindings[2].binding = 2;
	core->descriptor_set_layout_bindings[2].descriptorCount = 1;
	core->descriptor_set_layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	core->descriptor_set_layout_bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	core->descriptor_set_layout_bindings[2].pImmutableSamplers = nullptr;

	core->descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	core->descriptor_set_layout_info.bindingCount = 3;
	core->descriptor_set_layout_info.pBindings = core->descriptor_set_layout_bindings;
	core->descriptor_set_layout_info.flags = 0;
	core->descriptor_set_layout_info.pNext = nullptr;

	const VkResult descriptor_set_layout_creation_result = vkCreateDescriptorSetLayout(core->logical_device, &core->descriptor_set_layout_info, nullptr, &core->main_descriptor_set_layout);

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		core->copy_descriptor_set_layouts[i] = core->main_descriptor_set_layout;
	}

	core->descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	core->descriptor_pool_sizes[0].descriptorCount = core->total_texture_count;

	core->descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	core->descriptor_pool_sizes[1].descriptorCount = core->total_texture_count;

	core->descriptor_pool_sizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	core->descriptor_pool_sizes[2].descriptorCount = core->total_texture_count;

	core->descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	core->descriptor_pool_info.maxSets = core->total_texture_count;
	core->descriptor_pool_info.pPoolSizes = core->descriptor_pool_sizes;
	core->descriptor_pool_info.poolSizeCount = 3;
	core->descriptor_pool_info.flags = 0;
	core->descriptor_pool_info.pNext = nullptr;

	const VkResult descriptor_pool_creation_result = vkCreateDescriptorPool(core->logical_device, &core->descriptor_pool_info, nullptr, &core->descriptor_pool);

	core->descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	core->descriptor_set_allocate_info.descriptorPool = core->descriptor_pool;
	core->descriptor_set_allocate_info.pSetLayouts = core->copy_descriptor_set_layouts;
	core->descriptor_set_allocate_info.descriptorSetCount = core->total_texture_count;
	core->descriptor_set_allocate_info.pNext = nullptr;

	const VkResult descriptor_set_allocation_result = vkAllocateDescriptorSets(core->logical_device, &core->descriptor_set_allocate_info, core->descriptor_sets);

	core->descriptor_buffer_infos[0].buffer = core->host_visible_vram_arena.buffer;
	core->descriptor_buffer_infos[0].offset = 0;
	core->descriptor_buffer_infos[0].range = sizeof(lightray_cvp_t);

	core->descriptor_buffer_infos[1].buffer = core->host_visible_storage_vram_arena.buffer;
	core->descriptor_buffer_infos[1].offset = gpu_side_bone_transform_matrix_buffer_suballocation_result.starting_offset;
	core->descriptor_buffer_infos[1].range = sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count);

	//texture_metadata_buffer_iter = 0;

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		u32 valid_sampler_index = 0;

		for (u32 j = 0; j < core->total_sampler_count; j++)
		{
			if (texture_sampler_metadata_buffer[i].filter == texture_sampler_metadata_filter_buffer[j].filter && texture_sampler_metadata_buffer[i].address_mode == texture_sampler_metadata_filter_buffer[j].address_mode)
			{
				valid_sampler_index = j;
				break;
			}
		}

		core->descriptor_combined_sampler_infos[i].imageView = core->texture_buffer[i].view;
		core->descriptor_combined_sampler_infos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //core->texture_buffer[i].layout;
		core->descriptor_combined_sampler_infos[i].sampler = core->sampler_buffer[valid_sampler_index];
		core->texture_buffer[i].sampler_index = valid_sampler_index;
	}

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		VkWriteDescriptorSet cvp_write_descriptor_set{};
		cvp_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cvp_write_descriptor_set.dstSet = core->descriptor_sets[i];
		cvp_write_descriptor_set.descriptorCount = 1;
		cvp_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		cvp_write_descriptor_set.dstBinding = 0;
		cvp_write_descriptor_set.dstArrayElement = 0;
		cvp_write_descriptor_set.pBufferInfo = &core->descriptor_buffer_infos[0];

		VkWriteDescriptorSet bone_buffer_write_descriptor_set{};
		bone_buffer_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bone_buffer_write_descriptor_set.dstSet = core->descriptor_sets[i];
		bone_buffer_write_descriptor_set.descriptorCount = 1;
		bone_buffer_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bone_buffer_write_descriptor_set.dstBinding = 2;
		bone_buffer_write_descriptor_set.dstArrayElement = 0;
		bone_buffer_write_descriptor_set.pBufferInfo = &core->descriptor_buffer_infos[1];

		VkWriteDescriptorSet combined_sampler_write_descriptor_set{};
		combined_sampler_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		combined_sampler_write_descriptor_set.descriptorCount = 1;
		combined_sampler_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		combined_sampler_write_descriptor_set.dstBinding = 1;
		combined_sampler_write_descriptor_set.dstSet = core->descriptor_sets[i];
		combined_sampler_write_descriptor_set.pImageInfo = &core->descriptor_combined_sampler_infos[i];
		combined_sampler_write_descriptor_set.dstArrayElement = 0;
		combined_sampler_write_descriptor_set.pNext = nullptr;
		combined_sampler_write_descriptor_set.pBufferInfo = nullptr;
		combined_sampler_write_descriptor_set.pTexelBufferView = nullptr;

		const u32 write_descriptor_count = 3u;
		const VkWriteDescriptorSet write_descriptor_sets[write_descriptor_count] { cvp_write_descriptor_set, bone_buffer_write_descriptor_set, combined_sampler_write_descriptor_set };

		vkUpdateDescriptorSets(core->logical_device, write_descriptor_count, write_descriptor_sets, 0, nullptr);
	}

	core->pipeline_shader_stage_infos[0] = core->shaders[0].stage_info;
	core->pipeline_shader_stage_infos[1] = core->shaders[1].stage_info;
	core->pipeline_shader_stage_infos[2] = core->shaders[2].stage_info;
	core->pipeline_shader_stage_infos[3] = core->shaders[3].stage_info;
	core->pipeline_shader_stage_infos[4] = core->shaders[4].stage_info;
	core->pipeline_shader_stage_infos[5] = core->shaders[5].stage_info;
	core->pipeline_shader_stage_infos[6] = core->shaders[6].stage_info;
	core->pipeline_shader_stage_infos[7] = core->shaders[7].stage_info;

	core->dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
	core->dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;

	core->pipeline_dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	core->pipeline_dynamic_state_info.pDynamicStates = core->dynamic_states;
	core->pipeline_dynamic_state_info.dynamicStateCount = 2;
	core->pipeline_dynamic_state_info.flags = 0;
	core->pipeline_dynamic_state_info.pNext = nullptr;

	core->vertex_input_binding_descriptions[0].binding = 0;
	core->vertex_input_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	core->vertex_input_binding_descriptions[0].stride = sizeof(lightray_vertex_t);

	core->vertex_input_binding_descriptions[1].binding = 1;
	core->vertex_input_binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	core->vertex_input_binding_descriptions[1].stride = sizeof(lightray_render_instance_t);

	core->overlay_vertex_input_binding_descriptions[0].binding = 0;
	core->overlay_vertex_input_binding_descriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	core->overlay_vertex_input_binding_descriptions[0].stride = sizeof(lightray_vertex_t);

	core->overlay_vertex_input_binding_descriptions[1].binding = 1;
	core->overlay_vertex_input_binding_descriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	core->overlay_vertex_input_binding_descriptions[1].stride = sizeof(lightray_render_instance_glyph_t);

	core->static_mesh_vertex_attribute_descriptions[0].binding = 0;
	core->static_mesh_vertex_attribute_descriptions[0].location = 0;
	core->static_mesh_vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->static_mesh_vertex_attribute_descriptions[0].offset = offsetof(lightray_vertex_t, position);

	core->static_mesh_vertex_attribute_descriptions[1].binding = 0;
	core->static_mesh_vertex_attribute_descriptions[1].location = 1;
	core->static_mesh_vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->static_mesh_vertex_attribute_descriptions[1].offset = offsetof(lightray_vertex_t, normal);

	core->static_mesh_vertex_attribute_descriptions[2].binding = 1;
	core->static_mesh_vertex_attribute_descriptions[2].location = 2;
	core->static_mesh_vertex_attribute_descriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->static_mesh_vertex_attribute_descriptions[2].offset = 0;

	core->static_mesh_vertex_attribute_descriptions[3].binding = 1;
	core->static_mesh_vertex_attribute_descriptions[3].location = 3;
	core->static_mesh_vertex_attribute_descriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->static_mesh_vertex_attribute_descriptions[3].offset = sizeof(glm::vec4);

	core->static_mesh_vertex_attribute_descriptions[4].binding = 1;
	core->static_mesh_vertex_attribute_descriptions[4].location = 4;
	core->static_mesh_vertex_attribute_descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->static_mesh_vertex_attribute_descriptions[4].offset = sizeof(glm::vec4) * 2;

	core->static_mesh_vertex_attribute_descriptions[5].binding = 1;
	core->static_mesh_vertex_attribute_descriptions[5].location = 5;
	core->static_mesh_vertex_attribute_descriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->static_mesh_vertex_attribute_descriptions[5].offset = sizeof(glm::vec4) * 3;

	core->textured_static_mesh_vertex_attribute_descriptions[0].binding = 0;
	core->textured_static_mesh_vertex_attribute_descriptions[0].location = 0;
	core->textured_static_mesh_vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[0].offset = offsetof(lightray_vertex_t, position);

	core->textured_static_mesh_vertex_attribute_descriptions[1].binding = 0;
	core->textured_static_mesh_vertex_attribute_descriptions[1].location = 1;
	core->textured_static_mesh_vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[1].offset = offsetof(lightray_vertex_t, normal);

	core->textured_static_mesh_vertex_attribute_descriptions[2].binding = 0;
	core->textured_static_mesh_vertex_attribute_descriptions[2].location = 2;
	core->textured_static_mesh_vertex_attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[2].offset = offsetof(lightray_vertex_t, uv);

	core->textured_static_mesh_vertex_attribute_descriptions[3].binding = 1;
	core->textured_static_mesh_vertex_attribute_descriptions[3].location = 3;
	core->textured_static_mesh_vertex_attribute_descriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[3].offset = 0;

	core->textured_static_mesh_vertex_attribute_descriptions[4].binding = 1;
	core->textured_static_mesh_vertex_attribute_descriptions[4].location = 4;
	core->textured_static_mesh_vertex_attribute_descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[4].offset = sizeof(glm::vec4);

	core->textured_static_mesh_vertex_attribute_descriptions[5].binding = 1;
	core->textured_static_mesh_vertex_attribute_descriptions[5].location = 5;
	core->textured_static_mesh_vertex_attribute_descriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[5].offset = sizeof(glm::vec4) * 2;

	core->textured_static_mesh_vertex_attribute_descriptions[6].binding = 1;
	core->textured_static_mesh_vertex_attribute_descriptions[6].location = 6;
	core->textured_static_mesh_vertex_attribute_descriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[6].offset = sizeof(glm::vec4) * 3;

	core->textured_static_mesh_vertex_attribute_descriptions[7].binding = 1;
	core->textured_static_mesh_vertex_attribute_descriptions[7].location = 7;
	core->textured_static_mesh_vertex_attribute_descriptions[7].format = VK_FORMAT_R32_SFLOAT;
	core->textured_static_mesh_vertex_attribute_descriptions[7].offset = offsetof(lightray_render_instance_t, layer_index);


	core->skeletal_mesh_vertex_attribute_descriptions[0].binding = 0;
	core->skeletal_mesh_vertex_attribute_descriptions[0].location = 0;
	core->skeletal_mesh_vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->skeletal_mesh_vertex_attribute_descriptions[0].offset = offsetof(lightray_vertex_t, position);

	core->skeletal_mesh_vertex_attribute_descriptions[1].binding = 0;
	core->skeletal_mesh_vertex_attribute_descriptions[1].location = 1;
	core->skeletal_mesh_vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->skeletal_mesh_vertex_attribute_descriptions[1].offset = offsetof(lightray_vertex_t, normal);

	core->skeletal_mesh_vertex_attribute_descriptions[2].binding = 0;
	core->skeletal_mesh_vertex_attribute_descriptions[2].location = 2;
	core->skeletal_mesh_vertex_attribute_descriptions[2].format = VK_FORMAT_R8G8B8A8_UINT;
	core->skeletal_mesh_vertex_attribute_descriptions[2].offset = offsetof(lightray_vertex_t, bone_indices);

	core->skeletal_mesh_vertex_attribute_descriptions[3].binding = 0;
	core->skeletal_mesh_vertex_attribute_descriptions[3].location = 3;
	core->skeletal_mesh_vertex_attribute_descriptions[3].format = VK_FORMAT_R8G8B8A8_UINT;
	core->skeletal_mesh_vertex_attribute_descriptions[3].offset = offsetof(lightray_vertex_t, weights);

	core->skeletal_mesh_vertex_attribute_descriptions[4].binding = 1;
	core->skeletal_mesh_vertex_attribute_descriptions[4].location = 4;
	core->skeletal_mesh_vertex_attribute_descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->skeletal_mesh_vertex_attribute_descriptions[4].offset = 0;

	core->skeletal_mesh_vertex_attribute_descriptions[5].binding = 1;
	core->skeletal_mesh_vertex_attribute_descriptions[5].location = 5;
	core->skeletal_mesh_vertex_attribute_descriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->skeletal_mesh_vertex_attribute_descriptions[5].offset = sizeof(glm::vec4);

	core->skeletal_mesh_vertex_attribute_descriptions[6].binding = 1;
	core->skeletal_mesh_vertex_attribute_descriptions[6].location = 6;
	core->skeletal_mesh_vertex_attribute_descriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->skeletal_mesh_vertex_attribute_descriptions[6].offset = sizeof(glm::vec4) * 2;

	core->skeletal_mesh_vertex_attribute_descriptions[7].binding = 1;
	core->skeletal_mesh_vertex_attribute_descriptions[7].location = 7;
	core->skeletal_mesh_vertex_attribute_descriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->skeletal_mesh_vertex_attribute_descriptions[7].offset = sizeof(glm::vec4) * 3;

	core->skeletal_mesh_vertex_attribute_descriptions[8].binding = 1;
	core->skeletal_mesh_vertex_attribute_descriptions[8].location = 8;
	core->skeletal_mesh_vertex_attribute_descriptions[8].format = VK_FORMAT_R32_UINT;
	core->skeletal_mesh_vertex_attribute_descriptions[8].offset = offsetof(lightray_render_instance_t, computed_bone_transform_matrix_buffer_offset_with_respect_to_instance);

	core->overlay_vertex_attribute_descriptions[0].binding = 0;
	core->overlay_vertex_attribute_descriptions[0].location = 0;
	core->overlay_vertex_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[0].offset = offsetof(lightray_vertex_t, position);

	core->overlay_vertex_attribute_descriptions[1].binding = 0;
	core->overlay_vertex_attribute_descriptions[1].location = 1;
	core->overlay_vertex_attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[1].offset = offsetof(lightray_vertex_t, uv);

	core->overlay_vertex_attribute_descriptions[2].binding = 1;
	core->overlay_vertex_attribute_descriptions[2].location = 2;
	core->overlay_vertex_attribute_descriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[2].offset = 0;

	core->overlay_vertex_attribute_descriptions[3].binding = 1;
	core->overlay_vertex_attribute_descriptions[3].location = 3;
	core->overlay_vertex_attribute_descriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[3].offset = sizeof(glm::vec4);

	core->overlay_vertex_attribute_descriptions[4].binding = 1;
	core->overlay_vertex_attribute_descriptions[4].location = 4;
	core->overlay_vertex_attribute_descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[4].offset = sizeof(glm::vec4) * 2;

	core->overlay_vertex_attribute_descriptions[5].binding = 1;
	core->overlay_vertex_attribute_descriptions[5].location = 5;
	core->overlay_vertex_attribute_descriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[5].offset = sizeof(glm::vec4) * 3;

	core->overlay_vertex_attribute_descriptions[6].binding = 1;
	core->overlay_vertex_attribute_descriptions[6].location = 6;
	core->overlay_vertex_attribute_descriptions[6].format = VK_FORMAT_R32G32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[6].offset = offsetof(lightray_render_instance_glyph_t, uv_min);

	core->overlay_vertex_attribute_descriptions[7].binding = 1;
	core->overlay_vertex_attribute_descriptions[7].location = 7;
	core->overlay_vertex_attribute_descriptions[7].format = VK_FORMAT_R32G32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[7].offset = offsetof(lightray_render_instance_glyph_t, uv_max);

	core->overlay_vertex_attribute_descriptions[8].binding = 1;
	core->overlay_vertex_attribute_descriptions[8].location = 8;
	core->overlay_vertex_attribute_descriptions[8].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->overlay_vertex_attribute_descriptions[8].offset = offsetof(lightray_render_instance_glyph_t, color);

	core->pipeline_vertex_input_state_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	core->pipeline_vertex_input_state_infos[0].pVertexBindingDescriptions = core->vertex_input_binding_descriptions;
	core->pipeline_vertex_input_state_infos[0].vertexBindingDescriptionCount = 2;
	core->pipeline_vertex_input_state_infos[0].pVertexAttributeDescriptions = core->static_mesh_vertex_attribute_descriptions;
	core->pipeline_vertex_input_state_infos[0].vertexAttributeDescriptionCount = 6; // opaque untextured static mesh layout

	core->pipeline_vertex_input_state_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	core->pipeline_vertex_input_state_infos[1].pVertexBindingDescriptions = core->vertex_input_binding_descriptions;
	core->pipeline_vertex_input_state_infos[1].vertexBindingDescriptionCount = 2;
	core->pipeline_vertex_input_state_infos[1].pVertexAttributeDescriptions = core->textured_static_mesh_vertex_attribute_descriptions;
	core->pipeline_vertex_input_state_infos[1].vertexAttributeDescriptionCount = 8; // opaque textured static mesh layout

	core->pipeline_vertex_input_state_infos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	core->pipeline_vertex_input_state_infos[2].pVertexBindingDescriptions = core->vertex_input_binding_descriptions;
	core->pipeline_vertex_input_state_infos[2].vertexBindingDescriptionCount = 2;
	core->pipeline_vertex_input_state_infos[2].pVertexAttributeDescriptions = core->skeletal_mesh_vertex_attribute_descriptions;
	core->pipeline_vertex_input_state_infos[2].vertexAttributeDescriptionCount = 9; // opaque untextured skeletal mesh layout

	core->pipeline_vertex_input_state_infos[3].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	core->pipeline_vertex_input_state_infos[3].pVertexBindingDescriptions = core->overlay_vertex_input_binding_descriptions;
	core->pipeline_vertex_input_state_infos[3].vertexBindingDescriptionCount = 2;
	core->pipeline_vertex_input_state_infos[3].pVertexAttributeDescriptions = core->overlay_vertex_attribute_descriptions;
	core->pipeline_vertex_input_state_infos[3].vertexAttributeDescriptionCount = 9; // overlay layout

	core->pipeline_input_assembly_state_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	core->pipeline_input_assembly_state_infos[0].primitiveRestartEnable = VK_FALSE;
	core->pipeline_input_assembly_state_infos[0].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	core->pipeline_input_assembly_state_infos[0].pNext = nullptr;

	core->viewport.x = 0.0f;
	core->viewport.y = 0.0f;
	core->viewport.width = (f32)core->swapchain_info.imageExtent.width;
	core->viewport.height = (f32)core->swapchain_info.imageExtent.height;
	core->viewport.minDepth = 0.0f;
	core->viewport.maxDepth = 1.0f;

	core->scissor.offset = { 0,0 };
	core->scissor.extent = core->swapchain_info.imageExtent;

	core->clear_values[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	core->clear_values[1].depthStencil = { 1.0f, 0 };

	core->pipeline_viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	core->pipeline_viewport_state_info.pViewports = &core->viewport;
	core->pipeline_viewport_state_info.viewportCount = 1;
	core->pipeline_viewport_state_info.pScissors = &core->scissor;
	core->pipeline_viewport_state_info.scissorCount = 1;
	core->pipeline_viewport_state_info.flags = 0;
	core->pipeline_viewport_state_info.pNext = nullptr;

	// opaque
	core->pipeline_rasterization_state_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	core->pipeline_rasterization_state_infos[0].depthClampEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[0].rasterizerDiscardEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[0].polygonMode = VK_POLYGON_MODE_FILL;
	core->pipeline_rasterization_state_infos[0].lineWidth = 1.0f;
	core->pipeline_rasterization_state_infos[0].cullMode = VK_CULL_MODE_BACK_BIT;
	core->pipeline_rasterization_state_infos[0].frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	core->pipeline_rasterization_state_infos[0].depthBiasEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[0].depthBiasConstantFactor = 0.0f;
	core->pipeline_rasterization_state_infos[0].depthBiasClamp = 0.0f;
	core->pipeline_rasterization_state_infos[0].depthBiasSlopeFactor = 0.0f;
	core->pipeline_rasterization_state_infos[0].flags = 0;
	core->pipeline_rasterization_state_infos[0].pNext = nullptr;

	// wireframe 
	core->pipeline_rasterization_state_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	core->pipeline_rasterization_state_infos[1].depthClampEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[1].rasterizerDiscardEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[1].polygonMode = VK_POLYGON_MODE_LINE;
	core->pipeline_rasterization_state_infos[1].lineWidth = 1.0f;
	core->pipeline_rasterization_state_infos[1].cullMode = VK_CULL_MODE_NONE;
	core->pipeline_rasterization_state_infos[1].frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	core->pipeline_rasterization_state_infos[1].depthBiasEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[1].depthBiasConstantFactor = 0.0f;
	core->pipeline_rasterization_state_infos[1].depthBiasClamp = 0.0f;
	core->pipeline_rasterization_state_infos[1].depthBiasSlopeFactor = 0.0f;
	core->pipeline_rasterization_state_infos[1].flags = 0;
	core->pipeline_rasterization_state_infos[1].pNext = nullptr;

	// overlay
	core->pipeline_rasterization_state_infos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	core->pipeline_rasterization_state_infos[2].depthClampEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[2].rasterizerDiscardEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[2].polygonMode = VK_POLYGON_MODE_FILL;
	core->pipeline_rasterization_state_infos[2].lineWidth = 1.0f;
	core->pipeline_rasterization_state_infos[2].cullMode = VK_CULL_MODE_NONE;
	core->pipeline_rasterization_state_infos[2].frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	core->pipeline_rasterization_state_infos[2].depthBiasEnable = VK_FALSE;
	core->pipeline_rasterization_state_infos[2].depthBiasConstantFactor = 0.0f;
	core->pipeline_rasterization_state_infos[2].depthBiasClamp = 0.0f;
	core->pipeline_rasterization_state_infos[2].depthBiasSlopeFactor = 0.0f;
	core->pipeline_rasterization_state_infos[2].flags = 0;
	core->pipeline_rasterization_state_infos[2].pNext = nullptr;

	core->pipeline_multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	core->pipeline_multisample_state_info.sampleShadingEnable = VK_FALSE;
	core->pipeline_multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	core->pipeline_multisample_state_info.minSampleShading = 1.0f;
	core->pipeline_multisample_state_info.pSampleMask = nullptr;
	core->pipeline_multisample_state_info.alphaToCoverageEnable = VK_FALSE;
	core->pipeline_multisample_state_info.alphaToOneEnable = VK_FALSE;
	core->pipeline_multisample_state_info.flags = 0;
	core->pipeline_multisample_state_info.pNext = nullptr;

	core->pipeline_color_blend_attachment_state.blendEnable = VK_TRUE;
	core->pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	core->pipeline_color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	core->pipeline_color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	core->pipeline_color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	core->pipeline_color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	core->pipeline_color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	core->pipeline_color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	core->pipeline_color_blend_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	core->pipeline_color_blend_state_info.logicOpEnable = VK_FALSE;
	core->pipeline_color_blend_state_info.logicOp = VK_LOGIC_OP_AND;
	core->pipeline_color_blend_state_info.attachmentCount = 1;
	core->pipeline_color_blend_state_info.pAttachments = &core->pipeline_color_blend_attachment_state;
	core->pipeline_color_blend_state_info.blendConstants[0] = 0.0f;
	core->pipeline_color_blend_state_info.blendConstants[1] = 0.0f;
	core->pipeline_color_blend_state_info.blendConstants[2] = 0.0f;
	core->pipeline_color_blend_state_info.blendConstants[3] = 0.0f;
	core->pipeline_color_blend_state_info.flags = 0;
	core->pipeline_color_blend_state_info.pNext = nullptr;

	core->pipeline_layout_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	core->pipeline_layout_infos[0].pSetLayouts = &core->main_descriptor_set_layout;
	core->pipeline_layout_infos[0].setLayoutCount = 1;
	core->pipeline_layout_infos[0].pushConstantRangeCount = 0;
	core->pipeline_layout_infos[0].pPushConstantRanges = nullptr;
	core->pipeline_layout_infos[0].flags = 0;
	core->pipeline_layout_infos[0].pNext = nullptr;

	const VkResult  pipeline_layout_creation_result = vkCreatePipelineLayout(core->logical_device, &core->pipeline_layout_infos[0], nullptr, &core->pipeline_layouts[0]);

	VkAttachmentDescription depth_attachment{};
	depth_attachment.format = supported_depth_fomat;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depth_attachment.flags = 0;

	VkAttachmentReference depth_attachment_reference{};
	depth_attachment_reference.attachment = 1;
	depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	core->attachment_description.format = core->swapchain_info.imageFormat;
	core->attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	core->attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	core->attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	core->attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	core->attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	core->attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	core->attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	core->attachment_description.flags = 0;

	core->attachment_reference.attachment = 0;
	core->attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	core->subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	core->subpass_description.pColorAttachments = &core->attachment_reference;
	core->subpass_description.colorAttachmentCount = 1;
	core->subpass_description.pDepthStencilAttachment = &depth_attachment_reference;
	core->subpass_description.inputAttachmentCount = 0;
	core->subpass_description.pInputAttachments = nullptr;
	core->subpass_description.pPreserveAttachments = nullptr;
	core->subpass_description.preserveAttachmentCount = 0;
	core->subpass_description.pResolveAttachments = nullptr;
	core->subpass_description.flags = 0;

	core->subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	core->subpass_dependency.dstSubpass = 0;
	core->subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	core->subpass_dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	core->subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	core->subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[2]{};
	attachments[0] = core->attachment_description;
	attachments[1] = depth_attachment;

	core->render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	core->render_pass_info.pAttachments = attachments;
	core->render_pass_info.attachmentCount = 2;
	core->render_pass_info.subpassCount = 1;
	core->render_pass_info.pSubpasses = &core->subpass_description;
	core->render_pass_info.dependencyCount = 1;
	core->render_pass_info.pDependencies = &core->subpass_dependency;
	core->render_pass_info.flags = 0;
	core->render_pass_info.pNext = nullptr;

	const VkResult render_pass_creation_result = vkCreateRenderPass(core->logical_device, &core->render_pass_info, nullptr, &core->render_pass);

	lightray_vulkan_create_swapchain_framebuffers(core);

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info{};
	depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_info.depthTestEnable = VK_TRUE;
	depth_stencil_state_info.depthWriteEnable = VK_TRUE;
	depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil_state_info.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state_info.minDepthBounds = 0.0f;
	depth_stencil_state_info.maxDepthBounds = 1.0f;
	depth_stencil_state_info.stencilTestEnable = VK_FALSE;
	depth_stencil_state_info.front = {};
	depth_stencil_state_info.back = {};
	depth_stencil_state_info.flags = 0;
	depth_stencil_state_info.pNext = nullptr;

	VkPipelineDepthStencilStateCreateInfo overlay_depth_stencil_state_info{};
	overlay_depth_stencil_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	overlay_depth_stencil_state_info.depthTestEnable = VK_FALSE;
	overlay_depth_stencil_state_info.depthWriteEnable = VK_FALSE;
	overlay_depth_stencil_state_info.depthCompareOp = VK_COMPARE_OP_NEVER;
	overlay_depth_stencil_state_info.minDepthBounds = 0.0f;
	overlay_depth_stencil_state_info.maxDepthBounds = 0.0f;
	overlay_depth_stencil_state_info.stencilTestEnable = VK_FALSE;
	overlay_depth_stencil_state_info.front = {};
	overlay_depth_stencil_state_info.back = {};
	overlay_depth_stencil_state_info.flags = 0;
	overlay_depth_stencil_state_info.pNext = nullptr;

	// static mesh pipeline
	core->graphics_pipeline_infos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[0].stageCount = 2;
	core->graphics_pipeline_infos[0].pStages = core->pipeline_shader_stage_infos;
	core->graphics_pipeline_infos[0].pVertexInputState = &core->pipeline_vertex_input_state_infos[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_SHADER_SET_STARTING_INDEX];
	core->graphics_pipeline_infos[0].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[0].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[0].pRasterizationState = &core->pipeline_rasterization_state_infos[0];
	core->graphics_pipeline_infos[0].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[0].pDepthStencilState = &depth_stencil_state_info;
	core->graphics_pipeline_infos[0].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[0].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[0].layout = core->pipeline_layouts[0];
	core->graphics_pipeline_infos[0].renderPass = core->render_pass;
	core->graphics_pipeline_infos[0].subpass = 0;
	core->graphics_pipeline_infos[0].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[0].basePipelineIndex = -1;
	core->graphics_pipeline_infos[0].flags = 0;
	core->graphics_pipeline_infos[0].pNext = nullptr;

	// wireframe pipeline, not used
	core->graphics_pipeline_infos[1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[1].stageCount = 2;
	core->graphics_pipeline_infos[1].pStages = core->pipeline_shader_stage_infos;
	core->graphics_pipeline_infos[1].pVertexInputState = &core->pipeline_vertex_input_state_infos[LIGHTRAY_VULKAN_UNTEXTURED_STATIC_MESH_SHADER_SET_STARTING_INDEX];
	core->graphics_pipeline_infos[1].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[1].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[1].pRasterizationState = &core->pipeline_rasterization_state_infos[1];
	core->graphics_pipeline_infos[1].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[1].pDepthStencilState = &depth_stencil_state_info;
	core->graphics_pipeline_infos[1].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[1].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[1].layout = core->pipeline_layouts[0];
	core->graphics_pipeline_infos[1].renderPass = core->render_pass;
	core->graphics_pipeline_infos[1].subpass = 0;
	core->graphics_pipeline_infos[1].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[1].basePipelineIndex = -1;
	core->graphics_pipeline_infos[1].flags = 0;
	core->graphics_pipeline_infos[1].pNext = nullptr;

	// textured static mesh pipeline
	core->graphics_pipeline_infos[2].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[2].stageCount = 2;
	core->graphics_pipeline_infos[2].pStages = &core->pipeline_shader_stage_infos[LIGHTRAY_VULKAN_TEXTURED_STATIC_MESH_SHADER_SET_STARTING_INDEX];
	core->graphics_pipeline_infos[2].pVertexInputState = &core->pipeline_vertex_input_state_infos[1];
	core->graphics_pipeline_infos[2].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[2].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[2].pRasterizationState = &core->pipeline_rasterization_state_infos[0];
	core->graphics_pipeline_infos[2].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[2].pDepthStencilState = &depth_stencil_state_info;
	core->graphics_pipeline_infos[2].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[2].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[2].layout = core->pipeline_layouts[0];
	core->graphics_pipeline_infos[2].renderPass = core->render_pass;
	core->graphics_pipeline_infos[2].subpass = 0;
	core->graphics_pipeline_infos[2].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[2].basePipelineIndex = -1;
	core->graphics_pipeline_infos[2].flags = 0;
	core->graphics_pipeline_infos[2].pNext = nullptr;

	// skeletal mesh pipeline
	core->graphics_pipeline_infos[3].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[3].stageCount = 2;
	core->graphics_pipeline_infos[3].pStages = &core->pipeline_shader_stage_infos[LIGHTRAY_VULKAN_UNTEXTURED_SKELETAL_MESH_SHADER_SET_STARTING_INDEX];
	core->graphics_pipeline_infos[3].pVertexInputState = &core->pipeline_vertex_input_state_infos[2];
	core->graphics_pipeline_infos[3].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[3].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[3].pRasterizationState = &core->pipeline_rasterization_state_infos[0];
	core->graphics_pipeline_infos[3].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[3].pDepthStencilState = &depth_stencil_state_info;
	core->graphics_pipeline_infos[3].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[3].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[3].layout = core->pipeline_layouts[0];
	core->graphics_pipeline_infos[3].renderPass = core->render_pass;
	core->graphics_pipeline_infos[3].subpass = 0;
	core->graphics_pipeline_infos[3].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[3].basePipelineIndex = -1;
	core->graphics_pipeline_infos[3].flags = 0;
	core->graphics_pipeline_infos[3].pNext = nullptr;

	// overlay pipeline
	core->graphics_pipeline_infos[4].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[4].stageCount = 2;
	core->graphics_pipeline_infos[4].pStages = &core->pipeline_shader_stage_infos[6];
	core->graphics_pipeline_infos[4].pVertexInputState = &core->pipeline_vertex_input_state_infos[3];
	core->graphics_pipeline_infos[4].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[4].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[4].pRasterizationState = &core->pipeline_rasterization_state_infos[2];
	core->graphics_pipeline_infos[4].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[4].pDepthStencilState = &overlay_depth_stencil_state_info;
	core->graphics_pipeline_infos[4].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[4].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[4].layout = core->pipeline_layouts[0];
	core->graphics_pipeline_infos[4].renderPass = core->render_pass;
	core->graphics_pipeline_infos[4].subpass = 0;
	core->graphics_pipeline_infos[4].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[4].basePipelineIndex = -1;
	core->graphics_pipeline_infos[4].flags = 0;
	core->graphics_pipeline_infos[4].pNext = nullptr;

	const VkResult graphics_pipelines_creation_result = vkCreateGraphicsPipelines(core->logical_device, nullptr, 5, core->graphics_pipeline_infos, nullptr, core->pipelines);

	core->command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	core->command_pool_info.queueFamilyIndex = graphics_queue_family_index;
	core->command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	core->command_pool_info.pNext = nullptr;

	const VkResult command_pool_creation_result =  vkCreateCommandPool(core->logical_device, &core->command_pool_info, nullptr, &core->command_pool);

	core->render_command_buffers_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	core->render_command_buffers_allocate_info.commandPool = core->command_pool;
	core->render_command_buffers_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	core->render_command_buffers_allocate_info.commandBufferCount = core->swapchain_image_in_use_count;
	core->render_command_buffers_allocate_info.pNext = nullptr;

	core->general_purpose_command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	core->general_purpose_command_buffer_allocate_info.commandPool = core->command_pool;
	core->general_purpose_command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	core->general_purpose_command_buffer_allocate_info.commandBufferCount = 1;
	core->general_purpose_command_buffer_allocate_info.pNext = nullptr;

	const VkResult render_command_buffer_allocation_result = vkAllocateCommandBuffers(core->logical_device, &core->render_command_buffers_allocate_info, core->render_command_buffers);
	const VkResult general_purpose_command_buffer_allocation_result = vkAllocateCommandBuffers(core->logical_device, &core->general_purpose_command_buffer_allocate_info, &core->general_purpose_command_buffer);

	core->semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	core->semaphore_info.flags = 0;
	core->semaphore_info.pNext = nullptr;

	core->inflight_fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	core->inflight_fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	core->inflight_fence_info.pNext = nullptr;

	for (u32 i = 0; i < core->swapchain_image_in_use_count; i++)
	{
		const VkResult image_available_for_rendering_semaphore_creation_result = vkCreateSemaphore(core->logical_device, &core->semaphore_info, nullptr, &core->image_available_for_rendering_semaphores[i]);
		const VkResult image_finished_rendering_semaphore_creation_result = vkCreateSemaphore(core->logical_device, &core->semaphore_info, nullptr, &core->image_finished_rendering_semaphores[i]);
		const VkResult inlfight_fence_creation_result = vkCreateFence(core->logical_device, &core->inflight_fence_info, nullptr, &core->inflight_fences[i]);
	}

	core->general_purpose_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	core->general_purpose_command_buffer_begin_info.flags = 0;
	core->general_purpose_command_buffer_begin_info.pNext = nullptr;
	core->general_purpose_command_buffer_begin_info.pInheritanceInfo = nullptr;

	u32 host_visible_vram_arena_texture_staging_buffer_current_index = core->host_visible_vram_arena_staging_texture_buffer_starting_index;
	u64 current_host_visible_vram_arena_buffer_copying_offset = core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_texture_staging_buffer_current_index];

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		sunder_buffer_copy_data_t texture_ram_buffers_to_host_visible_buffer_copying_data{};
		texture_ram_buffers_to_host_visible_buffer_copying_data.dst_size = core->host_visible_vram_arena.capacity;
		texture_ram_buffers_to_host_visible_buffer_copying_data.src_size = core->texture_buffer[i].size;
		texture_ram_buffers_to_host_visible_buffer_copying_data.bytes_to_write = core->texture_buffer[i].size;
		texture_ram_buffers_to_host_visible_buffer_copying_data.src_offset = 0;
		texture_ram_buffers_to_host_visible_buffer_copying_data.dst_offset = current_host_visible_vram_arena_buffer_copying_offset;

		lightray_vulkan_texture_population_data_t texture_population_data{};
		texture_population_data.queue = core->graphics_queue;
		texture_population_data.command_buffer = core->general_purpose_command_buffer;
		texture_population_data.command_buffer_begin_info = &core->general_purpose_command_buffer_begin_info;
		texture_population_data.buffer = core->host_visible_vram_arena.buffer;
		texture_population_data.buffer_offset = current_host_visible_vram_arena_buffer_copying_offset;
		texture_population_data.starting_layer = 0;
		texture_population_data.layer_count = 1;

		const u64 bytes_written_to_host_visible_buffer = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->texture_buffer[i].buffer, &texture_ram_buffers_to_host_visible_buffer_copying_data);

		lightray_vulkan_transition_texture_layout(core->graphics_queue, core->general_purpose_command_buffer, &core->general_purpose_command_buffer_begin_info, &core->texture_buffer[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		lightray_vulkan_populate_texture(&core->texture_buffer[i], &texture_population_data);
		lightray_vulkan_transition_texture_layout(core->graphics_queue, core->general_purpose_command_buffer, &core->general_purpose_command_buffer_begin_info, &core->texture_buffer[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		host_visible_vram_arena_texture_staging_buffer_current_index++;
		host_visible_vram_arena_texture_staging_buffer_current_index = sunder_clamp_u32(0, host_visible_vram_arena_allocation_count - 1, host_visible_vram_arena_texture_staging_buffer_current_index);
		current_host_visible_vram_arena_buffer_copying_offset = core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_texture_staging_buffer_current_index];
	}

	const u8 texture_ram_buffer_free_flags = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_RAM_BUFFER_BIT }, 1);

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		const lightray_vulkan_result texture_ram_buffer_free_result = lightray_vulkan_free_texture(core, &core->texture_buffer[i], texture_ram_buffer_free_flags);
	}

	for (u32 i = 0; i < core->total_texture_count; i++)
	{
		SUNDER_LOG("\n\ntiling: ");
		SUNDER_LOG(core->texture_buffer[i].tiling);
		SUNDER_LOG("\nformat: ");
		SUNDER_LOG(core->texture_buffer[i].format);
		SUNDER_LOG("\nfilter: ");
		SUNDER_LOG(core->texture_buffer[i].filter);
		SUNDER_LOG("\naddress_mode: ");
		SUNDER_LOG(core->texture_buffer[i].address_mode);
		SUNDER_LOG("\nsampler_index: ");
		SUNDER_LOG(core->texture_buffer[i].sampler_index);
		SUNDER_LOG("\nwidth: ");
		SUNDER_LOG(core->texture_buffer[i].width);
		SUNDER_LOG("\nheight: ");
		SUNDER_LOG(core->texture_buffer[i].height);
		SUNDER_LOG("\nkind: ");
		SUNDER_LOG(core->texture_buffer[i].kind);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// channel names allocation and copy
	u32 animation_channel_names_iter = 0;

	for (u32 a = animation_starting_offset; a < animation_range; a++)
	{	
		const aiAnimation* current_animation = scenes[a]->mAnimations[0];
		const u32 current_channel_count = current_animation->mNumChannels;

		for (u32 c = 0; c < current_channel_count; c++)
		{
			const aiNodeAnim* current_channel = current_animation->mChannels[c];
			const u32 current_channel_name_length = current_channel->mNodeName.length;
			cstring_literal* current_channel_name = current_channel->mNodeName.C_Str();
			lightray_populate_node_related_string_upon_suballocation(&core->general_purpose_ram_arena, &core->animation_core.animation_channel_names[animation_channel_names_iter], current_channel_name, current_channel_name_length);
			animation_channel_names_iter++;
		}
	}

	for (u32 i = 0; i < core->animation_core.total_animation_channel_count; i++)
	{
		SUNDER_LOG("\n");
		sunder_log_string(&core->animation_core.animation_channel_names[i]);
	}

	// bone names allocation and copy
	u32 bone_names_iter = 0;

	for (u32 sm = skeletal_mesh_starting_offset; sm < skeletal_mesh_range; sm++)
	{
		const u32 current_bone_count = meshes[sm]->mNumBones;

		for (u32 b = 0; b < current_bone_count; b++)
		{
			const u32 current_bone_name_length = meshes[sm]->mBones[b]->mName.length;
			cstring_literal* current_bone_name_cstring = meshes[sm]->mBones[b]->mName.C_Str();
			lightray_populate_node_related_string_upon_suballocation(&core->general_purpose_ram_arena, &core->animation_core.bone_names[bone_names_iter], current_bone_name_cstring, current_bone_name_length);
			bone_names_iter++;
		}
	}
	
	for (u32 i = 0; i < core->animation_core.total_bone_count; i++)
	{
		SUNDER_LOG("\n");
		sunder_log_string(&core->animation_core.bone_names[i]);
	}

	// node buffer population 2 passes
	u32 skeleton_buffer_iter = 0;
	u32 current_skeletal_mesh_node_buffer_index_first_pass = 0;
	u32 current_skeletal_mesh_node_buffer_index_second_pass = 0;

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		core->animation_core.skeleton_buffer[skeleton_buffer_iter].node_buffer_offset = current_skeletal_mesh_node_buffer_index_first_pass;
		lightray_assimp_execute_first_node_buffer_population_pass(scenes[i]->mRootNode, core->animation_core.node_buffer, &core->general_purpose_ram_arena, core->animation_core.node_names,  &current_skeletal_mesh_node_buffer_index_first_pass);
		core->animation_core.skeleton_buffer[skeleton_buffer_iter].node_count = current_skeletal_mesh_node_buffer_index_first_pass;
		lightray_assimp_execute_second_node_buffer_population_pass(scenes[i]->mRootNode, core->animation_core.node_buffer, core->animation_core.skeleton_buffer[skeleton_buffer_iter].node_count, core->animation_core.node_names, &current_skeletal_mesh_node_buffer_index_second_pass);
	}

	for (u32 i = 0; i < core->animation_core.total_node_count; i++)
	{
		core->animation_core.node_buffer[i].bone_buffer_index = LIGHTRAY_INVALID_NODE_BONE_BUFFER_INDEX;
		core->animation_core.node_buffer[i].animation_channel_buffer_index = LIGHTRAY_INVALID_NODE_ANIMATION_CHANNEL_BUFFER_INDEX;
	}

	u32 bone_buffer_iter = 0;
	u32 computed_bone_transform_matrix_buffer_offset = 0;
	u32 bone_buffer_offset = 0;
	skeleton_buffer_iter = 0;
	skeletal_mesh_metadata_buffer_iter = 0;

	// skeleton buffer population
	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		const aiMesh* current_mesh = meshes[i];

		for (u32 j = 0; j < current_mesh->mNumBones; j++)
		{
			const aiBone* current_bone = current_mesh->mBones[j];
			core->animation_core.bone_buffer[bone_buffer_iter].inverse_bind_pose_matrix = lightray_assimp_to_glm_mat4(current_bone->mOffsetMatrix);
			bone_buffer_iter++;
		}

		core->animation_core.skeleton_buffer[skeleton_buffer_iter].bone_buffer_offset = bone_buffer_offset;
		core->animation_core.skeleton_buffer[skeleton_buffer_iter].computed_bone_transform_matrix_buffer_offset = computed_bone_transform_matrix_buffer_offset;

		const u32 skeletal_mesh_instance_bone_count = lightray_compute_skeletal_mesh_bone_count_with_respect_to_instance_count(current_mesh->mNumBones, initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count);  //current_mesh->mNumBones* setup_data->skeletal_mesh_metadata_buffer[skeleton_mesh_metadata_buffer_iter2].instance_count;
		core->animation_core.skeleton_buffer[skeleton_buffer_iter].bone_count = current_mesh->mNumBones;
		core->animation_core.skeleton_buffer[skeleton_buffer_iter].computed_bone_transform_matrix_buffer_bone_count = skeletal_mesh_instance_bone_count;
		core->animation_core.skeleton_buffer[skeleton_buffer_iter].instance_count = initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count;

		bone_buffer_offset += current_mesh->mNumBones;
		computed_bone_transform_matrix_buffer_offset += skeletal_mesh_instance_bone_count;
		skeletal_mesh_metadata_buffer_iter++;
		skeleton_buffer_iter++;
	}

	SUNDER_LOG("\n\nskeleton buffer :\n");
	SUNDER_LOG("\n=================================================================\n");

	for (u32 i = 0; i < core->animation_core.total_skeleton_count; i++)
	{
		SUNDER_LOG("bone buffer offset: ");
		SUNDER_LOG(core->animation_core.skeleton_buffer[i].bone_buffer_offset);
		SUNDER_LOG("\nbone count: ");
		SUNDER_LOG(core->animation_core.skeleton_buffer[i].bone_count);
		SUNDER_LOG("\ncomputed_bone_transform_matrix_buffer_offset: ");
		SUNDER_LOG(core->animation_core.skeleton_buffer[i].computed_bone_transform_matrix_buffer_offset);
		SUNDER_LOG("\ncomputed_bone_transform_matrix_buffer_bone_count: ");
		SUNDER_LOG(core->animation_core.skeleton_buffer[i].computed_bone_transform_matrix_buffer_bone_count);
		SUNDER_LOG("\n=================================================================\n");
	}

	// animation, channel buffers population
	u32 animation_position_key_buffer_iter = 0;
	u32 animation_rotation_key_buffer_iter = 0;
	u32 animation_scale_key_buffer_iter = 0;

	u32 animation_channel_buffer_iter = 0;
	u32 animation_channel_buffer_offset = 0;

	u32 animation_position_key_buffer_offset = 0;
	u32 animation_rotation_key_buffer_offset = 0;
	u32 animation_scale_key_buffer_offset = 0;

	u32 animation_buffer_iter = 0;

	for (u32 i = animation_starting_offset; i < animation_range; i++)
	{
		const aiAnimation* current_animation = scenes[i]->mAnimations[0];
		const u32 current_channel_count = current_animation->mNumChannels;

		const f32 animation_tickrate = SUNDER_CAST2(f32)current_animation->mTicksPerSecond != 0 ? SUNDER_CAST(f32, current_animation->mTicksPerSecond) : LIGHTRAY_DEFAULT_ANIMATION_TICKRATE;
		core->animation_core.animation_buffer[animation_buffer_iter].tickrate = animation_tickrate;
		core->animation_core.animation_buffer[animation_buffer_iter].duration = SUNDER_CAST2(f32)current_animation->mDuration;
		core->animation_core.animation_buffer[animation_buffer_iter].channel_count = current_channel_count;
		core->animation_core.animation_buffer[animation_buffer_iter].channel_buffer_offset = animation_channel_buffer_offset;
		animation_channel_buffer_offset += current_channel_count;

		for (u32 j = 0; j < current_channel_count; j++)
		{
			const aiNodeAnim* current_channel = current_animation->mChannels[j];

			core->animation_core.animation_channel_buffer[animation_channel_buffer_iter].position_key_count = current_channel->mNumPositionKeys;
			core->animation_core.animation_channel_buffer[animation_channel_buffer_iter].position_key_buffer_offset = animation_position_key_buffer_offset;
			animation_position_key_buffer_offset += current_channel->mNumPositionKeys;

			core->animation_core.animation_channel_buffer[animation_channel_buffer_iter].rotation_key_count = current_channel->mNumRotationKeys;
			core->animation_core.animation_channel_buffer[animation_channel_buffer_iter].rotation_key_buffer_offset = animation_rotation_key_buffer_offset;
			animation_rotation_key_buffer_offset += current_channel->mNumRotationKeys;

			core->animation_core.animation_channel_buffer[animation_channel_buffer_iter].scale_key_count = current_channel->mNumScalingKeys;
			core->animation_core.animation_channel_buffer[animation_channel_buffer_iter].scale_key_buffer_offset = animation_scale_key_buffer_offset;
			animation_scale_key_buffer_offset += current_channel->mNumScalingKeys;

			for (u32 k = 0; k < current_channel->mNumPositionKeys; k++)
			{
				core->animation_core.animation_position_key_buffer[animation_position_key_buffer_iter].vec = lightray_assimp_to_glm_vec3(current_channel->mPositionKeys[k].mValue);
				core->animation_core.animation_position_key_buffer[animation_position_key_buffer_iter].time = SUNDER_CAST2(f32)current_channel->mPositionKeys[k].mTime;
				animation_position_key_buffer_iter++;
			}

			for (u32 k = 0; k < current_channel->mNumRotationKeys; k++)
			{
				core->animation_core.animation_rotation_key_buffer[animation_rotation_key_buffer_iter].quat = lightray_assimp_to_glm_quat(current_channel->mRotationKeys[k].mValue);
				core->animation_core.animation_rotation_key_buffer[animation_rotation_key_buffer_iter].time = SUNDER_CAST2(f32)current_channel->mRotationKeys[k].mTime;
				animation_rotation_key_buffer_iter++;
			}

			for (u32 k = 0; k < current_channel->mNumScalingKeys; k++)
			{
				core->animation_core.animation_scale_key_buffer[animation_scale_key_buffer_iter].vec = lightray_assimp_to_glm_vec3(current_channel->mScalingKeys[k].mValue);
				core->animation_core.animation_scale_key_buffer[animation_scale_key_buffer_iter].time = SUNDER_CAST2(f32)current_channel->mScalingKeys[k].mTime;
				animation_scale_key_buffer_iter++;
			}

			animation_channel_buffer_iter++;
		}

		animation_buffer_iter++;
	}

	// 3rd node buffer population pass
	for (u32 i = 0; i < core->animation_core.total_skeleton_count; i++)
	{
		const lightray_skeleton_t current_skeleton = core->animation_core.skeleton_buffer[i];

		const u32 current_node_buffer_offset = current_skeleton.node_buffer_offset;
		const u32 current_node_count = current_skeleton.node_count;

		const u32 current_bone_buffer_offset = current_skeleton.bone_buffer_offset;
		const u32 current_bone_count = current_skeleton.bone_count;

		for (u32 n = 0; n < current_node_count; n++)
		{
			const u32 current_node_buffer_index = current_node_buffer_offset + n;

			for (u32 b = 0; b < current_bone_count; b++)
			{
				const u32 current_bone_buffer_index = current_bone_buffer_offset + b;
				const bool node_maps_to_bone = sunder_compare_strings(core->animation_core.node_names[current_node_buffer_index].data, core->animation_core.node_names[current_node_buffer_index].length, core->animation_core.bone_names[current_bone_buffer_index].data, core->animation_core.bone_names[current_bone_buffer_index].length);

				if (node_maps_to_bone)
				{
					core->animation_core.node_buffer[current_node_buffer_index].bone_buffer_index = b;
					break;
				}
			}

			for (u32 a = 0; a < core->animation_core.total_animation_count; a++)
			{
				const lightray_animation_t current_animation = core->animation_core.animation_buffer[a];
				const u32 current_animation_channel_buffer_offset = current_animation.channel_buffer_offset;
				const u32 current_animation_channel_count = current_animation.channel_count;

				for (u32 c = 0; c < current_animation_channel_count; c++)
				{
					const u32 current_animation_channel_index = current_animation_channel_buffer_offset + c;
					const bool animation_channel_maps_to_node = sunder_compare_strings(core->animation_core.node_names[current_node_buffer_index].data, core->animation_core.node_names[current_node_buffer_index].length, core->animation_core.animation_channel_names[current_animation_channel_index].data, core->animation_core.animation_channel_names[current_animation_channel_index].length);

					if (animation_channel_maps_to_node)
					{
						core->animation_core.node_buffer[current_node_buffer_index].animation_channel_buffer_index = c;
						break;
					}
				}
			}
		}
	}

	SUNDER_LOG("\nnode buffer\n============================================================");
	for (u32 i = 0; i < core->animation_core.total_node_count; i++)
	{
		SUNDER_LOG("\nname: ");
		sunder_log_string(&core->animation_core.node_names[i]);
		SUNDER_LOG("\nparent_index: ");
		SUNDER_LOG(core->animation_core.node_buffer[i].parent_index);
		SUNDER_LOG("\nbone_buffer_index: ");
		SUNDER_LOG(core->animation_core.node_buffer[i].bone_buffer_index);
		SUNDER_LOG("\nanimation_channel_buffer_index: ");
		SUNDER_LOG(core->animation_core.node_buffer[i].animation_channel_buffer_index);
		SUNDER_LOG("\n============================================================");
	}

	// skeleton bind pose computation
	skeleton_buffer_iter = 0;

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		const aiNode* current_root_node = scenes[i]->mRootNode;
		core->animation_core.global_root_inverse_matrix_buffer[skeleton_buffer_iter] = glm::inverse(lightray_assimp_to_glm_mat4(current_root_node->mTransformation));
		const u32 current_node_count = core->animation_core.skeleton_buffer[skeleton_buffer_iter].node_count;
		const u32 current_node_buffer_offset = core->animation_core.skeleton_buffer[skeleton_buffer_iter].node_buffer_offset;
		const u32 current_skeleton_bone_buffer_offset = core->animation_core.skeleton_buffer[skeleton_buffer_iter].bone_buffer_offset;
		core->animation_core.node_buffer[current_node_buffer_offset].global_transform_matrix = core->animation_core.node_buffer[current_node_buffer_offset].local_transform_matrix; // this expects correct execution from all of the node related code above

		for (u32 n = 1; n < current_node_count; n++)
		{
			const u32 current_node_index = current_node_buffer_offset + n;
			const u32 current_parent_node_index = current_node_buffer_offset + core->animation_core.node_buffer[current_node_index].parent_index;
			const u32 current_bone_buffer_index = core->animation_core.node_buffer[current_node_index].bone_buffer_index;

			core->animation_core.node_buffer[current_node_index].global_transform_matrix = core->animation_core.node_buffer[current_parent_node_index].global_transform_matrix * core->animation_core.node_buffer[current_node_index].local_transform_matrix; // this expects correct execution from all of the node related code above

			if (current_bone_buffer_index != LIGHTRAY_INVALID_NODE_BONE_BUFFER_INDEX)
			{
				const u32 current_bone_index = current_skeleton_bone_buffer_offset + current_bone_buffer_index;
				core->animation_core.bone_bind_pose_matrix_buffer[current_bone_index] = core->animation_core.global_root_inverse_matrix_buffer[skeleton_buffer_iter] * core->animation_core.node_buffer[current_node_index].global_transform_matrix * core->animation_core.bone_buffer[current_bone_index].inverse_bind_pose_matrix;
			}
		}
	}

	for (u32 i = 0; i < core->cpu_side_instance_count; i++)
	{
		core->cpu_side_render_instance_buffer[i].model.model = glm::mat4(1.0f);
		core->cpu_side_render_instance_buffer[i].layer_index = 0;
		core->cpu_side_render_instance_buffer[i].computed_bone_transform_matrix_buffer_offset_with_respect_to_instance = LIGHTRAY_INVALID_COMPUTED_BONE_TRANSFORM_MATRIX_BUFFER_OFFSET;
	}

	// skeleton bind pose application, render instance buffer computed_bone_transform_matrix_buffer_offset_with_respect_to_instance population
	u32 skeletal_mesh_instance_offset_iter = skeletal_mesh_instance_starting_offset;

	for (u32 sm = 0; sm < core->animation_core.total_skeleton_count; sm++)
	{
		const lightray_skeleton_t current_skeleton = core->animation_core.skeleton_buffer[sm];
		const u32 current_computed_bone_transform_matrix_buffer_offset = current_skeleton.computed_bone_transform_matrix_buffer_offset;
		const u32 current_bone_count = current_skeleton.bone_count;
		const u32 current_instance_count = current_skeleton.instance_count;
		const u32 current_bone_buffer_offset = current_skeleton.bone_buffer_offset;

		for (u32 i = 0; i < current_instance_count; i++)
		{
			const u32 current_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance = lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(i, current_bone_count, current_computed_bone_transform_matrix_buffer_offset);
			core->cpu_side_render_instance_buffer[skeletal_mesh_instance_offset_iter].computed_bone_transform_matrix_buffer_offset_with_respect_to_instance = current_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance;
			lightray_vulkan_apply_bind_pose(core, sm, i);
			skeletal_mesh_instance_offset_iter++;
		}
	}

	for (u32 i = 0; i < core->cpu_side_instance_count; i++)
	{
		SUNDER_LOG("\nlayer index:\t\t\t\t\t\t\t       ");
		SUNDER_LOG(core->cpu_side_render_instance_buffer[i].layer_index);
		SUNDER_LOG("\ncomputed_bone_transform_matrix_buffer_offset_with_respect_to_instance: ");
		SUNDER_LOG(core->cpu_side_render_instance_buffer[i].computed_bone_transform_matrix_buffer_offset_with_respect_to_instance);
		SUNDER_LOG("\n\n");
	}

	// populating mesh render pass data
	u32 index_buffer_offset_iter = 0;
	u32 instance_model_buffer_offset_iter = 0;
	u32 vertex_buffer_offset_iter = 0;

	for(u32 i = 0; i < core->total_static_mesh_count; i++)
	{
		// index buffer related
		core->mesh_render_pass_data_buffer[i].index_count = lightray_assimp_get_mesh_index_count(meshes[i]);
		core->mesh_render_pass_data_buffer[i].index_buffer_offset = index_buffer_offset_iter;
		index_buffer_offset_iter += core->mesh_render_pass_data_buffer[i].index_count;

		// instance buffer related
		core->mesh_render_pass_data_buffer[i].instance_count = initialization_data->static_mesh_metadata_buffer[i].instance_count;
		core->mesh_render_pass_data_buffer[i].instance_to_render_count = initialization_data->static_mesh_metadata_buffer[i].instance_count;
		core->mesh_render_pass_data_buffer[i].instance_buffer_offset = instance_model_buffer_offset_iter;
		instance_model_buffer_offset_iter += initialization_data->static_mesh_metadata_buffer[i].instance_count;

		// vertex buffer related
		core->mesh_render_pass_data_buffer[i].vertex_count = meshes[i]->mNumVertices;
		core->mesh_render_pass_data_buffer[i].vertex_buffer_offset = vertex_buffer_offset_iter;
		vertex_buffer_offset_iter += meshes[i]->mNumVertices;

		core->mesh_render_pass_data_buffer[i].texture_index = LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX;
	}

	skeletal_mesh_metadata_buffer_iter = 0;

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		// index buffer related
		core->mesh_render_pass_data_buffer[i].index_count = lightray_assimp_get_mesh_index_count(meshes[i]);
		core->mesh_render_pass_data_buffer[i].index_buffer_offset = index_buffer_offset_iter;
		index_buffer_offset_iter += core->mesh_render_pass_data_buffer[i].index_count;

		// instance buffer related
		core->mesh_render_pass_data_buffer[i].instance_count = initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count;
		core->mesh_render_pass_data_buffer[i].instance_to_render_count = initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count;
		core->mesh_render_pass_data_buffer[i].instance_buffer_offset = instance_model_buffer_offset_iter;
		instance_model_buffer_offset_iter += initialization_data->skeletal_mesh_metadata_buffer[skeletal_mesh_metadata_buffer_iter].instance_count;

		// vertex buffer related
		core->mesh_render_pass_data_buffer[i].vertex_count = meshes[i]->mNumVertices;
		core->mesh_render_pass_data_buffer[i].vertex_buffer_offset = vertex_buffer_offset_iter;
		vertex_buffer_offset_iter += meshes[i]->mNumVertices;

		core->mesh_render_pass_data_buffer[i].texture_index = LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX;
		skeletal_mesh_metadata_buffer_iter++;
	}

	core->overlay_vertex_subbuffer_offset = core->mesh_render_pass_data_buffer[core->total_mesh_count - 1].vertex_buffer_offset + core->mesh_render_pass_data_buffer[core->total_mesh_count - 1].vertex_count;
	core->overlay_index_subbuffer_offset = core->mesh_render_pass_data_buffer[core->total_mesh_count - 1].index_buffer_offset + core->mesh_render_pass_data_buffer[core->total_mesh_count - 1].index_count;

	for (u32 i = 0; i < core->total_static_mesh_count; i++)
	{
		core->mesh_render_pass_data_buffer[i].render_type = LIGHTRAY_VULKAN_MESH_RENDER_TYPE_STATIC;
	}

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		core->mesh_render_pass_data_buffer[i].render_type = LIGHTRAY_VULKAN_MESH_RENDER_TYPE_SKELETAL;
	};

	SUNDER_LOG("\n\n");
	lightray_vulkan_log_mesh_render_pass_data_buffer(core);

	// parsing mesh index buffers
	u32 index_buffer_iter = 0;

	for(u32 m = 0; m < core->total_mesh_count; m++)
	{
		for (u32 f = 0; f < meshes[m]->mNumFaces; f++)
		{
			for (u32 i = 0; i < meshes[m]->mFaces[f].mNumIndices; i++)
			{
				core->cpu_side_index_buffer[index_buffer_iter] = meshes[m]->mFaces[f].mIndices[i] + (u32)core->mesh_render_pass_data_buffer[m].vertex_buffer_offset;
				index_buffer_iter++;
			}
		}
	}

	// parsing mesh vertex buffers
	u32 vertex_buffer_iter = 0;

	for (u32 m = 0; m < core->total_mesh_count; m++)
	{
		for (u32 v = 0; v < meshes[m]->mNumVertices; v++)
		{
			core->cpu_side_vertex_buffer[vertex_buffer_iter].position = lightray_assimp_to_glm_vec3(meshes[m]->mVertices[v]);

			core->cpu_side_vertex_buffer[vertex_buffer_iter].uv.x = meshes[m]->mTextureCoords[0][v].x;
			core->cpu_side_vertex_buffer[vertex_buffer_iter].uv.y = meshes[m]->mTextureCoords[0][v].y;
			
			core->cpu_side_vertex_buffer[vertex_buffer_iter].normal = lightray_assimp_to_glm_vec3(meshes[m]->mNormals[v]);

			vertex_buffer_iter++;
		}
	}

	skeleton_buffer_iter = 0;

	for (u32 i = skeletal_mesh_starting_offset; i < skeletal_mesh_range; i++)
	{
		const aiMesh* current_mesh = meshes[i];

		for (u32 j = 0; j < current_mesh->mNumBones; j++)
		{
			const aiBone* current_bone = current_mesh->mBones[j];
			const u32 current_bone_index = j + core->animation_core.skeleton_buffer[skeleton_buffer_iter].bone_buffer_offset;
			u32 ct = 0;

			for (u32 k = 0; k < current_bone->mNumWeights; k++)
			{
				const u32 vertex_id = current_bone->mWeights[k].mVertexId + (u32)core->mesh_render_pass_data_buffer[i].vertex_buffer_offset;
				const f32 weight = current_bone->mWeights[k].mWeight;

				for (u32 v = 0; v < 4; v++)
				{
					const u8 current_weight_value = core->cpu_side_vertex_buffer[vertex_id].weights[v];

					if (current_weight_value == 0)
					{
						core->cpu_side_vertex_buffer[vertex_id].weights[v] = lightray_pack_f32_to_u8(weight);
						core->cpu_side_vertex_buffer[vertex_id].bone_indices[v] = current_bone_index;
						//SUNDER_LOG("\n");
						//SUNDER_LOG(vertex_id);

						//SUNDER_LOG((u16)core->cpu_side_vertex_buffer[vertex_id].weights[v]);
						//SUNDER_LOG(" | ");
						//SUNDER_LOG((u16)core->cpu_side_vertex_buffer[vertex_id].bone_indices[v]);
						//SUNDER_LOG("\n\n");

						break;
					}

					else
					{
						SUNDER_LOG("\nCURRENT WEIGHT VALUE IS NOT ZERO\n");
					}
				}
			}
		}

		skeleton_buffer_iter++;
	}

	/*core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].position.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].position.y = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].uv.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].uv.y = 1.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].position.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].position.y = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].uv.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].uv.y = 1.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].position.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].position.y = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].uv.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].uv.y = 0.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].position.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].position.y = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].uv.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].uv.y = 0.0f;*/



	/*core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].position.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].position.y = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].uv.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].uv.y = 0.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].position.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].position.y = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].uv.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].uv.y = 0.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].position.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].position.y = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].uv.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].uv.y = 1.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].position.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].position.y = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].uv.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].uv.y = 1.0f;*/



	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].position.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].position.y = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].uv.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 0].uv.y = 1.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].position.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].position.y = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].uv.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 1].uv.y = 1.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].position.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].position.y = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].uv.x = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 2].uv.y = 0.0f;

	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].position.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].position.y = 1.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].uv.x = 0.0f;
	core->cpu_side_vertex_buffer[core->overlay_vertex_subbuffer_offset + 3].uv.y = 0.0f;

	core->cpu_side_index_buffer[core->overlay_index_subbuffer_offset + 0] = SUNDER_CAST2(u32)core->overlay_vertex_subbuffer_offset + 0;
	core->cpu_side_index_buffer[core->overlay_index_subbuffer_offset + 1] = SUNDER_CAST2(u32)core->overlay_vertex_subbuffer_offset + 1;
	core->cpu_side_index_buffer[core->overlay_index_subbuffer_offset + 2] = SUNDER_CAST2(u32)core->overlay_vertex_subbuffer_offset + 2;
	core->cpu_side_index_buffer[core->overlay_index_subbuffer_offset + 3] = SUNDER_CAST2(u32)core->overlay_vertex_subbuffer_offset + 0;
	core->cpu_side_index_buffer[core->overlay_index_subbuffer_offset + 4] = SUNDER_CAST2(u32)core->overlay_vertex_subbuffer_offset + 2;
	core->cpu_side_index_buffer[core->overlay_index_subbuffer_offset + 5] = SUNDER_CAST2(u32)core->overlay_vertex_subbuffer_offset + 3;


	const u64 bytes_to_write_vertex_buffer = sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count);
	const u64 bytes_to_write_index_buffer = sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count);
	const u64 bytes_to_write_instance_model_buffer = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), total_instance_model_count);

	sunder_buffer_copy_data_t buffer_copy_data{};
	buffer_copy_data.dst_size = core->host_visible_vram_arena.capacity;
	buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_vertex_buffer_starting_index];
	buffer_copy_data.src_size = bytes_to_write_vertex_buffer;
	buffer_copy_data.src_offset = 0;
	buffer_copy_data.bytes_to_write = bytes_to_write_vertex_buffer;

	const u64 vertex_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->cpu_side_vertex_buffer, &buffer_copy_data); // vertex buffer

	buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_index_buffer_starting_index];
	buffer_copy_data.src_size = bytes_to_write_index_buffer;
	buffer_copy_data.bytes_to_write = bytes_to_write_index_buffer;

	const u64 index_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->cpu_side_index_buffer, &buffer_copy_data); // index buffer

	buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_buffer_starting_index];
	buffer_copy_data.src_size = bytes_to_write_instance_model_buffer;
	buffer_copy_data.bytes_to_write = bytes_to_write_instance_model_buffer;

	const u64 instance_model_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->cpu_side_render_instance_buffer, &buffer_copy_data); // instance buffer;

	sunder_buffer_copy_data_t computed_bone_transform_matrix_buffer_copy_data{};
	computed_bone_transform_matrix_buffer_copy_data.dst_size = core->host_visible_storage_vram_arena.capacity;
	computed_bone_transform_matrix_buffer_copy_data.dst_offset = gpu_side_bone_transform_matrix_buffer_suballocation_result.starting_offset;
	computed_bone_transform_matrix_buffer_copy_data.bytes_to_write = sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count);
	computed_bone_transform_matrix_buffer_copy_data.src_size = sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count);
	computed_bone_transform_matrix_buffer_copy_data.src_offset = 0;

	const u64 computed_bone_transform_matrix_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_storage_vram_arena_view, core->animation_core.computed_bone_matrix_buffer, &computed_bone_transform_matrix_buffer_copy_data);

	lightray_vulkan_copy_buffer(core, core->device_local_vram_arena.buffer, core->host_visible_vram_arena.buffer, core->device_local_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_VERTEX_BUFFER_INDEX], core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_vertex_buffer_starting_index], sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count)); // vertex buffer
	lightray_vulkan_copy_buffer(core, core->device_local_vram_arena.buffer, core->host_visible_vram_arena.buffer, core->device_local_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_INDEX_BUFFER_INDEX], core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_index_buffer_starting_index], sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count)); // index buffer
	
	for (u32 i = 0; i < core->total_mesh_count + core->animation_core.total_animation_count; i++)
	{
		importers[i].~Importer();
	}

	/////////////////////////// setup core end ///////////////////////////
}

void lightray_vulkan_terminate_core(lightray_vulkan_core_t* core)
{
	vkDeviceWaitIdle(core->logical_device);

	const u8 texture_buffer_texture_free_mask = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_IMAGE_BIT, LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_VIEW_BIT }, 2);

	for (u32 i = 0; i < core->total_texture_count_including_depth_texture; i++)
	{
		const lightray_vulkan_result texture_ram_buffer_free_result = lightray_vulkan_free_texture(core, &core->texture_buffer[i], texture_buffer_texture_free_mask);
	}

	for (u32 i = 0; i < core->total_vram_texture_arena_count; i++)
	{
		lightray_vulkan_free_vram_texture_arena(core, &core->vram_texture_arena_buffer[i]);
	}

	vkUnmapMemory(core->logical_device, core->host_visible_storage_vram_arena.device_memory);
	vkUnmapMemory(core->logical_device, core->host_visible_vram_arena.device_memory);

	lightray_vulkan_free_vram_arena(core, &core->host_visible_vram_arena);
	lightray_vulkan_free_vram_arena(core, &core->device_local_vram_arena);
	lightray_vulkan_free_vram_arena(core, &core->host_visible_storage_vram_arena);

	for (u32 i = 0; i < core->total_sampler_count; i++)
	{
		vkDestroySampler(core->logical_device, core->sampler_buffer[i], nullptr);
	}

	core->swapchain_image_in_use_count = lightray_vulkan_get_supported_swapchain_image_count(&core->surface_capabilities);

	for (u32 i = 0; i < core->swapchain_image_in_use_count; i++)
	{
		vkDestroySemaphore(core->logical_device, core->image_available_for_rendering_semaphores[i], nullptr);
		vkDestroySemaphore(core->logical_device, core->image_finished_rendering_semaphores[i], nullptr);
		vkDestroyFence(core->logical_device, core->inflight_fences[i], nullptr);
	}

	vkFreeCommandBuffers(core->logical_device, core->command_pool, core->swapchain_image_in_use_count, core->render_command_buffers);
	vkFreeCommandBuffers(core->logical_device, core->command_pool, 1, &core->general_purpose_command_buffer);

	vkDestroyCommandPool(core->logical_device, core->command_pool, nullptr);

	vkDestroyDescriptorPool(core->logical_device, core->descriptor_pool, nullptr);
	vkDestroyDescriptorSetLayout(core->logical_device, core->main_descriptor_set_layout, nullptr);
	vkDestroyRenderPass(core->logical_device, core->render_pass, nullptr);

	// unrolled fixed size loops for faster application termination
	vkDestroyPipelineLayout(core->logical_device, core->pipeline_layouts[0], nullptr);

	lightray_vulkan_free_shader(core, &core->shaders[0]);
	lightray_vulkan_free_shader(core, &core->shaders[1]);
	lightray_vulkan_free_shader(core, &core->shaders[2]);
	lightray_vulkan_free_shader(core, &core->shaders[3]);
	lightray_vulkan_free_shader(core, &core->shaders[4]);
	lightray_vulkan_free_shader(core, &core->shaders[5]);
	lightray_vulkan_free_shader(core, &core->shaders[6]);
	lightray_vulkan_free_shader(core, &core->shaders[7]);

	vkDestroyPipeline(core->logical_device, core->pipelines[0], nullptr);
	vkDestroyPipeline(core->logical_device, core->pipelines[1], nullptr);
	vkDestroyPipeline(core->logical_device, core->pipelines[2], nullptr);
	vkDestroyPipeline(core->logical_device, core->pipelines[3], nullptr);
	vkDestroyPipeline(core->logical_device, core->pipelines[4], nullptr);

	lightray_vulkan_free_swapchain(core);
	lightray_vulkan_free_swapchain_framebuffers(core);
	vkDestroySurfaceKHR(core->instance, core->surface, nullptr);
	vkDestroyDevice(core->logical_device, nullptr);
	vkDestroyInstance(core->instance, nullptr);

	glfwDestroyWindow(core->window);
	glfwTerminate();
	sunder_free_arena(&core->general_purpose_ram_arena);
}

u32 lightray_vulkan_get_supported_swapchain_image_count(const VkSurfaceCapabilitiesKHR* surface_capabilities)
{
	const u32 max_supported_swapchain_image_count = surface_capabilities->maxImageCount;
	u32 supported_swapchain_image_count = 0;

	if (max_supported_swapchain_image_count == 0)
	{
		supported_swapchain_image_count = 3;
	}

	else if (max_supported_swapchain_image_count > 2)
	{
		supported_swapchain_image_count = 3;
	}

	else if (max_supported_swapchain_image_count == 2)
	{
		supported_swapchain_image_count = 2;
	}

	else
	{
		supported_swapchain_image_count = 1;
	}

	return supported_swapchain_image_count;
}

bool lightray_vulkan_is_fps_capped(const lightray_vulkan_core_t* core)
{
	return SUNDER_IS_ANY_BIT_SET(core->flags, LIGHTRAY_BITS_FPS_CAPPED_BIT, 1U);
}

void lightray_vulkan_set_target_fps(lightray_vulkan_core_t* core, f32 desired_fps)
{
	if (!lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_SET_BIT(core->flags, LIGHTRAY_BITS_FPS_CAPPED_BIT, 1U);
	}

	lightray_set_target_fps(desired_fps, &core->frame_duration_s, &core->fps);
}

void lightray_vulkan_uncap_fps(lightray_vulkan_core_t* core)
{
	if (lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_ZERO_BIT(core->flags, LIGHTRAY_BITS_FPS_CAPPED_BIT, 1U);
	}
}

u64 lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(u32 suballocation_count, u64 alignment)
{
	return sunder_compute_aligned_allocation_size(sizeof(u64), suballocation_count, alignment);
}

u32 lightray_vulkan_get_graphics_queue_family_index(const VkQueueFamilyProperties* supported_queue_family_properties, const u32* swapchain_presentation_supported_queue_family_indices, u32 swapchain_presentation_supported_queue_family_index_count)
{
	for (u32 i = 0; i < swapchain_presentation_supported_queue_family_index_count; i++)
	{
		if (supported_queue_family_properties[swapchain_presentation_supported_queue_family_indices[i]].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			return swapchain_presentation_supported_queue_family_indices[i];
		}
	}

	return UINT32_MAX;
}

u32 lightray_vulkan_get_present_queue_family_index(const VkQueueFamilyProperties* supported_queue_family_properties, const u32* swapchain_presentation_supported_queue_family_indices, u32 swapchain_presentation_supported_queue_family_index_count)
{
	const u32 graphics_queue_index = lightray_vulkan_get_graphics_queue_family_index(supported_queue_family_properties, swapchain_presentation_supported_queue_family_indices, swapchain_presentation_supported_queue_family_index_count);

	for (u32 i = 0; i < swapchain_presentation_supported_queue_family_index_count; i++)
	{
		if (swapchain_presentation_supported_queue_family_indices[i] != graphics_queue_index)
		{
			return swapchain_presentation_supported_queue_family_indices[i];
		}
	}

	return UINT32_MAX;
}

u32 lightray_vulkan_get_vram_type_index(const lightray_vulkan_core_t* core, u32 vram_filter, VkMemoryPropertyFlags vram_property_flags)
{
	const u32 vram_type_count = core->gpu_vram_properties.memoryTypeCount;

	for (u32 i = 0; i < vram_type_count; i++)
	{
		if (SUNDER_IS_ANY_BIT_SET(vram_filter, i, 1U) && (core->gpu_vram_properties.memoryTypes[i].propertyFlags & vram_property_flags) == vram_property_flags)
		{
			return i;
		}
	}

	return UINT32_MAX;;
}

lightray_vulkan_result lightray_vulkan_allocate_vram_arena_debug(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena, const lightray_vulkan_vram_arena_allocation_data_t* allocation_data)
{
	if (allocation_data->suballocation_count == 0) { LIGHTRAY_VULKAN_RESULT_SUBALLOCATION_COUNT_IS_0; }
	if (allocation_data->metadata_allocation_size != sunder_compute_array_size_in_bytes(sizeof(u64), allocation_data->suballocation_count)) { LIGHTRAY_VULKAN_RESULT_INVALID_METADATA_ALLOCATION_SIZE; }
	if (allocation_data->allocation_size == 0) { LIGHTRAY_VULKAN_RESULT_ARENA_ALLOCATION_SIZE_IS_0; }

	const sunder_arena_suballocation_result_t suballocation_offset_buffer_suballocation_result = sunder_suballocate_from_arena_debug(allocation_data->metadata_arena, allocation_data->metadata_allocation_size, 8);
	arena->suballocation_offset_buffer = (u64*)suballocation_offset_buffer_suballocation_result.data;

	VkBufferCreateInfo buffer_create_info{};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = allocation_data->allocation_size;
	buffer_create_info.usage = allocation_data->usage_flags;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	buffer_create_info.flags = 0;
	buffer_create_info.pQueueFamilyIndices = nullptr;
	buffer_create_info.queueFamilyIndexCount = 0;
	buffer_create_info.pNext = nullptr;

	const VkResult buffer_creation_result = vkCreateBuffer(core->logical_device, &buffer_create_info, nullptr, &arena->buffer);
	if (buffer_creation_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_RESULT_FAILED_TO_CREATE_BUFFER; }

	VkMemoryRequirements vram_requirements{};
	vkGetBufferMemoryRequirements(core->logical_device, arena->buffer, &vram_requirements);

	VkMemoryAllocateInfo device_memory_allocation_info{};
	device_memory_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	device_memory_allocation_info.allocationSize = vram_requirements.size;
	device_memory_allocation_info.memoryTypeIndex = lightray_vulkan_get_vram_type_index(core, vram_requirements.memoryTypeBits, allocation_data->vram_properties);
	device_memory_allocation_info.pNext = nullptr;

	const VkResult device_memory_allocation_result = vkAllocateMemory(core->logical_device, &device_memory_allocation_info, nullptr, &arena->device_memory);
	if (device_memory_allocation_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_RESULT_OUT_OF_VRAM; }

	const VkResult buffer_binding_result = vkBindBufferMemory(core->logical_device, arena->buffer, arena->device_memory, 0);
	if (buffer_binding_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_RESULT_FAILED_TO_BIND_BUFFER; }

	SUNDER_LOG("\n[vram arena allocation] allocated size: ");
	SUNDER_LOG(device_memory_allocation_info.allocationSize);
	SUNDER_LOG("\n[vram arena allocation] required alignment: ");
	SUNDER_LOG(vram_requirements.alignment);
	SUNDER_LOG("\n[vram arena allocation] required vram type index: ");
	SUNDER_LOG(device_memory_allocation_info.memoryTypeIndex);
	SUNDER_LOG("\n[vram arena allocation] max suballocation count: ");
	SUNDER_LOG(allocation_data->suballocation_count);
	SUNDER_LOG("\n\n[vram arena allocation] vram properties:\n");

	if ((allocation_data->vram_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
	{
		SUNDER_LOG("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | ");
	}

	if ((allocation_data->vram_properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
	{
		SUNDER_LOG("VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | ");
	}

	if ((allocation_data->vram_properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
	{
		SUNDER_LOG("VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | ");
	}

	SUNDER_LOG("\n\n[vram arena allocation] vram arena usage:\n");

	if ((allocation_data->usage_flags & VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
	{
		SUNDER_LOG("VK_BUFFER_USAGE_INDEX_BUFFER_BIT | ");
	}

	if ((allocation_data->usage_flags & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT))
	{
		SUNDER_LOG("VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | ");
	}

	if ((allocation_data->usage_flags & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT))
	{
		SUNDER_LOG("VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | ");
	}

	if ((allocation_data->usage_flags & VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
	{
		SUNDER_LOG("VK_BUFFER_USAGE_TRANSFER_SRC_BIT | ");
	}

	if ((allocation_data->usage_flags & VK_BUFFER_USAGE_TRANSFER_DST_BIT))
	{
		SUNDER_LOG("VK_BUFFER_USAGE_TRANSFER_DST_BIT | ");
	}

	if ((allocation_data->usage_flags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT))
	{
		SUNDER_LOG("VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | ");
	}

	SUNDER_LOG("\n");
		
	arena->vram_type_index = device_memory_allocation_info.memoryTypeIndex;
	arena->alignment = SUNDER_CAST(u32, vram_requirements.alignment);
	arena->capacity = device_memory_allocation_info.allocationSize;
	arena->current_offset = 0;
	arena->max_suballocation_count = allocation_data->suballocation_count;
	arena->suballocation_total_count = 0;
	arena->suballocation_total_size_in_bytes = 0;
	arena->vram_property_flags = allocation_data->vram_properties;

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

lightray_vulkan_vram_arena_suballocation_result_t lightray_vulkan_suballocate_from_vram_arena_debug(lightray_vulkan_vram_arena_t* arena, u64 suballocation_size)
{
	lightray_vulkan_vram_arena_suballocation_result_t res;
	res.result = LIGHTRAY_VULKAN_RESULT_FAILURE;
	res.starting_offset = UINT64_MAX;
	
	res.result = LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_DEVICE_MEMORY;
	if (arena->device_memory == nullptr) { return res; }
	res.result = LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_BUFFER;
	if (arena->buffer == nullptr) { return res; }

	const u64 aligned_offset = sunder_align64(arena->current_offset, arena->alignment);
	const u64 bytes_of_padding = aligned_offset - arena->current_offset;
	res.result = LIGHTRAY_VULKAN_RESULT_OUT_OF_VRAM;

	if (aligned_offset > arena->capacity) { return res; }
	if (aligned_offset + suballocation_size > arena->capacity) { return res; }

	SUNDER_LOG("\n[vram arena suballocation] suballocation offset buffer:\n");
	for (u32 i = 0; i < arena->suballocation_total_count; i++)
	{
		SUNDER_LOG(arena->suballocation_offset_buffer[i]);
		SUNDER_LOG(" ");
	}

	SUNDER_LOG("\n[vram arena suballocation] ");
	SUNDER_LOG(arena->current_offset);
	SUNDER_LOG("\\");
	SUNDER_LOG(arena->capacity);
	SUNDER_LOG("\n[vram arena suballocation] alignment: ");
	SUNDER_LOG(arena->alignment);
	SUNDER_LOG("\n[vram arena suballocation] requested suballocation size: ");
	SUNDER_LOG(suballocation_size);
	SUNDER_LOG("\n[vram arena suballocation] current offset: ");
	SUNDER_LOG(arena->current_offset);

	SUNDER_LOG("\n[vram arena suballocation] aligned offset: ");
	SUNDER_LOG(aligned_offset);

	SUNDER_LOG("\n[vram arena suballocation] bytes of padding added: ");
	SUNDER_LOG(bytes_of_padding);
	
	res.result = LIGHTRAY_VULKAN_RESULT_SUCCESS;
	res.starting_offset = aligned_offset;

	SUNDER_LOG("\n[vram arena suballocation] suballocated at offset : ");
	SUNDER_LOG(aligned_offset);

	const u64 post_suballocation_offset = aligned_offset + suballocation_size;
	SUNDER_LOG("\n[vram arena suballocation] post suballocation offset: ");
	SUNDER_LOG(post_suballocation_offset);

	arena->current_offset = post_suballocation_offset;
	arena->suballocation_total_size_in_bytes += bytes_of_padding;
	arena->suballocation_offset_buffer[arena->suballocation_total_count] = aligned_offset;
	arena->suballocation_total_count++;
	
	SUNDER_LOG("\n");

	return res;
}

lightray_vulkan_result lightray_vulkan_free_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena)
{
	if (arena->device_memory == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_DEVICE_MEMORY; }
	if (arena->buffer == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_BUFFER; }

	vkDestroyBuffer(core->logical_device, arena->buffer, nullptr);
	vkFreeMemory(core->logical_device, arena->device_memory, nullptr);

	arena->max_suballocation_count = 0;
	arena->suballocation_total_count = 0;
	arena->suballocation_total_size_in_bytes = 0;
	arena->alignment = 0;
	arena->capacity = 0;
	arena->current_offset = 0;
	arena->vram_property_flags = 0;
	arena->vram_type_index = 0;

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

void lightray_vulkan_create_swapchain(lightray_vulkan_core_t* core)
{
	core->swapchain_image_in_use_count = lightray_vulkan_get_supported_swapchain_image_count(&core->surface_capabilities);

	core->swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	core->swapchain_info.surface = core->surface;
	core->swapchain_info.minImageCount = core->swapchain_image_in_use_count;
	core->swapchain_info.imageFormat = core->chosen_surface_format.format;
	core->swapchain_info.imageColorSpace = core->chosen_surface_format.colorSpace;
	core->swapchain_info.imageExtent = core->surface_capabilities.currentExtent;
	core->swapchain_info.imageArrayLayers = 1;
	core->swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	core->swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	core->swapchain_info.queueFamilyIndexCount = core->swapchain_presentation_supported_queue_family_index_count;
	core->swapchain_info.pQueueFamilyIndices = core->swapchain_presentation_supported_queue_family_indices;
	core->swapchain_info.preTransform = core->surface_capabilities.currentTransform;
	core->swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	core->swapchain_info.presentMode = core->chosen_swapchain_present_mode;
	core->swapchain_info.clipped = VK_TRUE;
	core->swapchain_info.oldSwapchain = nullptr;
	core->swapchain_info.flags = 0;
	core->swapchain_info.pNext = nullptr;

	const VkResult swapchain_creation_result = vkCreateSwapchainKHR(core->logical_device, &core->swapchain_info, nullptr, &core->swapchain);
	SUNDER_LOG("\nswapchain_creation_result: ");
	SUNDER_LOG(swapchain_creation_result);

	const VkResult swapchain_images_retrieaval_result = vkGetSwapchainImagesKHR(core->logical_device, core->swapchain, &core->swapchain_image_in_use_count, core->swapchain_images);
	SUNDER_LOG("\nswapchain_images_retrieaval_result: ");
	SUNDER_LOG(swapchain_images_retrieaval_result);

	for (u32 i = 0; i < core->swapchain_image_in_use_count; i++)
	{
		core->swapchain_image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		core->swapchain_image_view_info.image = core->swapchain_images[i];
		core->swapchain_image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		core->swapchain_image_view_info.format = core->chosen_surface_format.format;
		core->swapchain_image_view_info.pNext = nullptr;
		core->swapchain_image_view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		core->swapchain_image_view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		core->swapchain_image_view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		core->swapchain_image_view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		core->swapchain_image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		core->swapchain_image_view_info.subresourceRange.baseMipLevel = 0;
		core->swapchain_image_view_info.subresourceRange.levelCount = 1;
		core->swapchain_image_view_info.subresourceRange.baseArrayLayer = 0;
		core->swapchain_image_view_info.subresourceRange.layerCount = 1;
		core->swapchain_image_view_info.flags = 0;

		const VkResult image_view_creation_result = vkCreateImageView(core->logical_device, &core->swapchain_image_view_info, nullptr, &core->swapchain_image_views[i]);
		SUNDER_LOG("\nimage_view_creation_result  at ");
		SUNDER_LOG(i);
		SUNDER_LOG(" is ");
		SUNDER_LOG(image_view_creation_result);
	}
}

void lightray_vulkan_free_swapchain(lightray_vulkan_core_t* core)
{
	core->swapchain_image_in_use_count = lightray_vulkan_get_supported_swapchain_image_count(&core->surface_capabilities);

	for (u32 i = 0; i < core->swapchain_image_in_use_count; i++)
	{
		vkDestroyImageView(core->logical_device, core->swapchain_image_views[i], nullptr);
	}

	vkDestroySwapchainKHR(core->logical_device, core->swapchain, nullptr);
}

lightray_vulkan_result lightray_vulkan_create_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader, VkShaderStageFlagBits stage_flags)
{
	if (stage_flags == 0) { return LIGHTRAY_VULKAN_RESULT_INVALID_SHADER_STAGE_FLAGS; }
	if (shader->byte_code == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_BYTE_CODE_BUFFER; }
	if (shader->byte_code_size == 0) { return LIGHTRAY_VULKAN_RESULT_INVALID_BYTE_CODE_SIZE; }

	shader->stage = stage_flags;

	VkShaderModuleCreateInfo module_info{};
	module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_info.codeSize = shader->byte_code_size;
	module_info.pCode = (const u32*)shader->byte_code;
	module_info.flags = 0;
	module_info.pNext = nullptr;

	const VkResult module_creation_result = vkCreateShaderModule(core->logical_device, &module_info, nullptr, &shader->module);

	shader->stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader->stage_info.module = shader->module;
	shader->stage_info.stage = stage_flags;
	shader->stage_info.pName = "main";
	shader->stage_info.flags = 0;
	shader->stage_info.pSpecializationInfo = nullptr;
	shader->stage_info.pNext = nullptr;

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

lightray_vulkan_result lightray_vulkan_free_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader)
{
	if (shader->module == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_SHADER_MODULE; }

	vkDestroyShaderModule(core->logical_device, shader->module, nullptr);
	shader->byte_code_size = 0;

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

u64 lightray_vulkan_compute_required_user_chosen_arena_suballocation_size(u32 mesh_count, u32 texture_count, u32 msdf_font_atlas_count, u32 animation_count, u32 camera_count, u32 alignment)
{
	const u32 texture_count_local = msdf_font_atlas_count + texture_count;
	const u32 texture_count_local_wrt_depth_texture = texture_count_local + 1;
	const u64 importers_allocation_size = sunder_compute_aligned_allocation_size(8, mesh_count + animation_count, alignment);
	const u64 scenes_allocation_size = sunder_compute_aligned_allocation_size(8, mesh_count + animation_count, alignment);
	const u64 meshes_allocation_size = sunder_compute_aligned_allocation_size(8, mesh_count, alignment);
	const u64 texture_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vulkan_texture_t), texture_count_local_wrt_depth_texture, alignment); // + 1 for depth image
	const u64 texture_vram_requirements_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkMemoryRequirements), texture_count_local_wrt_depth_texture, alignment); // + 1 for depth image
	const u64 vram_type_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), texture_count_local_wrt_depth_texture, alignment); // + 1 for depth image
	const u64 vram_type_index_filter_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), texture_count_local_wrt_depth_texture, alignment); // + 1 for depth image
	const u64 vram_texture_arena_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vulkan_vram_texture_arena_t), texture_count_local_wrt_depth_texture, alignment); // + 1 for depth image
	const u64 sampler_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkSampler), texture_count_local, alignment);
	const u64 texture_filtering_filter_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkFilter), texture_count_local, alignment);
	const u64 descriptor_combined_sampler_info_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorImageInfo), texture_count_local, alignment);
	const u64 descriptor_set_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorSet), texture_count_local, alignment);
	const u64 copy_descriptor_set_layout_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorSetLayout), texture_count_local, alignment);
	const u64 host_visible_vram_arena_suballocation_starting_offsets_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), 3u + texture_count_local + camera_count, alignment);
	const u64 mesh_render_pass_data_reordering_helper_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), mesh_count, alignment);
	const u64 aligned_allocation_size_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), LIGHTRAY_VULKAN_ALIGNED_ALLOCATION_SIZE_BUFFER_LENGTH, alignment);

	return sunder_align64(texture_buffer_allocation_size + importers_allocation_size + scenes_allocation_size + meshes_allocation_size + texture_vram_requirements_buffer_allocation_size + vram_type_index_buffer_allocation_size + vram_type_index_filter_buffer_allocation_size + vram_texture_arena_buffer_allocation_size + sampler_buffer_allocation_size + texture_filtering_filter_buffer_allocation_size + descriptor_combined_sampler_info_buffer_allocation_size + descriptor_set_buffer_allocation_size + copy_descriptor_set_layout_buffer_allocation_size + host_visible_vram_arena_suballocation_starting_offsets_allocation_size + mesh_render_pass_data_reordering_helper_buffer_allocation_size + aligned_allocation_size_buffer_allocation_size, alignment);
}

void lightray_vulkan_set_relative_path(lightray_vulkan_core_t* core, cstring_literal* path)
{
	core->relative_path = path;
}

lightray_vulkan_result lightray_vulkan_load_texture(cstring_literal* path, lightray_vulkan_texture_t* texture)
{
	texture->buffer = stbi_load(path, &texture->width, &texture->height, nullptr, STBI_rgb_alpha);
	texture->size = ((u64)(texture->width * texture->height)) * ((u64)STBI_rgb_alpha);

	if (texture->buffer == NULL) { return LIGHTRAY_VULKAN_RESULT_FAILED_TO_LOAD_IMAGE; }
	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

lightray_vulkan_result lightray_vulkan_create_texture(lightray_vulkan_core_t* core, lightray_vulkan_texture_t* texture, const lightray_vulkan_texture_creation_data_t* creation_data)
{
	if (creation_data->creation_flags == 0) { return LIGHTRAY_VULKAN_RESULT_INVALID_FLAGS; }

	const u8 mask = sunder_to_bit_mask8({LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_IMAGE_BIT, LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_VIEW_BIT}, 2);

	if (SUNDER_IS_SINGLE_BIT_SET(creation_data->creation_flags, LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_IMAGE_BIT, 1))
	{
		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = creation_data->type;
		image_info.extent.width = creation_data->width;
		image_info.extent.height = creation_data->height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = creation_data->layer_count;
		image_info.format = creation_data->format;
		image_info.tiling = creation_data->tiling;
		image_info.initialLayout = texture->layout;
		image_info.usage = creation_data->usage_flags;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.flags = 0;
		image_info.pNext = nullptr;
		image_info.pQueueFamilyIndices = nullptr;
		image_info.queueFamilyIndexCount = 0;

		const VkResult image_creation_result = vkCreateImage(core->logical_device, &image_info, nullptr, &texture->image);

		texture->aspect_flags = creation_data->aspect_flags;
		texture->format = creation_data->format;
		texture->usage_flags = creation_data->usage_flags;
		texture->width = creation_data->width;
		texture->height = creation_data->height;
		texture->filter = creation_data->filter;
		texture->layer_count = creation_data->layer_count;
		texture->type = creation_data->type;
		texture->kind = creation_data->kind;

		return LIGHTRAY_VULKAN_RESULT_SUCCESS;
	}

	else if (SUNDER_IS_SINGLE_BIT_SET(creation_data->creation_flags, LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_VIEW_BIT, 1))
	{
		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.viewType = creation_data->view_type;
		view_info.image = texture->image;
		view_info.format = texture->format;;
		view_info.subresourceRange.aspectMask = texture->aspect_flags;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = texture->layer_count;
		view_info.pNext = nullptr;
		view_info.flags = 0;

		const VkResult view_creation_result = vkCreateImageView(core->logical_device, &view_info, nullptr, &texture->view);

		texture->view_type = creation_data->view_type;

		return LIGHTRAY_VULKAN_RESULT_SUCCESS;
	}

	else
	{
		return LIGHTRAY_VULKAN_RESULT_INVALID_FLAGS;
	}
}

void lightray_vulkan_transition_texture_layout(VkQueue queue, VkCommandBuffer command_buffer, const VkCommandBufferBeginInfo* command_buffer_begin_info, lightray_vulkan_texture_t* texture, VkImageLayout new_layout)
{
	VkImageMemoryBarrier image_vram_barrier{};
	VkPipelineStageFlags pipeline_src_stage_flags = 0;
	VkPipelineStageFlags pipeline_dst_stage_flags = 0;

	if (texture->layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		image_vram_barrier.srcAccessMask = 0;
		image_vram_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		pipeline_src_stage_flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		pipeline_dst_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}

	else if (texture->layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		image_vram_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_vram_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		pipeline_src_stage_flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
		pipeline_dst_stage_flags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	
	image_vram_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_vram_barrier.image = texture->image;
	image_vram_barrier.oldLayout = texture->layout;
	image_vram_barrier.newLayout = new_layout;
	image_vram_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_vram_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	image_vram_barrier.subresourceRange.aspectMask = texture->aspect_flags;
	image_vram_barrier.subresourceRange.baseMipLevel = 0;
	image_vram_barrier.subresourceRange.levelCount = 1;
	image_vram_barrier.subresourceRange.baseArrayLayer = 0;
	image_vram_barrier.subresourceRange.layerCount = texture->layer_count;
	image_vram_barrier.pNext = nullptr;

	lightray_vulkan_begin_command_buffer_recording(command_buffer, command_buffer_begin_info);

	vkCmdPipelineBarrier(command_buffer, pipeline_src_stage_flags, pipeline_dst_stage_flags, 0, 0, nullptr, 0, nullptr, 1, &image_vram_barrier);

	lightray_vulkan_end_command_buffer_recording(command_buffer, queue);

	texture->layout = new_layout;
}

void lightray_vulkan_populate_texture(lightray_vulkan_texture_t* texture, const lightray_vulkan_texture_population_data_t* population_data)
{
	VkBufferImageCopy buffer_image_copy{};
	buffer_image_copy.bufferOffset = population_data->buffer_offset;
	buffer_image_copy.bufferRowLength = 0;
	buffer_image_copy.bufferImageHeight = 0;

	buffer_image_copy.imageOffset = {0, 0, 0};
	buffer_image_copy.imageExtent = {(u32)texture->width, (u32)texture->height, 1};

	buffer_image_copy.imageSubresource.aspectMask = texture->aspect_flags;
	buffer_image_copy.imageSubresource.mipLevel = 0;
	buffer_image_copy.imageSubresource.baseArrayLayer = population_data->starting_layer;
	buffer_image_copy.imageSubresource.layerCount = population_data->layer_count;

	lightray_vulkan_begin_command_buffer_recording(population_data->command_buffer, population_data->command_buffer_begin_info);

	vkCmdCopyBufferToImage(population_data->command_buffer, population_data->buffer, texture->image, texture->layout, 1, &buffer_image_copy);

	lightray_vulkan_end_command_buffer_recording(population_data->command_buffer, population_data->queue);
}

lightray_vulkan_result lightray_vulkan_free_texture(lightray_vulkan_core_t* core, lightray_vulkan_texture_t* texture, u8 free_flags)
{	
	if (free_flags == 0) { return LIGHTRAY_VULKAN_RESULT_INVALID_FLAGS; }

	const u8 mask = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_RAM_BUFFER_BIT, LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_IMAGE_BIT,  LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_VIEW_BIT }, 3);
	const u8 mask2 = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_IMAGE_BIT,  LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_VIEW_BIT }, 2);

	if (SUNDER_ARE_BITS_SET(free_flags, mask))
	{
		if (texture->buffer == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_TEXTURE_BUFFER; }

		stbi_image_free(texture->buffer);
		vkDestroyImageView(core->logical_device, texture->view, nullptr);
		vkDestroyImage(core->logical_device, texture->image, nullptr);

		return LIGHTRAY_VULKAN_RESULT_SUCCESS;
	}

	else if (SUNDER_IS_SINGLE_BIT_SET(free_flags, LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_RAM_BUFFER_BIT, 1))
	{
		if (texture->buffer == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_TEXTURE_BUFFER; }

		stbi_image_free(texture->buffer);

		return LIGHTRAY_VULKAN_RESULT_SUCCESS;
	}

	else if (SUNDER_IS_SINGLE_BIT_SET(free_flags, LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_IMAGE_BIT, 1))
	{
		if (texture->image == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_IMAGE; }

		vkDestroyImage(core->logical_device, texture->image, nullptr);

		return LIGHTRAY_VULKAN_RESULT_SUCCESS;
	}

	else if (SUNDER_ARE_BITS_SET(free_flags, mask2))
	{
		if (texture->image == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_IMAGE; }
		if (texture->view == nullptr) { return LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_VIEW; }

		vkDestroyImageView(core->logical_device, texture->view, nullptr);
		vkDestroyImage(core->logical_device, texture->image, nullptr);

		return LIGHTRAY_VULKAN_RESULT_SUCCESS;
	}

	else return LIGHTRAY_VULKAN_RESULT_INVALID_FLAGS;
}

lightray_vulkan_result lightray_vulkan_allocate_vram_texture_arena_debug(lightray_vulkan_core_t* core, lightray_vulkan_vram_texture_arena_t* arena, u64 allocation_size, u32 alignment, u32 vram_type_index)
{
	VkMemoryAllocateInfo vram_allocate_info{};
	vram_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vram_allocate_info.allocationSize = allocation_size;
	vram_allocate_info.memoryTypeIndex = vram_type_index;
	vram_allocate_info.pNext = nullptr;

	const VkResult device_memory_allocation_result = vkAllocateMemory(core->logical_device, &vram_allocate_info, nullptr, &arena->device_memory);
	if (device_memory_allocation_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_RESULT_OUT_OF_VRAM; }

	arena->alignment = alignment;
	arena->capacity = allocation_size;
	arena->vram_type_index = vram_type_index;

	SUNDER_LOG("\n[vram texture arena allocation] required alignment:  ");
	SUNDER_LOG(arena->alignment);
	SUNDER_LOG("\n[vram texture arena allocation] required capacity: ");
	SUNDER_LOG(arena->capacity);
	SUNDER_LOG("\n[vram texture arena allocation] vram type index: ");
	SUNDER_LOG(arena->vram_type_index);

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

lightray_vulkan_result lightray_vulkan_suballocate_texture(lightray_vulkan_core_t* core, lightray_vulkan_vram_texture_arena_t* arena, lightray_vulkan_texture_t* texture, u64 texture_size)
{
	const u64 aligned_offset = sunder_align64(arena->current_offset, arena->alignment);
	const u64 bytes_of_padding = aligned_offset - arena->current_offset;
	const u64 new_offset = aligned_offset + texture_size;

	if (new_offset > arena->capacity)
	{
		return LIGHTRAY_VULKAN_RESULT_OUT_OF_VRAM;
	}

	SUNDER_LOG("\n\n");
	SUNDER_LOG(arena->current_offset);
	SUNDER_LOG("/");
	SUNDER_LOG(arena->capacity);
	SUNDER_LOG("\n[vram texture suballaction] required alignment: ");
	SUNDER_LOG(arena->alignment);
	SUNDER_LOG("\n[vram texture suballaction] requested suballocation size: ");
	SUNDER_LOG(texture_size);
	SUNDER_LOG("\n[vram texture suballaction] current offset: ");
	SUNDER_LOG(arena->current_offset);
	SUNDER_LOG("\n[vram texture suballaction] aligned offset: ");
	SUNDER_LOG(aligned_offset);
	SUNDER_LOG("\n[vram texture suballaction] bytes of padding added: ");
	SUNDER_LOG(bytes_of_padding);
	SUNDER_LOG("\n[vram texture suballaction] suballocated at offset: ");
	SUNDER_LOG(aligned_offset);
	SUNDER_LOG("\n[vram texture suballaction] post suballocation offset: ");
	SUNDER_LOG(new_offset);

	const VkResult image_binding_result = vkBindImageMemory(core->logical_device, texture->image, arena->device_memory, aligned_offset);
	if (image_binding_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_RESULT_FAILED_TO_BIND_IMAGE; }

	arena->current_offset = new_offset;
	arena->suballocation_total_count++;
	arena->suballocation_total_size_in_bytes += texture_size + bytes_of_padding;

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

lightray_vulkan_result lightray_vulkan_free_vram_texture_arena(lightray_vulkan_core_t* core, lightray_vulkan_vram_texture_arena_t* arena)
{
	vkFreeMemory(core->logical_device, arena->device_memory, nullptr);

	arena->alignment = 0;
	arena->capacity = 0;
	arena->current_offset = 0;
	arena->vram_type_index = 0;
	arena->suballocation_total_count = 0;
	arena->suballocation_total_size_in_bytes = 0;

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

lightray_vulkan_result lightray_vulkan_bind_texture(lightray_vulkan_core_t* core, u32 mesh_index, u32 texture_index)
{
	lightray_vulkan_result res = LIGHTRAY_VULKAN_RESULT_SUCCESS;;

	if (mesh_index > core->total_mesh_count - 1)
	{
		res = LIGHTRAY_VULKAN_RESULT_INVALID_MESH_INDEX;
		return res;
	}

	if (texture_index > core->total_texture_count - 1)
	{
		res = LIGHTRAY_VULKAN_RESULT_INVALID_TEXTURE_INDEX;
		return res;
	}

	if (core->mesh_render_pass_data_buffer[mesh_index].texture_index != LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX)
	{
		res = LIGHTRAY_VULKAN_RESULT_OVERWRITTEN_TEXTURE_INDEX;
	}

	core->mesh_render_pass_data_buffer[mesh_index].texture_index = texture_index;

	return res;
}

lightray_vulkan_result lightray_vulkan_execute_pre_render_pass_buffer_reorder(lightray_vulkan_core_t* core)
{
	for (u32 i = 0; i < core->total_mesh_count; i++)
	{
		core->mesh_render_pass_data_reordering_helper_buffer[i] = core->mesh_render_pass_data_buffer[i].index_buffer_offset;
	}

	u32 total_textured_mesh_count = 0;
	u32 total_untextured_mesh_count = 0;
	u32 untextured_static_mesh_count = 0;
	u32 untextured_skeletal_mesh_count = 0;
	u32 textured_static_mesh_count = 0;
	u32 textured_skeletal_mesh_count = 0;

	for (u32 i = 0; i < core->total_mesh_count; i++)
	{
		if (core->mesh_render_pass_data_buffer[i].texture_index != LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX)
		{
			if (core->mesh_render_pass_data_buffer[i].render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_STATIC)
			{
				textured_static_mesh_count++;
			}

			else if(core->mesh_render_pass_data_buffer[i].render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_SKELETAL)
			{
				textured_skeletal_mesh_count++;
			}

			total_textured_mesh_count++;
		}

		else
		{
			if (core->mesh_render_pass_data_buffer[i].render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_STATIC)
			{
				untextured_static_mesh_count++;
			}

			else if (core->mesh_render_pass_data_buffer[i].render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_SKELETAL)
			{
				untextured_skeletal_mesh_count++;
			}

			total_untextured_mesh_count++;
		}
	}

	// untextured static meshes -> untextred skeletal meshes -> textured static meshes -> textured skeletal meshes.

	core->static_mesh_render_pass_data_buffer_indices.untextured_starting_offset = 0;
	core->static_mesh_render_pass_data_buffer_indices.untextured_range = untextured_static_mesh_count;
	
	core->skeletal_mesh_render_pass_data_buffer_indices.untextured_starting_offset = untextured_static_mesh_count;
	core->skeletal_mesh_render_pass_data_buffer_indices.untextured_range = untextured_static_mesh_count + untextured_skeletal_mesh_count;

	core->static_mesh_render_pass_data_buffer_indices.textured_starting_offset = core->skeletal_mesh_render_pass_data_buffer_indices.untextured_range;
	core->static_mesh_render_pass_data_buffer_indices.textured_range = core->skeletal_mesh_render_pass_data_buffer_indices.untextured_range + textured_static_mesh_count;

	core->skeletal_mesh_render_pass_data_buffer_indices.textured_starting_offset = core->static_mesh_render_pass_data_buffer_indices.textured_range;
	core->skeletal_mesh_render_pass_data_buffer_indices.textured_range = core->static_mesh_render_pass_data_buffer_indices.textured_range + textured_skeletal_mesh_count;

	u32 last_untextured_static_mesh_index = core->static_mesh_render_pass_data_buffer_indices.untextured_starting_offset;
	u32 last_untextured_skeletal_mesh_index = core->skeletal_mesh_render_pass_data_buffer_indices.untextured_starting_offset;
	u32 last_textured_static_mesh_index = core->static_mesh_render_pass_data_buffer_indices.textured_starting_offset;
	u32 last_textured_skeletal_mesh_index = core->skeletal_mesh_render_pass_data_buffer_indices.textured_starting_offset;
	
	// gross as fuck, but works

	for (u32 j = 0; j < core->total_mesh_count; j++)
	{
		u32 swapped_counter = 0;

		for (u32 i = 0; i < core->total_mesh_count; i++)
		{
			const lightray_vulkan_mesh_render_pass_data_t* current_mesh = &core->mesh_render_pass_data_buffer[i];

			if (current_mesh->render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_STATIC)
			{
				if (current_mesh->texture_index == LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX)
				{
					swapped_counter += lightray_vulkan_swap_meshes_inplace(core->mesh_render_pass_data_buffer, i, &last_untextured_static_mesh_index, core->static_mesh_render_pass_data_buffer_indices.untextured_starting_offset, core->static_mesh_render_pass_data_buffer_indices.untextured_range, LIGHTRAY_VULKAN_MESH_RENDER_TYPE_STATIC, false);
				}

				else
				{
					swapped_counter += lightray_vulkan_swap_meshes_inplace(core->mesh_render_pass_data_buffer, i, &last_textured_static_mesh_index, core->static_mesh_render_pass_data_buffer_indices.textured_starting_offset, core->static_mesh_render_pass_data_buffer_indices.textured_range, LIGHTRAY_VULKAN_MESH_RENDER_TYPE_STATIC, true);
				}
			}

			else if (current_mesh->render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_SKELETAL)
			{
				if (current_mesh->texture_index == LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX)
				{
					swapped_counter += lightray_vulkan_swap_meshes_inplace(core->mesh_render_pass_data_buffer, i, &last_untextured_skeletal_mesh_index, core->skeletal_mesh_render_pass_data_buffer_indices.untextured_starting_offset, core->skeletal_mesh_render_pass_data_buffer_indices.untextured_range, LIGHTRAY_VULKAN_MESH_RENDER_TYPE_SKELETAL, false);
				}

				else
				{
					swapped_counter += lightray_vulkan_swap_meshes_inplace(core->mesh_render_pass_data_buffer, i, &last_textured_skeletal_mesh_index, core->skeletal_mesh_render_pass_data_buffer_indices.textured_starting_offset, core->skeletal_mesh_render_pass_data_buffer_indices.textured_range, LIGHTRAY_VULKAN_MESH_RENDER_TYPE_SKELETAL, true);
				}
			}
		}

		SUNDER_LOG("\nswapped counter at ");
		SUNDER_LOG(j);
		SUNDER_LOG(" : ");
		SUNDER_LOG(swapped_counter);

		if (swapped_counter == 0)
		{
			SUNDER_LOG("\nTERMINATED ITERATION AT ");
			SUNDER_LOG(j);

			break;
		}
	}

	//lightray_vulkan_quick_sort_mesh_render_pass_data(&core->mesh_render_pass_data_buffer[core->static_mesh_render_pass_data_buffer_indices.textured_starting_offset], 0, core->static_mesh_render_pass_data_buffer_indices.textured_range - 1, lightray_vulkan_quick_sort_compare_texture_index_less_mesh_render_pass_data);
	//lightray_vulkan_quick_sort_mesh_render_pass_data(&core->mesh_render_pass_data_buffer[core->skeletal_mesh_render_pass_data_buffer_indices.textured_starting_offset], 0, core->skeletal_mesh_render_pass_data_buffer_indices.textured_range - 1, lightray_vulkan_quick_sort_compare_texture_index_less_mesh_render_pass_data);

	for (u32 i = 0; i < core->total_mesh_count; i++)
	{
		const u32 current_original_mesh_index_buffer_offset = core->mesh_render_pass_data_reordering_helper_buffer[i];

		for (u32 j = 0; j < core->total_mesh_count; j++)
		{
			const u32 current_reordered_mesh_index_buffer_offset = core->mesh_render_pass_data_buffer[j].index_buffer_offset;

			if (current_original_mesh_index_buffer_offset == current_reordered_mesh_index_buffer_offset)
			{
				core->mesh_render_pass_data_mapping_buffer[i] = j;
			}
		}
	}

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

void lightray_vulkan_begin_command_buffer_recording(VkCommandBuffer command_buffer, const VkCommandBufferBeginInfo* begin_info)
{
	vkBeginCommandBuffer(command_buffer, begin_info);
}

void lightray_vulkan_end_command_buffer_recording(VkCommandBuffer command_buffer, VkQueue queue)
{
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(queue, 1, &submit_info, nullptr);
	vkQueueWaitIdle(queue);
}

void lightray_vulkan_copy_buffer(lightray_vulkan_core_t* core, VkBuffer dst, VkBuffer src, u64 dst_offset, u64 src_offset, u64 size)
{
	core->general_purpose_command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	core->general_purpose_command_buffer_begin_info.flags = 0;
	core->general_purpose_command_buffer_begin_info.pNext = nullptr;
	core->general_purpose_command_buffer_begin_info.pInheritanceInfo = nullptr;

	VkBufferCopy copy_region{};
	copy_region.dstOffset = dst_offset;
	copy_region.srcOffset = src_offset;
	copy_region.size = size;

	lightray_vulkan_begin_command_buffer_recording(core->general_purpose_command_buffer, &core->general_purpose_command_buffer_begin_info);

	vkCmdCopyBuffer(core->general_purpose_command_buffer, src, dst, 1, &copy_region);

	lightray_vulkan_end_command_buffer_recording(core->general_purpose_command_buffer, core->graphics_queue);
}

void lightray_vulkan_create_swapchain_framebuffers(lightray_vulkan_core_t* core)
{
	for (u32 i = 0; i < core->swapchain_image_in_use_count; i++)
	{
		VkImageView attachments[2]{};
		attachments[0] = core->swapchain_image_views[i];
		attachments[1] = core->texture_buffer[core->depth_texture_index].view; // depth image, later add defines for these

		core->swapchain_framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		core->swapchain_framebuffer_info.pAttachments = attachments;
		core->swapchain_framebuffer_info.attachmentCount = 2;
		core->swapchain_framebuffer_info.width = core->swapchain_info.imageExtent.width;
		core->swapchain_framebuffer_info.height = core->swapchain_info.imageExtent.height;
		core->swapchain_framebuffer_info.layers = 1;
		core->swapchain_framebuffer_info.flags = 0;
		core->swapchain_framebuffer_info.renderPass = core->render_pass;
		core->swapchain_framebuffer_info.pNext = nullptr;

		const VkResult swapchain_framebuffer_creation_result = vkCreateFramebuffer(core->logical_device, &core->swapchain_framebuffer_info, nullptr, &core->swapchain_framebuffers[i]);
	}
}

void lightray_vulkan_free_swapchain_framebuffers(lightray_vulkan_core_t* core)
{
	core->swapchain_image_in_use_count = lightray_vulkan_get_supported_swapchain_image_count(&core->surface_capabilities);

	for (u32 i = 0; i < core->swapchain_image_in_use_count; i++)
	{
		vkDestroyFramebuffer(core->logical_device, core->swapchain_framebuffers[i], nullptr);
	}
}

void lightray_vulkan_execute_render_pass(lightray_vulkan_core_t* core, u32 flags)
{
	static u32 current_frame = 0;
	u32 index_of_acquired_swapchain_image = 0;

	vkWaitForFences(core->logical_device, 1, &core->inflight_fences[current_frame], VK_TRUE, UINT64_MAX);

	vkAcquireNextImageKHR(core->logical_device, core->swapchain, UINT64_MAX, core->image_available_for_rendering_semaphores[current_frame], nullptr, &index_of_acquired_swapchain_image);

	// recreate swapchain if needed later

	vkResetFences(core->logical_device, 1, &core->inflight_fences[current_frame]);
	vkResetCommandBuffer(core->render_command_buffers[current_frame], 0);

	core->render_command_buffers_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	core->render_command_buffers_begin_info.flags = 0;
	core->render_command_buffers_begin_info.pNext = nullptr;
	core->render_command_buffers_begin_info.pInheritanceInfo = nullptr;

	core->render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	core->render_pass_begin_info.clearValueCount = 2;
	core->render_pass_begin_info.pClearValues = core->clear_values;
	core->render_pass_begin_info.framebuffer = core->swapchain_framebuffers[current_frame];
	core->render_pass_begin_info.renderArea.extent = core->swapchain_info.imageExtent;
	core->render_pass_begin_info.renderArea.offset = { 0,0 };
	core->render_pass_begin_info.renderPass = core->render_pass;
	core->render_pass_begin_info.pNext = nullptr;

	vkBeginCommandBuffer(core->render_command_buffers[current_frame], &core->render_command_buffers_begin_info);
	
	vkCmdBeginRenderPass(core->render_command_buffers[current_frame], &core->render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

	// opaque untextured static meshes
	vkCmdBindPipeline(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipelines[0]); // bind wireframe pipeline. render all wireframe stuff, bind opaque untextures static mehses, and then other stuff

	u64 vertex_buffer_offsets[2]{};
	vertex_buffer_offsets[0] = core->device_local_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_VERTEX_BUFFER_INDEX];
	vertex_buffer_offsets[1] = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_buffer_starting_index];

	VkBuffer vertex_buffers[2]{};
	vertex_buffers[0] = core->device_local_vram_arena.buffer;
	vertex_buffers[1] = core->host_visible_vram_arena.buffer;

	vkCmdBindVertexBuffers(core->render_command_buffers[current_frame], 0, 2, vertex_buffers, vertex_buffer_offsets);
	vkCmdBindIndexBuffer(core->render_command_buffers[current_frame], core->device_local_vram_arena.buffer, core->device_local_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_INDEX_BUFFER_INDEX], VK_INDEX_TYPE_UINT32);
	vkCmdSetViewport(core->render_command_buffers[current_frame], 0, 1, &core->viewport);
	vkCmdSetScissor(core->render_command_buffers[current_frame], 0, 1, &core->scissor);

	vkCmdBindDescriptorSets(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipeline_layouts[0], 0, 1, &core->descriptor_sets[0], 1, &core->perspective_camera_dynamic_offset);

	for (u32 i = core->static_mesh_render_pass_data_buffer_indices.untextured_starting_offset; i < core->static_mesh_render_pass_data_buffer_indices.untextured_range; i++)
	{
		vkCmdDrawIndexed(core->render_command_buffers[current_frame], core->mesh_render_pass_data_buffer[i].index_count, core->mesh_render_pass_data_buffer[i].instance_count, core->mesh_render_pass_data_buffer[i].index_buffer_offset, 0, core->mesh_render_pass_data_buffer[i].instance_buffer_offset);
	}

	// opaque untextured skeletal meshes

	vkCmdBindPipeline(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipelines[3]);

	for (u32 i = core->skeletal_mesh_render_pass_data_buffer_indices.untextured_starting_offset; i < core->skeletal_mesh_render_pass_data_buffer_indices.untextured_range; i++)
	{
		vkCmdDrawIndexed(core->render_command_buffers[current_frame], core->mesh_render_pass_data_buffer[i].index_count, core->mesh_render_pass_data_buffer[i].instance_count, core->mesh_render_pass_data_buffer[i].index_buffer_offset, 0, core->mesh_render_pass_data_buffer[i].instance_buffer_offset);
	}

	// opaque textured static meshes
	vkCmdBindPipeline(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipelines[2]);

	u32 previous_texture_index = 0;
	const u32 first_texture_index = core->mesh_render_pass_data_buffer[core->static_mesh_render_pass_data_buffer_indices.textured_starting_offset].texture_index;
	vkCmdBindDescriptorSets(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipeline_layouts[0], 0, 1, &core->descriptor_sets[first_texture_index], 1, &core->perspective_camera_dynamic_offset);

	for (u32 i = core->static_mesh_render_pass_data_buffer_indices.textured_starting_offset; i < core->static_mesh_render_pass_data_buffer_indices.textured_range; i++)
	{
		const u32 current_texture_index = core->mesh_render_pass_data_buffer[i].texture_index;

		if (current_texture_index != previous_texture_index)
		{
			vkCmdBindDescriptorSets(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipeline_layouts[0], 0, 1, &core->descriptor_sets[current_texture_index], 1, &core->perspective_camera_dynamic_offset);
			previous_texture_index = current_texture_index;
		}

		vkCmdDrawIndexed(core->render_command_buffers[current_frame], core->mesh_render_pass_data_buffer[i].index_count, core->mesh_render_pass_data_buffer[i].instance_count, core->mesh_render_pass_data_buffer[i].index_buffer_offset, 0, core->mesh_render_pass_data_buffer[i].instance_buffer_offset);
	}

	// opaque textured skeletal meshes
	// ...
	u64 vertex_buffer_offsets2[2]{};
	vertex_buffer_offsets2[0] = core->device_local_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_VERTEX_BUFFER_INDEX];
	vertex_buffer_offsets2[1] = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_glyph_buffer_starting_index];

	VkBuffer vertex_buffers2[2]{};
	vertex_buffers2[0] = core->device_local_vram_arena.buffer;
	vertex_buffers2[1] = core->host_visible_vram_arena.buffer;

	vkCmdBindDescriptorSets(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipeline_layouts[0], 0, 1, &core->descriptor_sets[2], 1, &core->overlay_camera_dynamic_offset);
	vkCmdBindPipeline(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipelines[4]);
	vkCmdBindVertexBuffers(core->render_command_buffers[current_frame], 0, 2, vertex_buffers2, vertex_buffer_offsets2);
	vkCmdDrawIndexed(core->render_command_buffers[current_frame], 6, core->overlay_core.current_glyph_render_instance_count, core->overlay_index_subbuffer_offset, 0, 0);

	vkCmdEndRenderPass(core->render_command_buffers[current_frame]);

	vkEndCommandBuffer(core->render_command_buffers[current_frame]);
	
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags pipeline_wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit_info.pWaitSemaphores = &core->image_available_for_rendering_semaphores[current_frame];
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitDstStageMask = &pipeline_wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &core->render_command_buffers[current_frame];
	submit_info.pSignalSemaphores = &core->image_finished_rendering_semaphores[current_frame];
	submit_info.signalSemaphoreCount = 1;

	vkQueueSubmit(core->graphics_queue, 1, &submit_info, core->inflight_fences[current_frame]);

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &core->image_finished_rendering_semaphores[current_frame];
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &core->swapchain;
	present_info.pImageIndices = &index_of_acquired_swapchain_image;
	present_info.pResults = nullptr;
	present_info.pNext = nullptr;

	vkQueuePresentKHR(core->present_queue, &present_info);

	current_frame = (current_frame + 1) % core->swapchain_image_in_use_count;
}

VkFormat lightray_vulkan_get_supported_image_format(VkPhysicalDevice gpu, const VkFormat* formats, u32 format_count, VkImageTiling tiling, VkImageType type, VkFormatFeatureFlags features, VkImageUsageFlags usage, VkFormat prefered_format)
{
	VkPhysicalDeviceImageFormatInfo2 image_format_info{};
	image_format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	image_format_info.type = type;
	image_format_info.usage = usage;
	image_format_info.tiling = tiling;
	image_format_info.flags = 0;
	image_format_info.pNext = nullptr;

	VkFormat last_supported_format = VK_FORMAT_UNDEFINED;

	for (u32 i = 0; i < format_count; i++)
	{
		image_format_info.format = formats[i];

		VkImageFormatProperties2 format_properties{};
		format_properties.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;

		const VkResult supported = vkGetPhysicalDeviceImageFormatProperties2(gpu, &image_format_info, &format_properties);

		if (supported == VK_SUCCESS)
		{
			last_supported_format = formats[i];

			if (last_supported_format == prefered_format)
			{
				return last_supported_format;
			}
		}
	}

	return last_supported_format;
	
	// old version, maybe useful
	/*
	VkFormat last_supported_format = VK_FORMAT_UNDEFINED;

	for (u32 i = 0; i < format_count; i++)
	{
		VkFormatProperties format_properties{};
		vkGetPhysicalDeviceFormatProperties(gpu, formats[i], &format_properties);

		if (tiling == VK_IMAGE_TILING_LINEAR && (format_properties.linearTilingFeatures & features) == features)
		{
			last_supported_format = formats[i];

			if (last_supported_format == prefered_format)
			{
				return formats[i];
			}
		}

		else if (tiling == VK_IMAGE_TILING_OPTIMAL && ((format_properties.optimalTilingFeatures & features) == features))
		{
			last_supported_format = formats[i];

			if (last_supported_format == prefered_format)
			{
				return formats[i];
			}
		}
	}

	return last_supported_format;
	*/
}

bool lightray_vulkan_is_format_supported(VkPhysicalDevice gpu, VkFormat format)
{
	VkFormatProperties properties{};
	vkGetPhysicalDeviceFormatProperties(gpu, format, &properties);

	return properties.linearTilingFeatures != 0 || properties.optimalTilingFeatures != 0 || properties.bufferFeatures != 0;
}

void lightray_vulkan_set_camera_fov(lightray_vulkan_core_t* core, u32 camera_index, f32 desired_fov)
{
	core->camera_buffer[camera_index].fov = desired_fov;
	core->cvp_buffer[camera_index].projection = lightray_construct_perspective_projection_matrix(core->camera_buffer[camera_index].fov, lightray_compute_aspect_ratio((f32)core->swapchain_info.imageExtent.width, (f32)core->swapchain_info.imageExtent.height), core->camera_buffer[camera_index].near_clip_plane_distance, core->camera_buffer[camera_index].far_clip_plane_distance, core->camera_buffer[camera_index].vulkan_y_flip);
}

void lightray_vulkan_populate_mesh_binding_offset_buffer(const lightray_vulkan_core_t* core, lightray_scene_t* scene)
{
	for (u32 i = 0; i < core->total_mesh_count; i++)
	{
		scene->mesh_binding_offsets[i].current_opaque_instance_model_index = core->mesh_render_pass_data_buffer[i].instance_buffer_offset;
		scene->mesh_binding_offsets[i].last_opaque_instance_model_index = core->mesh_render_pass_data_buffer[i].instance_buffer_offset + core->mesh_render_pass_data_buffer[i].instance_count - 1;
		scene->mesh_binding_metadata_buffer[i].instance_count = core->mesh_render_pass_data_buffer[i].instance_count;
	}
}

u32 lightray_vulkan_get_total_mesh_instance_count(const lightray_vulkan_static_mesh_metadata_t* static_mesh_metadata_buffer, u32 static_mesh_count, const lightray_vulkan_skeletal_mesh_metadata_t* skeletal_mesh_metadata_buffer, u32 skeletal_mesh_count)
{
	u32 total_instance_count = 0;

	if (static_mesh_count != 0)
	{
		for (u32 i = 0; i < static_mesh_count; i++)
		{
			total_instance_count += static_mesh_metadata_buffer[i].instance_count;
		}
	}

	if (skeletal_mesh_count != 0)
	{
		for (u32 i = 0; i < skeletal_mesh_count; i++)
		{
			total_instance_count += skeletal_mesh_metadata_buffer[i].instance_count;
		}
	}

	return total_instance_count;
}

void lightray_vulkan_set_animation_playback_scale(lightray_vulkan_core_t* core, u32 playback_command_index, f32 scale)
{
	core->animation_core.playback_command_buffer[playback_command_index].scale = scale;
}

u32 lightray_vulkan_push_animation_playback_command(lightray_vulkan_core_t* core, u32 animation_index, u32 skeleton_index, u32 instance_index, bool loop)
{
	const u32 index = core->animation_core.playback_command_count;

	if (loop)
	{
		SUNDER_SET_BIT(core->animation_core.looped_playback_flags, index, 1ull);
	}

	core->animation_core.playback_command_buffer[index].animation_index = animation_index;
	core->animation_core.playback_command_buffer[index].skeletal_mesh_index = skeleton_index;
	core->animation_core.playback_command_buffer[index].instance_index = instance_index;
	core->animation_core.playback_command_buffer[index].scale = 1.0f;
	core->animation_core.playback_command_buffer[index].time = 0;
	core->animation_core.playback_command_buffer[index].ticks = 0;
	core->animation_core.playback_command_count++;

	return index;
}

void lightray_vulkan_withdraw_animation_playback_command(lightray_vulkan_core_t* core, u32 playback_command_index)
{
	// swapback last and provided indices, decrement command count, zero the bit for that index
}

void lightray_vulkan_flush_animation_playback_command_buffer(lightray_vulkan_core_t* core)
{
	for (u32 i = 0; i < core->animation_core.playback_command_count; i++)
	{
		core->animation_core.playback_command_buffer[i].instance_index = 0;
		core->animation_core.playback_command_buffer[i].animation_index = 0;
		core->animation_core.playback_command_buffer[i].skeletal_mesh_index = 0;
		core->animation_core.playback_command_buffer[i].scale = 0.0f;
		core->animation_core.playback_command_buffer[i].time = 0.0f;
		core->animation_core.playback_command_buffer[i].ticks = 0.0f;
	}

	core->animation_core.playback_flags = 0;
	core->animation_core.looped_playback_flags = 0;
	core->animation_core.playback_command_count = 0;
}

lightray_vulkan_result lightray_vulkan_play_animation(lightray_vulkan_core_t* core, u32 playback_command_index)
{
	if (SUNDER_IS_ANY_BIT_SET(core->animation_core.looped_playback_flags, playback_command_index, 1ull))
	{
		SUNDER_SET_BIT(core->animation_core.playback_flags, playback_command_index, 1ull);
	}
	
	else
	{
		SUNDER_SET_BIT(core->animation_core.playback_flags, playback_command_index, 1ull);
		core->animation_core.playback_command_buffer[playback_command_index].time = 0;
		core->animation_core.playback_command_buffer[playback_command_index].ticks = 0;
	}

	return LIGHTRAY_VULKAN_RESULT_SUCCESS;
}

bool lightray_vulkan_quick_sort_compare_texture_index_less_mesh_render_pass_data(const lightray_vulkan_mesh_render_pass_data_t* i, const lightray_vulkan_mesh_render_pass_data_t* j)
{
	return i->texture_index < j->texture_index;
}

SUNDER_IMPLEMENT_QUICK_SORT_PARTITION_FUNCTION(lightray_vulkan_mesh_render_pass_data_t, mesh_render_pass_data, lightray_vulkan)
SUNDER_IMPLEMENT_QUICK_SORT_FUNCTION(lightray_vulkan_mesh_render_pass_data_t, mesh_render_pass_data, lightray_vulkan)

SUNDER_IMPLEMENT_EXISTS_FUNCTION(VkFilter, lightray_vulkan, vk_filter, u32)

SUNDER_IMPLEMENT_QUERY_BUFFER_INDEX_FUNCTION(VkFilter, lightray_vulkan, vk_filter, u32)

void lightray_vulkan_log_mesh_render_pass_data_buffer(const lightray_vulkan_core_t* core)
{
	for (u32 i = 0; i < core->total_mesh_count; i++)
	{
		SUNDER_LOG("index count: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].index_count);
		SUNDER_LOG("\n");
		SUNDER_LOG("index buffer offset: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].index_buffer_offset);
		SUNDER_LOG("\n");
		SUNDER_LOG("vertex count: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].vertex_count);
		SUNDER_LOG("\n");
		SUNDER_LOG("vertex buffer offset: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].vertex_buffer_offset);
		SUNDER_LOG("\n");
		SUNDER_LOG("instance  count: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].instance_count);
		SUNDER_LOG("\n");
		SUNDER_LOG("instance buffer offset: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].instance_buffer_offset);
		SUNDER_LOG("\ntexture index: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].texture_index);
		SUNDER_LOG("\nrender type: ");
		SUNDER_LOG(core->mesh_render_pass_data_buffer[i].render_type);
		SUNDER_LOG("\n\n");
	}
}

u32 lightray_vulkan_swap_meshes_inplace(lightray_vulkan_mesh_render_pass_data_t* mesh_render_pass_data_buffer, u32 i, u32* dst_index, u32 starting_index, u32 range, lightray_vulkan_mesh_render_type render_type, bool textured)
{
	 // recursion sucks elden ring fire giant balls

	const u32 derefed_dst_index = *dst_index;

	if (i >= starting_index && i < range)
	{
		return 0;
	}

	if (!(derefed_dst_index >= starting_index && derefed_dst_index < range))
	{
		return 0;
	}

	const lightray_vulkan_mesh_render_pass_data_t* mesh_at_derefed_index = &mesh_render_pass_data_buffer[derefed_dst_index];

	if (mesh_at_derefed_index->render_type == render_type)
	{
		// xddxdxdxdxdxdxdxdxdxddxxddxdxdxddxdxddxdxxddxdxxdxdxdxdxddxxdx
		if (textured && mesh_at_derefed_index->render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_SKELETAL || !textured && mesh_at_derefed_index->render_type == LIGHTRAY_VULKAN_MESH_RENDER_TYPE_STATIC)
		{
			(*dst_index)++;
			return lightray_vulkan_swap_meshes_inplace(mesh_render_pass_data_buffer, i, dst_index, starting_index, range, render_type, textured);
		}
	}

	const lightray_vulkan_mesh_render_pass_data_t bubbled_mesh = mesh_render_pass_data_buffer[i];
	mesh_render_pass_data_buffer[i] = mesh_render_pass_data_buffer[derefed_dst_index];
	mesh_render_pass_data_buffer[derefed_dst_index] = bubbled_mesh;

	(*dst_index)++;

	return 1;
}

void lightray_vulkan_apply_bind_pose(lightray_vulkan_core_t* core, u32 skeleton_index, u32 instance_index)
{
	const lightray_skeleton_t current_skeleton = core->animation_core.skeleton_buffer[skeleton_index];
	const u32 current_bone_count = current_skeleton.bone_count;
	const u32 current_bone_buffer_offset = current_skeleton.bone_buffer_offset;
	const u32 current_computed_bone_transform_matrix_buffer_offset = current_skeleton.computed_bone_transform_matrix_buffer_offset;
	const u32 current_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance = lightray_compute_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance(instance_index, current_bone_count, current_computed_bone_transform_matrix_buffer_offset);

	for (u32 b = 0; b < current_bone_count; b++)
	{
		core->animation_core.computed_bone_matrix_buffer[current_computed_bone_transform_matrix_buffer_offset_with_respect_to_instance + b] = core->animation_core.bone_bind_pose_matrix_buffer[current_bone_buffer_offset + b];
	}
}

void lightray_vulkan_initialize_core_tick_end_data(lightray_vulkan_core_t* core, lightray_scene_t* scene, lightray_vulkan_core_tick_end_data_t* tick_data)
{
	tick_data->scene = scene;

	tick_data->instance_model_buffer_copy_data.dst_size = core->host_visible_vram_arena.capacity;
	tick_data->instance_model_buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_buffer_starting_index];
	tick_data->instance_model_buffer_copy_data.src_size = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), core->cpu_side_instance_count);
	tick_data->instance_model_buffer_copy_data.src_offset = 0;
	tick_data->instance_model_buffer_copy_data.bytes_to_write = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), core->cpu_side_instance_count);

	const u64 computed_bone_transform_matrix_buffer_range = sunder_compute_array_size_in_bytes(sizeof(glm::mat4), core->animation_core.total_computed_bone_transform_matrix_buffer_bone_count);

	tick_data->computed_bone_matrix_buffer_copy_data.dst_size = computed_bone_transform_matrix_buffer_range;
	tick_data->computed_bone_matrix_buffer_copy_data.dst_offset = 0;
	tick_data->computed_bone_matrix_buffer_copy_data.src_size = computed_bone_transform_matrix_buffer_range;
	tick_data->computed_bone_matrix_buffer_copy_data.src_offset = 0;
	tick_data->computed_bone_matrix_buffer_copy_data.bytes_to_write = computed_bone_transform_matrix_buffer_range;
}

void lightray_vulkan_tick_core_begin(lightray_vulkan_core_t* core)
{
	core->current_time = (f32)glfwGetTime();
	core->delta_time = core->current_time - core->initial_time;
	core->initial_time = core->current_time;

	core->fps = 1.0f / core->delta_time;

	glfwPollEvents();
}

void lightray_vulkan_tick_core_end(lightray_vulkan_core_t* core, const lightray_vulkan_core_tick_end_data_t* tick_data)
{
	for (u32 i = 0; i < core->animation_core.playback_command_count; i++)
	{
		if (SUNDER_IS_ANY_BIT_SET(core->animation_core.playback_flags, i, 1ull))
		{
			const u32 current_instance_index = core->animation_core.playback_command_buffer[i].instance_index;
			const u32 current_animation_index = core->animation_core.playback_command_buffer[i].animation_index;
			const u32 current_skeleton_index = core->animation_core.playback_command_buffer[i].skeletal_mesh_index;
			const f32 current_animation_duration = core->animation_core.animation_buffer[current_animation_index].duration;

			if (SUNDER_IS_ANY_BIT_SET(core->animation_core.looped_playback_flags, i, 1ull))
			{
				core->animation_core.playback_command_buffer[i].time += core->delta_time * core->animation_core.playback_command_buffer[i].scale;

				const f32 ticks_per_second = core->animation_core.animation_buffer[current_animation_index].tickrate;
				const f32 time_in_ticks = core->animation_core.playback_command_buffer[i].time * ticks_per_second;
				const f32 animation_ticks = SUNDER_CAST2(f32)fmod(time_in_ticks, core->animation_core.animation_buffer[current_animation_index].duration);
				core->animation_core.playback_command_buffer[i].ticks = animation_ticks;

				lightray_compute_interpolated_skeleton_transform(&core->animation_core, current_animation_index, current_skeleton_index, current_instance_index);
				SUNDER_ZERO_BIT(core->animation_core.playback_flags, i, 1ull);
			}

			else
			{
				core->animation_core.playback_command_buffer[i].time += core->delta_time * core->animation_core.playback_command_buffer[i].scale;
				const f32 ticks_per_second = core->animation_core.animation_buffer[current_animation_index].tickrate;
				const f32 time_in_ticks = core->animation_core.playback_command_buffer[i].time * ticks_per_second;
				core->animation_core.playback_command_buffer[i].ticks = time_in_ticks;

				// advances the pose by one frame since the last frame, so that it wraps around and renders in bind pose (ex. 19 -> 0)
				if (time_in_ticks >= current_animation_duration)
				{
					core->animation_core.playback_command_buffer[i].time = 0.0f;
					core->animation_core.playback_command_buffer[i].ticks = 0.0f;

					SUNDER_ZERO_BIT(core->animation_core.playback_flags, i, 1ull);

					lightray_compute_interpolated_skeleton_transform(&core->animation_core, current_animation_index, current_skeleton_index, current_instance_index);
				}

				// computes the full pose across frames and hangs at the last frame (ex. 0 -> 19)
				else
				{
					lightray_compute_interpolated_skeleton_transform(&core->animation_core, current_animation_index, current_skeleton_index, current_instance_index);
				}
			}
		}
	}

	//////////////////////////////////

	i32 current_temp_buffer_offset = 0;

	for (u32 i = 0; i < core->overlay_core.text_element_count; i++)
	{
		const lightray_overlay_text_element_type current_type = core->overlay_core.text_element_buffer[i].type;
		const void* current_data_ptr = core->overlay_core.text_element_buffer[i].data;
		char temp_buffer[50]{};
		i32 data_length = 0;

		if (current_type == LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_F32)
		{
			const f32* f32_ptr = SUNDER_CAST(f32*, current_data_ptr);
			data_length = sunder_float_to_string(*f32_ptr, temp_buffer, core->overlay_core.text_element_buffer[i].precision, 10);
		}

		else if (current_type == LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_F64)
		{
			const f64* f64_ptr = SUNDER_CAST(f64*, current_data_ptr);
			data_length = sunder_float_to_string(*f64_ptr, temp_buffer, core->overlay_core.text_element_buffer[i].precision, 10);
		}

		else if (current_type == LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_U32)
		{
			const u32* u32_ptr = SUNDER_CAST(u32*, current_data_ptr);
			data_length = sunder_uint_to_string(*u32_ptr, temp_buffer, 10);
		}

		else if (current_type == LIGHTRAY_OVERLAY_TEXT_ELEMENT_TYPE_STRING)
		{
			u32 local_counter = 0;
			cstring_literal* string_ptr = SUNDER_CAST(cstring_literal*, current_data_ptr);

			for (u64 chr = 0; chr < core->overlay_core.text_element_buffer[i].element_count; chr++)
			{
				temp_buffer[local_counter] = string_ptr[chr];
				local_counter++;
			}

			data_length = SUNDER_CAST2(i32)core->overlay_core.text_element_buffer[i].element_count;
		}

		//const f64 scale = core->overlay_core.text_element_buffer[i].glyph_size / core->swapchain_info.imageExtent.height;
		const f64 scale = core->overlay_core.text_element_buffer[i].glyph_size;

		const f64 atlas_width = core->overlay_core.font_atlas_buffer[0].width;
		const f64 atlas_height = core->overlay_core.font_atlas_buffer[0].height;

		f64 pen_x = core->overlay_core.text_element_buffer[i].position.x;
		f64 pen_y = core->overlay_core.text_element_buffer[i].position.y;
		f64 ascender = core->overlay_core.font_atlas_buffer[0].ascender;
		f64 descender = core->overlay_core.font_atlas_buffer[0].descender;
		//f64 baseline = pen_y - ascender * scale;
		f64 baseline = pen_y - ((ascender * scale) / core->swapchain_info.imageExtent.height);

		for (i32 j = 0; j < data_length; j++)
		{
			const char ch = temp_buffer[j];
			const u32 glyph_index = (u32)ch;
			const lightray_glyph_t glyph = core->overlay_core.glyph_buffer[glyph_index];

			if (!glyph.has_bounds)
			{
				//pen_x += glyph.advance * scale;
				pen_x += (glyph.advance * scale) / core->swapchain_info.imageExtent.width;
				continue;
			}

			//const f64 width = (glyph.plane_bounds.right - glyph.plane_bounds.left) * scale;
			//const f64 height = (glyph.plane_bounds.top - glyph.plane_bounds.bottom) * scale;
			 
			const f64 width = ((glyph.plane_bounds.right - glyph.plane_bounds.left) * scale) / core->swapchain_info.imageExtent.width;
			const f64 height = ((glyph.plane_bounds.top - glyph.plane_bounds.bottom) * scale) / core->swapchain_info.imageExtent.height;

			//f64 pos_x = pen_x + glyph.plane_bounds.left * scale;
			//f64 pos_y = baseline + glyph.plane_bounds.bottom * scale;

			f64 pos_x = pen_x + (glyph.plane_bounds.left * scale) / core->swapchain_info.imageExtent.width;
			f64 pos_y = baseline + (glyph.plane_bounds.bottom * scale) / core->swapchain_info.imageExtent.height;

			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos_x, pos_y, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));

			const f64 u0 = glyph.atlas_bounds.left / atlas_width;
			const f64 v0 = (glyph.atlas_bounds.top / atlas_height);
			const f64 u1 = glyph.atlas_bounds.right / atlas_width;
			const f64 v1 = (glyph.atlas_bounds.bottom / atlas_height);

			core->overlay_core.render_instance_glyph_buffer[current_temp_buffer_offset + j].model_matrix = model;
			core->overlay_core.render_instance_glyph_buffer[current_temp_buffer_offset + j].uv_min = glm::vec2(u0, v0);
			core->overlay_core.render_instance_glyph_buffer[current_temp_buffer_offset + j].uv_max = glm::vec2(u1, v1);
			core->overlay_core.render_instance_glyph_buffer[current_temp_buffer_offset + j].color = core->overlay_core.text_element_buffer[i].color;

			//pen_x += glyph.advance * scale;
			pen_x += (glyph.advance * scale) / core->swapchain_info.imageExtent.width;
		}

		current_temp_buffer_offset += data_length;
		core->overlay_core.current_glyph_render_instance_count += data_length;
	}

	sunder_buffer_copy_data_t render_instance_glyph_buffer_copy_data{};
	render_instance_glyph_buffer_copy_data.dst_size = core->host_visible_vram_arena.capacity;
	render_instance_glyph_buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_glyph_buffer_starting_index];
	render_instance_glyph_buffer_copy_data.src_size = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), 256);
	render_instance_glyph_buffer_copy_data.src_offset = 0;
	render_instance_glyph_buffer_copy_data.bytes_to_write = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), 256);
	const u64 render_instance_glyph_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->overlay_core.render_instance_glyph_buffer, &render_instance_glyph_buffer_copy_data);

	/*
	cstring_literal* string = "-69";
	char buffer[10]{};
	char fbuffer[10]{};
	const i32 conversion_res = sunder_integer_to_string(162, buffer, 10);
	const i32 fconversion_res = sunder_float_to_string(3.6, fbuffer, 2, 10);

	for (i32 i = 0; i < fconversion_res; i++)
	{
		SUNDER_LOG(fbuffer[i]);
	}

	const f64 scale = 32.0 / core->swapchain_info.imageExtent.height;

	const f64 atlas_width = core->overlay_core.font_atlas_buffer[0].width;
	const f64 atlas_height = core->overlay_core.font_atlas_buffer[0].height;

	f64 pen_x = 0.0;
	f64 pen_y = 0.0;
	f64 ascender = core->overlay_core.font_atlas_buffer[0].ascender;
	f64 descender = core->overlay_core.font_atlas_buffer[0].descender;
	f64 baseline = pen_y - ascender * scale;

	for (u32 i = 0; i < 3; i++)
	{
		const u8 ch = fbuffer[i];
		const u32 glyph_index = ch;
		const lightray_glyph_t glyph = core->overlay_core.glyph_buffer[glyph_index];

		if (!glyph.has_bounds)
		{
			pen_x += glyph.advance * scale;
			continue;
		}

		const f64 width = (glyph.plane_bounds.right - glyph.plane_bounds.left) * scale;
		const f64 height = (glyph.plane_bounds.top - glyph.plane_bounds.bottom) * scale;

		SUNDER_LOG("\n\n");
		SUNDER_LOG("\nwidth: ");
		SUNDER_LOG(width);
		SUNDER_LOG("\nheight: ");
		SUNDER_LOG(height);

		f64 pos_x = pen_x + glyph.plane_bounds.left * scale;
		f64 pos_y = baseline + glyph.plane_bounds.bottom * scale;

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos_x, pos_y, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));

		const f64 u0 = glyph.atlas_bounds.left / atlas_width;
		const f64 v0 = (glyph.atlas_bounds.top / atlas_height);
		const f64 u1 = glyph.atlas_bounds.right / atlas_width;
		const f64 v1 = (glyph.atlas_bounds.bottom / atlas_height);

		core->overlay_core.render_instance_glyph_buffer[i].model_matrix = model;
		core->overlay_core.render_instance_glyph_buffer[i].uv_min = glm::vec2(u0, v0);
		core->overlay_core.render_instance_glyph_buffer[i].uv_max = glm::vec2(u1, v1);
		core->overlay_core.render_instance_glyph_buffer[i].color = glm::vec3(1.0f, 0.25f, 0.0f);

		pen_x += glyph.advance * scale;
	}

	// push updated data on the gpu
	sunder_buffer_copy_data_t render_instance_glyph_buffer_copy_data{};
	render_instance_glyph_buffer_copy_data.dst_size = core->host_visible_vram_arena.capacity;
	render_instance_glyph_buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_glyph_buffer_starting_index];
	render_instance_glyph_buffer_copy_data.src_size = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), 256);
	render_instance_glyph_buffer_copy_data.src_offset = 0;
	render_instance_glyph_buffer_copy_data.bytes_to_write = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), 256);
	const u64 render_instance_glyph_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->overlay_core.render_instance_glyph_buffer, &render_instance_glyph_buffer_copy_data);
	*/

	//////////////////////////////////

	for (u32 i = 0; i < tick_data->scene->mesh_binding_count; i++)
	{
		if (SUNDER_IS_ANY_BIT_SET(tick_data->scene->visibility_flags, i, 1ull))
		{
			const u32 transform_index = tick_data->scene->mesh_binding_buffer[i].transform_index;

			const glm::vec3 pos = tick_data->scene->position_buffer[transform_index];
			const glm::vec3 rot = tick_data->scene->rotation_buffer[transform_index];
			const glm::vec3 scale = tick_data->scene->scale_buffer[transform_index];
			const glm::quat orientation = glm::quat(rot);
			const glm::mat4 model = glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(orientation) * glm::scale(glm::mat4(1.0f), scale);

			const u32 instance_model_index = tick_data->scene->mesh_binding_buffer[i].instance_model_index;
			core->cpu_side_render_instance_buffer[instance_model_index].model.model = model;
		}
	}

	const u64 cpu_side_cvp_buffer_size_in_bytes = sunder_compute_array_size_in_bytes(sizeof(lightray_cvp_t), core->total_camera_count);
	const u64 host_visible_cvp_buffer_offset = core->host_visible_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_CVP_BUFFER_STARTING_INDEX];
	u8* u8_gpu_side_cvp_buffer_ptr = SUNDER_CAST(u8*, core->cpu_side_host_visible_vram_arena_view);
	void* void_cpu_side_cvp_buffer_ptr = core->cvp_buffer;
	u8* u8_cpu_side_cvp_buffer_ptr = SUNDER_CAST(u8*, void_cpu_side_cvp_buffer_ptr);
	const u32 cvp_step = sizeof(lightray_cvp_t);
	u32 accumulated_step = 0;
	u32 current_step = 0;

	for (u64 i = 0; i < cpu_side_cvp_buffer_size_in_bytes; i++)
	{
		if (accumulated_step < cvp_step)
		{
			accumulated_step++;
		}

		u8_gpu_side_cvp_buffer_ptr[host_visible_cvp_buffer_offset + i + current_step] = u8_cpu_side_cvp_buffer_ptr[i];

		if (accumulated_step == cvp_step)
		{
			accumulated_step = 0;
			current_step += cvp_step;
		}
	}

	const u64 instance_model_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->cpu_side_render_instance_buffer, &tick_data->instance_model_buffer_copy_data);
	const u64 computed_bone_matrix_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_storage_vram_arena_view, core->animation_core.computed_bone_matrix_buffer, &tick_data->computed_bone_matrix_buffer_copy_data);
}

void lightray_vulkan_log_mesh_render_pass_data_mapping_buffer(const lightray_vulkan_core_t* core)
{
	SUNDER_LOG("\nMAPPING BUFFER\n================================\n");

	for (u32 i = 0; i < core->total_mesh_count; i++)
	{
		SUNDER_LOG(core->mesh_render_pass_data_mapping_buffer[i]);
		SUNDER_LOG("\n");
	}
}

void lightray_vulkan_initialize_camera(lightray_vulkan_core_t* core, const lightray_camera_initialization_data_t* initialization_data)
{
	core->camera_buffer[initialization_data->camera_index].rotation = initialization_data->rotation;
	core->camera_buffer[initialization_data->camera_index].position = initialization_data->position;
	core->camera_buffer[initialization_data->camera_index].sensitivity = initialization_data->sensitivity;
	core->camera_buffer[initialization_data->camera_index].first_tick = true;
	core->camera_buffer[initialization_data->camera_index].projection_kind = initialization_data->projection_kind;
	core->camera_buffer[initialization_data->camera_index].vulkan_y_flip = initialization_data->vulkan_y_flip;
	core->camera_buffer[initialization_data->camera_index].near_clip_plane_distance = initialization_data->near_clip_plane_distance;
	core->camera_buffer[initialization_data->camera_index].far_clip_plane_distance = initialization_data->far_clip_plane_distance;

	if (initialization_data->projection_kind == LIGHTRAY_CAMERA_PROJECTION_KIND_PERSPECTIVE)
	{
		core->cvp_buffer[initialization_data->camera_index].view = glm::lookAt(initialization_data->position, glm::vec3(0.0f), glm::vec3(LIGHTRAY_WORLD_UP_VECTOR));

		lightray_vulkan_set_camera_fov(core, initialization_data->camera_index, initialization_data->fov);
	}

	else if(initialization_data->projection_kind == LIGHTRAY_CAMERA_PROJECTION_KIND_ORTHOGRAPHIC)
	{
		core->camera_buffer[initialization_data->camera_index].left = initialization_data->left;
		core->camera_buffer[initialization_data->camera_index].right = initialization_data->right;
		core->camera_buffer[initialization_data->camera_index].bottom = initialization_data->bottom;
		core->camera_buffer[initialization_data->camera_index].top = initialization_data->top;

		core->cvp_buffer[initialization_data->camera_index].projection = lightray_construct_orthographic_projection_matrix(initialization_data->left, initialization_data->right, initialization_data->bottom, initialization_data->top, initialization_data->near_clip_plane_distance, initialization_data->far_clip_plane_distance, initialization_data->vulkan_y_flip);
		core->cvp_buffer[initialization_data->camera_index].view = glm::mat4(1.0f); // later adjust this
	}
}

lightray_camera_t* lightray_vulkan_get_camera(const lightray_vulkan_core_t* core, u32 camera_index)
{
	return &core->camera_buffer[camera_index];
}

lightray_vulkan_result lightray_vulkan_bind_perspective_camera(lightray_vulkan_core_t* core, u32 camera_index)
{
	if (sunder_valid_index(camera_index, core->total_camera_count))
	{
		if (core->camera_buffer[camera_index].projection_kind == LIGHTRAY_CAMERA_PROJECTION_KIND_PERSPECTIVE)
		{
			core->perspective_camera_dynamic_offset = SUNDER_CAST2(u32)core->host_visible_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_CVP_BUFFER_STARTING_INDEX + camera_index];
			core->bound_perspective_camera_index = camera_index;
			SUNDER_SET_BIT(core->flags, LIGHTRAY_VULKAN_BITS_PERSPECTIVE_CAMERA_BOUND_BIT, 1ull);

			return LIGHTRAY_VULKAN_RESULT_SUCCESS;
		}

		return LIGHTRAY_VULKAN_RESULT_FAILURE;
	}

	return LIGHTRAY_VULKAN_RESULT_FAILURE;
}

lightray_vulkan_result lightray_vulkan_bind_overlay_camera(lightray_vulkan_core_t* core, u32 camera_index)
{
	if (sunder_valid_index(camera_index, core->total_camera_count))
	{
		if (core->camera_buffer[camera_index].projection_kind == LIGHTRAY_CAMERA_PROJECTION_KIND_ORTHOGRAPHIC)
		{
			core->overlay_camera_dynamic_offset = SUNDER_CAST2(u32)core->host_visible_vram_arena_suballocation_starting_offsets[LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_CVP_BUFFER_STARTING_INDEX + camera_index];
			SUNDER_SET_BIT(core->flags, LIGHTRAY_VULKAN_BITS_OVERLAY_CAMERA_BOUND_BIT, 1ull);

			return LIGHTRAY_VULKAN_RESULT_SUCCESS;
		}

		return LIGHTRAY_VULKAN_RESULT_FAILURE;
	}

	return LIGHTRAY_VULKAN_RESULT_FAILURE;
}

void lightray_vulkan_do_glyph_stuff(lightray_vulkan_core_t* core)
{
	cstring_literal* string = "-69";
	char buffer[10]{};
	char fbuffer[10]{};
	const i32 conversion_res = sunder_int_to_string(162, buffer, 10);
	const i32 fconversion_res = sunder_float_to_string(3.6, fbuffer, 2, 10);
	
	for (i32 i = 0; i < fconversion_res; i++)
	{
		SUNDER_LOG(fbuffer[i]);
	}

	const f64 scale = 32.0 / core->swapchain_info.imageExtent.height;

	const f64 atlas_width = core->overlay_core.font_atlas_buffer[0].width;
	const f64 atlas_height = core->overlay_core.font_atlas_buffer[0].height;

	f64 pen_x = 0.0;
	f64 pen_y = 0.0;
	f64 ascender = core->overlay_core.font_atlas_buffer[0].ascender;
	f64 descender = core->overlay_core.font_atlas_buffer[0].descender;
	f64 baseline = pen_y -	ascender * scale;

	for (u32 i = 0; i < 3; i++)
	{
		const u8 ch = fbuffer[i];
		const u32 glyph_index = ch;
		const lightray_glyph_t glyph = core->overlay_core.glyph_buffer[glyph_index];

		if (!glyph.has_bounds)
		{
			pen_x += glyph.advance * scale;
			continue;
		}

		const f64 width = (glyph.plane_bounds.right - glyph.plane_bounds.left) * scale;
		const f64 height = (glyph.plane_bounds.top - glyph.plane_bounds.bottom) * scale;

		SUNDER_LOG("\n\n");
		SUNDER_LOG("\nwidth: ");
		SUNDER_LOG(width);
		SUNDER_LOG("\nheight: ");
		SUNDER_LOG(height);

		f64 pos_x = pen_x + glyph.plane_bounds.left * scale;
		f64 pos_y = baseline + glyph.plane_bounds.bottom * scale;

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos_x, pos_y, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));

		const f64 u0 = glyph.atlas_bounds.left / atlas_width;
		const f64 v0 = (glyph.atlas_bounds.top / atlas_height);
		const f64 u1 = glyph.atlas_bounds.right / atlas_width;
		const f64 v1 = (glyph.atlas_bounds.bottom / atlas_height);

		core->overlay_core.render_instance_glyph_buffer[i].model_matrix = model;
		core->overlay_core.render_instance_glyph_buffer[i].uv_min = glm::vec2(u0, v0);
		core->overlay_core.render_instance_glyph_buffer[i].uv_max = glm::vec2(u1, v1);
		core->overlay_core.render_instance_glyph_buffer[i].color = glm::vec3(1.0f, 0.25f, 0.0f);

		pen_x += glyph.advance * scale;
	}

	// push updated data on the gpu
	sunder_buffer_copy_data_t render_instance_glyph_buffer_copy_data{};
	render_instance_glyph_buffer_copy_data.dst_size = core->host_visible_vram_arena.capacity;
	render_instance_glyph_buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[core->host_visible_vram_arena_render_instance_glyph_buffer_starting_index];
	render_instance_glyph_buffer_copy_data.src_size = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), 256);
	render_instance_glyph_buffer_copy_data.src_offset = 0;
	render_instance_glyph_buffer_copy_data.bytes_to_write = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_glyph_t), 256);
	const u64 render_instance_glyph_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->overlay_core.render_instance_glyph_buffer, &render_instance_glyph_buffer_copy_data);
}

bool lightray_vulkan_texture_sampler_metadata_exists(lightray_vulkan_texture_sampler_metadata_t* metadata_filter_buffer, u32 size, const lightray_vulkan_texture_sampler_metadata_t* metadata)
{
	for (u32 i = 0; i < size; i++)
	{
		if (metadata_filter_buffer[i].filter == metadata->filter && metadata_filter_buffer[i].address_mode == metadata->address_mode)
		{
			return true;
		}
	}

	return false;
}

void lightray_vulkan_push_overlay_text_elements(lightray_vulkan_core_t* core, const lightray_overlay_text_element_t* text_elements, u32 element_count)
{
	for (u32 i = 0; i < element_count; i++ )
	{
		core->overlay_core.text_element_buffer[i].type = text_elements[i].type;
		core->overlay_core.text_element_buffer[i].data = text_elements[i].data;
		core->overlay_core.text_element_buffer[i].element_count = text_elements[i].element_count;
		core->overlay_core.text_element_buffer[i].position = text_elements[i].position;
		core->overlay_core.text_element_buffer[i].glyph_size = text_elements[i].glyph_size;
		core->overlay_core.text_element_buffer[i].color = text_elements[i].color;
		core->overlay_core.text_element_buffer[i].precision = text_elements[i].precision;
	}

	core->overlay_core.text_element_count = element_count;
}

void lightray_vulkan_execute_post_render_pass_flush(lightray_vulkan_core_t* core)
{
	core->overlay_core.current_glyph_render_instance_count = 0;
}

f32 lightray_vulkan_normalize_width_f32(const lightray_vulkan_core_t* core, f32 pixels)
{
	return pixels / core->swapchain_info.imageExtent.width;
}

f32 lightray_vulkan_normalize_height_f32(const lightray_vulkan_core_t* core, f32 pixels)
{
	return pixels / core->swapchain_info.imageExtent.height;
}