#include "lightray_vulkan_core.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void lightray_vulkan_setup_core(lightray_vulkan_core_t* core, lightray_vulkan_game_resources_setup_data_t* game_resources_setup_data, u32 window_width, u32 window_height, cstring_literal* window_title, u32 target_fps, bool fullscreen_mode)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, SUNDER_TRUE);

	const u64 core_buffer_byte_count_written = sunder_init_buffer(core, sizeof(lightray_vulkan_core_t), 0, sizeof(lightray_vulkan_core_t));
			
	const std::filesystem::path project_root_path = lightray_get_project_root_path();
	const std::filesystem::path shaders_path = project_root_path / "shaders";
	const std::filesystem::path compiled_static_mesh_vertex_shader_path = shaders_path / "lightray_static_mesh_vertex_shader.vert.spv";
	const std::filesystem::path compiled_static_mesh_fragment_shader_path = shaders_path / "lightray_static_mesh_fragment_shader.frag.spv";

	const u64 compiled_static_mesh_vertex_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_vertex_shader_path);
	const u64 compiled_static_mesh_fragment_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_fragment_shader_path);

	core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size = compiled_static_mesh_vertex_shader_byte_code_size;
	core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size = compiled_static_mesh_fragment_shader_byte_code_size;

	core->window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

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

	core->win32_surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	core->win32_surface_info.hwnd = glfwGetWin32Window(core->window);
	core->win32_surface_info.hinstance = GetModuleHandle(nullptr);
	core->win32_surface_info.flags = 0;
	core->win32_surface_info.pNext = nullptr;

	const VkResult surface_creation_result = vkCreateWin32SurfaceKHR(core->instance, &core->win32_surface_info, nullptr, &core->surface);

	core->gpu_count = 1;
	const VkResult gpu_enumeration_result = vkEnumeratePhysicalDevices(core->instance, &core->gpu_count, core->gpus);
	vkGetPhysicalDeviceProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->gpu_properties);
	vkGetPhysicalDeviceFeatures(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->gpu_features);
	vkGetPhysicalDeviceMemoryProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->gpu_vram_properties);
	const VkResult surface_capabilities_retrieaval_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->surface_capabilities);

	vkGetPhysicalDeviceQueueFamilyProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->supported_queue_family_property_count, nullptr);
	vkGetPhysicalDeviceSurfaceFormatsKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_surface_format_count, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_swapchain_present_mode_count, nullptr);
	vkEnumerateDeviceExtensionProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], nullptr, &core->available_device_extension_count, nullptr);

	lightray_vulkan_get_supported_swapchain_image_count(core);

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

	sunder_arena_t import_asset_data_arena;
	const u32 mesh_count = game_resources_setup_data->mesh_count;
	const u64 import_asset_data_arena_per_buffer_allocation_size = sunder_compute_array_size_in_bytes(8, mesh_count);
	const u64 import_asset_data_arena_allocation_size = import_asset_data_arena_per_buffer_allocation_size * 3;
	const sunder_arena_result import_asset_data_arena_allocation_result = sunder_allocate_arena(&import_asset_data_arena, import_asset_data_arena_allocation_size, 8, SUNDER_ARENA_TYPE_STATIC);
	const u64 import_asset_data_arena_buffer_bytes_initialized = sunder_init_buffer(import_asset_data_arena.buffer, import_asset_data_arena.capacity, 0, import_asset_data_arena.capacity);

	Assimp::Importer* importers = nullptr;
	const aiScene** scenes = nullptr;
	const aiMesh** meshes = nullptr;

	sunder_arena_suballocation_result_t importers_suballocation_result = sunder_suballocate_from_arena_debug(&import_asset_data_arena, import_asset_data_arena_per_buffer_allocation_size, 8);
	importers = (Assimp::Importer*)importers_suballocation_result.data;

	sunder_arena_suballocation_result_t scenes_suballocation_result = sunder_suballocate_from_arena_debug(&import_asset_data_arena, import_asset_data_arena_per_buffer_allocation_size, 8);
	scenes = (const aiScene**)scenes_suballocation_result.data;

	sunder_arena_suballocation_result_t meshes_suballocation_result = sunder_suballocate_from_arena_debug(&import_asset_data_arena, import_asset_data_arena_per_buffer_allocation_size, 8);
	meshes = (const aiMesh**)meshes_suballocation_result.data;

	for (u32 i = 0; i < mesh_count; i++)
	{
		new(&importers[i]) Assimp::Importer();
	}

	for (u32 i = 0; i < mesh_count; i++)
	{
		scenes[i] = importers[i].ReadFile(game_resources_setup_data->static_mesh_metadata_buffer[i].path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_GenNormals);
		meshes[i] = scenes[i]->mMeshes[0];
	}

	u32 index_count_iter = 0;

	for (u32 i = 0; i  < mesh_count; i++)
	{
		index_count_iter = lightray_assimp_get_mesh_index_count(meshes[i]);

		SUNDER_LOG("\n");
		SUNDER_LOG(index_count_iter);

		total_index_count += index_count_iter;
		total_vertex_count += meshes[i]->mNumVertices;
		total_instance_model_count += game_resources_setup_data->static_mesh_metadata_buffer[i].instance_count;

		index_count_iter = 0;
	}

	const u64 general_purpose_ram_arena_proper_alignment = 8;

	const u64 cpu_side_vertex_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, general_purpose_ram_arena_proper_alignment);
	const u64 cpu_side_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, general_purpose_ram_arena_proper_alignment);
	const u64 cpu_side_instance_model_buffer_per_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_model_t), total_instance_model_count, general_purpose_ram_arena_proper_alignment);
	const u64 cpu_side_instance_model_allocation_size = sunder_align64(cpu_side_instance_model_buffer_per_buffer_allocation_size * 2, general_purpose_ram_arena_proper_alignment);

	const u64 device_local_vram_arena_vertex_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);
	const u64 device_local_vram_arena_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);
	const u64 device_local_vram_arena_allocation_size = sunder_align64(device_local_vram_arena_vertex_buffer_allocation_size + device_local_vram_arena_index_buffer_allocation_size, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);

	const u64 host_visible_vram_arena_vertex_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_instance_model_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_model_t), total_instance_model_count * 2, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_cvp_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_cvp_t), 1, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_staging_texture_buffer_allocation_size = 0;
	const u64 host_visible_vram_arena_allocation_size = sunder_align64(host_visible_vram_arena_vertex_buffer_allocation_size + host_visible_vram_arena_index_buffer_allocation_size + host_visible_vram_arena_instance_model_buffer_allocation_size + host_visible_vram_arena_cvp_allocation_size + host_visible_vram_arena_staging_texture_buffer_allocation_size, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);

	const u32 device_local_vram_arena_allocation_count = 2; // vertex buffer / index buffer
	const u32 host_visible_vram_arena_allocation_count = 5; // vertex buffer / index buffer / cvp / instance model buffer

	const u64 device_local_vram_arena_starting_offsets_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), device_local_vram_arena_allocation_count, general_purpose_ram_arena_proper_alignment);
	const u64 host_visible_vram_arena_starting_offsets_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), host_visible_vram_arena_allocation_count, general_purpose_ram_arena_proper_alignment);

	const u64 available_device_extensions_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkExtensionProperties), core->available_device_extension_count, general_purpose_ram_arena_proper_alignment);
	const u64 swapchain_presentation_supported_queue_family_indices_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), core->swapchain_presentation_supported_queue_family_index_count, general_purpose_ram_arena_proper_alignment);
	const u64 swapchain_images_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkImage), core->swapchain_image_in_use_count, general_purpose_ram_arena_proper_alignment);
	const u64 swapchain_image_related_data_allocation_size = sunder_compute_aligned_allocation_size(swapchain_images_allocation_size, 7, general_purpose_ram_arena_proper_alignment);
	const u64 shader_byte_code_combined_allocation_size = sunder_align64(compiled_static_mesh_vertex_shader_byte_code_size + compiled_static_mesh_fragment_shader_byte_code_size, general_purpose_ram_arena_proper_alignment);
	const u64 queue_family_properties_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkQueueFamilyProperties), core->supported_queue_family_property_count, general_purpose_ram_arena_proper_alignment);
	const u64 supported_surface_formats_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkSurfaceFormatKHR), core->supported_surface_format_count, general_purpose_ram_arena_proper_alignment); 
	const u64 supported_swapchain_present_modes_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkPresentModeKHR), core->supported_swapchain_present_mode_count, general_purpose_ram_arena_proper_alignment);

	const u64 mesh_render_pass_data_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_render_pass_data_t), mesh_count, general_purpose_ram_arena_proper_alignment);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	lightray_vulkan_vram_arena_allocation_data_t host_visible_vram_arena_allocation_data{};
	host_visible_vram_arena_allocation_data.allocation_size = host_visible_vram_arena_allocation_size;
	host_visible_vram_arena_allocation_data.usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	host_visible_vram_arena_allocation_data.metadata_arena = &core->general_purpose_ram_arena;
	host_visible_vram_arena_allocation_data.suballocation_count = host_visible_vram_arena_allocation_count;
	host_visible_vram_arena_allocation_data.vram_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	const u64 host_visible_vram_arena_metadata_allocation_size = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(&host_visible_vram_arena_allocation_data);

	host_visible_vram_arena_allocation_data.metadata_allocation_size = (u32)host_visible_vram_arena_metadata_allocation_size;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	lightray_vulkan_vram_arena_allocation_data_t device_local_vram_arena_allocation_data{};
	device_local_vram_arena_allocation_data.allocation_size = device_local_vram_arena_allocation_size;
	device_local_vram_arena_allocation_data.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	device_local_vram_arena_allocation_data.metadata_arena = &core->general_purpose_ram_arena;
	device_local_vram_arena_allocation_data.suballocation_count = device_local_vram_arena_allocation_count;
	device_local_vram_arena_allocation_data.vram_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	const u64 device_local_vram_arena_metadata_allocation_size = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(&device_local_vram_arena_allocation_data);

	device_local_vram_arena_allocation_data.metadata_allocation_size = (u32)device_local_vram_arena_metadata_allocation_size;
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const u64 general_purpose_ram_arena_allocation_size = sunder_align64(available_device_extensions_allocation_size + swapchain_presentation_supported_queue_family_indices_allocation_size + swapchain_image_related_data_allocation_size + shader_byte_code_combined_allocation_size + queue_family_properties_allocation_size + supported_surface_formats_allocation_size + supported_swapchain_present_modes_allocation_size + cpu_side_vertex_buffer_allocation_size + cpu_side_index_buffer_allocation_size + cpu_side_instance_model_allocation_size + mesh_render_pass_data_buffer_allocation_size + device_local_vram_arena_starting_offsets_allocation_size + host_visible_vram_arena_starting_offsets_allocation_size + device_local_vram_arena_metadata_allocation_size + host_visible_vram_arena_metadata_allocation_size, general_purpose_ram_arena_proper_alignment);

	const sunder_arena_result general_purpose_ram_arena_allocation_result = sunder_allocate_arena(&core->general_purpose_ram_arena, general_purpose_ram_arena_allocation_size, general_purpose_ram_arena_proper_alignment, SUNDER_ARENA_TYPE_STATIC);
	const u64 general_purpose_ram_arena_buffer_bytes_initialized = sunder_init_buffer(core->general_purpose_ram_arena.buffer, core->general_purpose_ram_arena.capacity, 0, core->general_purpose_ram_arena.capacity);
	
	const sunder_arena_suballocation_result_t available_device_extensions_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkExtensionProperties), core->available_device_extension_count), alignof(VkExtensionProperties));
	core->available_device_extensions  = (VkExtensionProperties*)available_device_extensions_suballocation_result.data;

	const sunder_arena_suballocation_result_t  swapchain_presentation_supported_queue_family_indices_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->swapchain_presentation_supported_queue_family_index_count), alignof(u32));
	core->swapchain_presentation_supported_queue_family_indices = (u32*)swapchain_presentation_supported_queue_family_indices_suballocation_result.data;

	const sunder_arena_suballocation_result_t swapchain_images_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkImage), core->swapchain_image_in_use_count), alignof(VkImage));
	core->swapchain_images = (VkImage*)swapchain_images_suballocation_result.data;

	const sunder_arena_suballocation_result_t swapchain_image_views_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkImageView), core->swapchain_image_in_use_count), alignof(VkImageView));
	core->swapchain_image_views = (VkImageView*)swapchain_image_views_suballocation_result.data;

	const sunder_arena_suballocation_result_t swapchain_framebuffers_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkFramebuffer), core->swapchain_image_in_use_count), alignof(VkFramebuffer));
	core->swapchain_framebuffers = (VkFramebuffer*)swapchain_framebuffers_suballocation_result.data;

	const sunder_arena_suballocation_result_t image_finished_rendering_semaphores_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkSemaphore), core->swapchain_image_in_use_count), alignof(VkSemaphore));
	core->image_finished_rendering_semaphores = (VkSemaphore*)image_finished_rendering_semaphores_suballocation_result.data;

	const sunder_arena_suballocation_result_t image_available_for_rendering_semaphores_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkSemaphore), core->swapchain_image_in_use_count), alignof(VkSemaphore));
	core->image_available_for_rendering_semaphores = (VkSemaphore*)image_available_for_rendering_semaphores_suballocation_result.data;

	const sunder_arena_suballocation_result_t inflight_fences_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkFence), core->swapchain_image_in_use_count), alignof(VkFence));
	core->inflight_fences = (VkFence*)inflight_fences_suballocation_result.data;

	const sunder_arena_suballocation_result_t render_command_buffers_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkCommandBuffer), core->swapchain_image_in_use_count), alignof(VkCommandBuffer));
	core->render_command_buffers = (VkCommandBuffer*)render_command_buffers_suballocation_result.data;

	const sunder_arena_suballocation_result_t supported_queue_family_properties_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkQueueFamilyProperties), core->supported_queue_family_property_count), alignof(VkQueueFamilyProperties));
	core->supported_queue_family_properties = (VkQueueFamilyProperties*)supported_queue_family_properties_suballocation_result.data;

	const sunder_arena_suballocation_result_t supported_surface_formats_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkSurfaceFormatKHR), core->supported_surface_format_count), alignof(VkSurfaceFormatKHR));
	core->supported_surface_formats = (VkSurfaceFormatKHR*)supported_surface_formats_suballocation_result.data;

	const sunder_arena_suballocation_result_t swapchain_present_modes_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkPresentModeKHR), core->supported_swapchain_present_mode_count), alignof(VkPresentModeKHR));
	core->supported_swapchain_present_modes = (VkPresentModeKHR*)swapchain_present_modes_suballocation_result.data;

	const sunder_arena_suballocation_result_t static_mesh_vertex_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_static_mesh_vertex_shader_byte_code_size, alignof(i8));
	core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code = (i8*)static_mesh_vertex_shader_byte_code_suballocation_result.data;

	const sunder_arena_suballocation_result_t static_mesh_fragment_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_static_mesh_fragment_shader_byte_code_size, alignof(i8));
	core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code = (i8*)static_mesh_fragment_shader_byte_code_suballocation_result.data;

	const sunder_arena_suballocation_result_t cpu_side_vertex_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count), alignof(lightray_vertex_t));
	core->cpu_side_vertex_buffer = (lightray_vertex_t*)cpu_side_vertex_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t cpu_side_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count), alignof(u32));
	core->cpu_side_index_buffer = (u32*)cpu_side_index_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t instance_model_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_model_t), total_instance_model_count), alignof(lightray_model_t));
	core->cpu_side_instance_model_buffer = (lightray_model_t*)instance_model_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t hidden_instance_model_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_model_t), total_instance_model_count), alignof(lightray_model_t));
	core->cpu_side_hidden_instance_model_buffer = (lightray_model_t*)hidden_instance_model_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_render_pass_data_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_render_pass_data_t), mesh_count), alignof(lightray_mesh_render_pass_data_t));
	core->mesh_render_pass_data_buffer = (lightray_mesh_render_pass_data_t*)mesh_render_pass_data_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t device_local_vram_arena_suballocation_starting_offsetss_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u64), device_local_vram_arena_allocation_count), alignof(u64));
	core->device_local_vram_arena_suballocation_starting_offsets = (u64*)device_local_vram_arena_suballocation_starting_offsetss_suballocation_result.data;

	const sunder_arena_suballocation_result_t host_visible_vram_arena_suballocation_starting_offsetss_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u64), host_visible_vram_arena_allocation_count), alignof(u64));
	core->host_visible_vram_arena_suballocation_starting_offsets = (u64*)host_visible_vram_arena_suballocation_starting_offsetss_suballocation_result.data;

	vkGetPhysicalDeviceQueueFamilyProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->supported_queue_family_property_count, core->supported_queue_family_properties);
	const VkResult available_device_extensions_enumeration_result = vkEnumerateDeviceExtensionProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], nullptr, &core->available_device_extension_count, core->available_device_extensions);
	const VkResult surface_formats_retrieval_result = vkGetPhysicalDeviceSurfaceFormatsKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_surface_format_count, core->supported_surface_formats);
	const VkResult swapchain_present_modes_retrieval_result = vkGetPhysicalDeviceSurfacePresentModesKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_swapchain_present_mode_count, core->supported_swapchain_present_modes);

	for (u32 i = 0; i < core->supported_surface_format_count; i++)
	{
		if (core->supported_surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && core->supported_surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			core->chosen_surface_format= core->supported_surface_formats[i];
		}
	}

	for (u32 i = 0; i < core->supported_swapchain_present_mode_count; i++)
	{
		if (core->supported_swapchain_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			SUNDER_SET_NTH_BIT(core->flags, LIGHTRAY_BITS_PRESENT_MODE_MAILBOX_SUPPORTED_BIT);
		}

		else if (core->supported_swapchain_present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
		{
			SUNDER_SET_NTH_BIT(core->flags, LIGHTRAY_BITS_PRESENT_MODE_FIFO_SUPPORTED_BIT);
		}

		else if (core->supported_swapchain_present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			SUNDER_SET_NTH_BIT(core->flags, LIGHTRAY_BITS_PRESENT_MODE_IMMEDIATE_SUPPORTED_BIT);
		}
	}

	if (SUNDER_IS_NTH_BIT_SET(core->flags, LIGHTRAY_BITS_PRESENT_MODE_MAILBOX_SUPPORTED_BIT) && core->swapchain_image_in_use_count == 3 || core->swapchain_image_in_use_count == 2)
	{
		core->chosen_swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
	}

	else if (SUNDER_IS_NTH_BIT_SET(core->flags, LIGHTRAY_BITS_PRESENT_MODE_IMMEDIATE_SUPPORTED_BIT) && core->chosen_swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR)
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

	const u32 graphics_queue_family_index = lightray_vulkan_get_graphics_queue_family_index(core);
	const u32 present_queue_family_index = lightray_vulkan_get_present_queue_family_index(core);

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

	const lightray_vulkan_vram_arena_result host_visible_vram_arena_allocation_result = lightray_vulkan_allocate_vram_arena_debug(core, &core->host_visible_vram_arena, &host_visible_vram_arena_allocation_data);
	const lightray_vulkan_vram_arena_result device_local_vram_arena_allocation_result = lightray_vulkan_allocate_vram_arena_debug(core, &core->device_local_vram_arena, &device_local_vram_arena_allocation_data);

	lightray_vulkan_create_swapchain(core);

	lightray_get_shader_byte_code(compiled_static_mesh_vertex_shader_path, core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size, 600'000ULL);
	lightray_get_shader_byte_code(compiled_static_mesh_fragment_shader_path, core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size, 600'000ULL);

	const lightray_vulkan_shader_result static_mesh_vertex_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX], VK_SHADER_STAGE_VERTEX_BIT);
	const lightray_vulkan_shader_result static_mesh_fragment_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX], VK_SHADER_STAGE_FRAGMENT_BIT);




	core->pipeline_shader_stage_infos[0] = core->shaders[0].stage_info;
	core->pipeline_shader_stage_infos[1] = core->shaders[1].stage_info;

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
	core->vertex_input_binding_descriptions[1].stride = sizeof(lightray_model_t);

	core->vertex_input_attribute_descriptions[0].binding = 0;
	core->vertex_input_attribute_descriptions[0].location = 0;
	core->vertex_input_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->vertex_input_attribute_descriptions[0].offset = offsetof(lightray_vertex_t, position);

	core->vertex_input_attribute_descriptions[1].binding = 0;
	core->vertex_input_attribute_descriptions[1].location = 1;
	core->vertex_input_attribute_descriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	core->vertex_input_attribute_descriptions[1].offset = offsetof(lightray_vertex_t, normal);

	core->vertex_input_attribute_descriptions[2].binding = 0;
	core->vertex_input_attribute_descriptions[2].location = 2;
	core->vertex_input_attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	core->vertex_input_attribute_descriptions[2].offset = offsetof(lightray_vertex_t, texture_coordinates);

	core->vertex_input_attribute_descriptions[3].binding = 0;
	core->vertex_input_attribute_descriptions[3].location = 3;
	core->vertex_input_attribute_descriptions[3].format = VK_FORMAT_R32_SFLOAT;
	core->vertex_input_attribute_descriptions[3].offset = offsetof(lightray_vertex_t, alpha);

	core->vertex_input_attribute_descriptions[4].binding = 1;
	core->vertex_input_attribute_descriptions[4].location = 4;
	core->vertex_input_attribute_descriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->vertex_input_attribute_descriptions[4].offset = 0;

	core->vertex_input_attribute_descriptions[5].binding = 1;
	core->vertex_input_attribute_descriptions[5].location = 5;
	core->vertex_input_attribute_descriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->vertex_input_attribute_descriptions[5].offset = sizeof(glm::vec4);

	core->vertex_input_attribute_descriptions[6].binding = 1;
	core->vertex_input_attribute_descriptions[6].location = 6;
	core->vertex_input_attribute_descriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->vertex_input_attribute_descriptions[6].offset = sizeof(glm::vec4) * 2;

	core->vertex_input_attribute_descriptions[7].binding = 1;
	core->vertex_input_attribute_descriptions[7].location = 7;
	core->vertex_input_attribute_descriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	core->vertex_input_attribute_descriptions[7].offset = sizeof(glm::vec4) * 3;

	core->pipeline_vertex_input_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	core->pipeline_vertex_input_state_info.pVertexBindingDescriptions = core->vertex_input_binding_descriptions;
	core->pipeline_vertex_input_state_info.vertexBindingDescriptionCount = 2;
	core->pipeline_vertex_input_state_info.pVertexAttributeDescriptions = core->vertex_input_attribute_descriptions;
	core->pipeline_vertex_input_state_info.vertexAttributeDescriptionCount = 8;

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

	core->pipeline_viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	core->pipeline_viewport_state_info.pViewports = &core->viewport;
	core->pipeline_viewport_state_info.viewportCount = 1;
	core->pipeline_viewport_state_info.pScissors = &core->scissor;
	core->pipeline_viewport_state_info.scissorCount = 1;
	core->pipeline_viewport_state_info.flags = 0;
	core->pipeline_viewport_state_info.pNext = nullptr;

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

	core->pipeline_multisample_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	core->pipeline_multisample_state_info.sampleShadingEnable = VK_FALSE;
	core->pipeline_multisample_state_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	core->pipeline_multisample_state_info.minSampleShading = 1.0f;
	core->pipeline_multisample_state_info.pSampleMask = nullptr;
	core->pipeline_multisample_state_info.alphaToCoverageEnable = VK_TRUE;
	core->pipeline_multisample_state_info.alphaToOneEnable = VK_FALSE;
	core->pipeline_multisample_state_info.flags = 0;
	core->pipeline_multisample_state_info.pNext = nullptr;

	core->pipeline_color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	core->pipeline_color_blend_attachment_state.blendEnable = VK_TRUE;
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

	core->pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	core->pipeline_layout_info.pSetLayouts = nullptr;
	core->pipeline_layout_info.setLayoutCount = 0;
	core->pipeline_layout_info.pushConstantRangeCount = 0;
	core->pipeline_layout_info.pPushConstantRanges = nullptr;
	core->pipeline_layout_info.flags = 0;
	core->pipeline_layout_info.pNext = nullptr;

	const VkResult  pipeline_layout_creation_result =  vkCreatePipelineLayout(core->logical_device, &core->pipeline_layout_info, nullptr, &core->pipeline_layout);

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
	core->subpass_description.pDepthStencilAttachment = nullptr;
	core->subpass_description.inputAttachmentCount = 0;
	core->subpass_description.pInputAttachments = nullptr;
	core->subpass_description.pPreserveAttachments = nullptr;
	core->subpass_description.preserveAttachmentCount = 0;
	core->subpass_description.pResolveAttachments = nullptr;
	core->subpass_description.flags = 0;

	core->subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	core->subpass_dependency.dstSubpass = 0;
	core->subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	core->subpass_dependency.srcAccessMask = 0;
	core->subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	core->subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	core->render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	core->render_pass_info.attachmentCount = 1;
	core->render_pass_info.pAttachments = &core->attachment_description;
	core->render_pass_info.subpassCount = 1;
	core->render_pass_info.pSubpasses = &core->subpass_description;
	core->render_pass_info.dependencyCount = 1;
	core->render_pass_info.pDependencies = &core->subpass_dependency;
	core->render_pass_info.flags = 0;
	core->render_pass_info.pNext = nullptr;

	const VkResult render_pass_creation_result = vkCreateRenderPass(core->logical_device, &core->render_pass_info, nullptr, &core->render_pass);

	core->graphics_pipeline_infos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[0].stageCount = 2;
	core->graphics_pipeline_infos[0].pStages = core->pipeline_shader_stage_infos;
	core->graphics_pipeline_infos[0].pVertexInputState = &core->pipeline_vertex_input_state_info;
	core->graphics_pipeline_infos[0].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[0].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[0].pRasterizationState = &core->pipeline_rasterization_state_infos[0];
	core->graphics_pipeline_infos[0].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[0].pDepthStencilState = nullptr;
	core->graphics_pipeline_infos[0].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[0].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[0].layout = core->pipeline_layout;
	core->graphics_pipeline_infos[0].renderPass = core->render_pass;
	core->graphics_pipeline_infos[0].subpass = 0;
	core->graphics_pipeline_infos[0].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[0].basePipelineIndex = -1;
	core->graphics_pipeline_infos[0].flags = 0;
	core->graphics_pipeline_infos[0].pNext = nullptr;

	core->graphics_pipeline_infos[1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[1].stageCount = 2;
	core->graphics_pipeline_infos[1].pStages = core->pipeline_shader_stage_infos;
	core->graphics_pipeline_infos[1].pVertexInputState = &core->pipeline_vertex_input_state_info;
	core->graphics_pipeline_infos[1].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[1].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[1].pRasterizationState = &core->pipeline_rasterization_state_infos[1];
	core->graphics_pipeline_infos[1].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[1].pDepthStencilState = nullptr;
	core->graphics_pipeline_infos[1].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[1].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[1].layout = core->pipeline_layout;
	core->graphics_pipeline_infos[1].renderPass = core->render_pass;
	core->graphics_pipeline_infos[1].subpass = 0;
	core->graphics_pipeline_infos[1].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[1].basePipelineIndex = -1;
	core->graphics_pipeline_infos[1].flags = 0;
	core->graphics_pipeline_infos[1].pNext = nullptr;

	const VkResult graphics_pipelines_creation_result = vkCreateGraphicsPipelines(core->logical_device, nullptr, 2, core->graphics_pipeline_infos, nullptr, core->pipelines);







	for (u32 i = 0; i < game_resources_setup_data->mesh_count; i++)
	{
		importers[i].~Importer();
	}

	sunder_free_arena(&import_asset_data_arena);
}

void lightray_vulkan_terminate_core(lightray_vulkan_core_t* core)
{
	lightray_vulkan_free_vram_arena(core, &core->host_visible_vram_arena);
	lightray_vulkan_free_vram_arena(core, &core->device_local_vram_arena);

	for (u32 i = 0; i < 2; i++)
	{
		lightray_vulkan_free_shader(core, &core->shaders[i]);
		vkDestroyPipeline(core->logical_device, core->pipelines[i], nullptr);
	}

	vkDestroyRenderPass(core->logical_device, core->render_pass, nullptr);
	vkDestroyPipelineLayout(core->logical_device, core->pipeline_layout, nullptr);
	lightray_vulkan_free_swapchain(core);
	vkDestroySurfaceKHR(core->instance, core->surface, nullptr);
	vkDestroyDevice(core->logical_device, nullptr);
	vkDestroyInstance(core->instance, nullptr);

	glfwDestroyWindow(core->window);
	glfwTerminate();
	sunder_free_arena(&core->general_purpose_ram_arena);
}

void lightray_vulkan_get_supported_swapchain_image_count(lightray_vulkan_core_t* core)
{
	const u32 max_supported_swapchain_image_count = core->surface_capabilities.maxImageCount;

	if (max_supported_swapchain_image_count == 0)
	{
		core->swapchain_image_in_use_count = 3;
	}

	else if (max_supported_swapchain_image_count > 2)
	{
		core->swapchain_image_in_use_count = 3;
	}

	else if (max_supported_swapchain_image_count == 2)
	{
		core->swapchain_image_in_use_count = 2;
	}

	else
	{
		core->swapchain_image_in_use_count = 1;
	}
}

bool lightray_vulkan_is_fps_capped(const lightray_vulkan_core_t* core)
{
	return SUNDER_IS_NTH_BIT_SET(core->flags, LIGHTRAY_BITS_IS_FPS_CAPPED);
}

void lightray_vulkan_set_target_fps(lightray_vulkan_core_t* core, f32 desired_fps)
{
	if (!lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_SET_NTH_BIT(core->flags, LIGHTRAY_BITS_IS_FPS_CAPPED);
	}

	lightray_set_target_fps(desired_fps, &core->frame_duration_s, &core->fps);
}

void lightray_vulkan_uncap_fps(lightray_vulkan_core_t* core)
{
	if (lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_ZERO_NTH_BIT(core->flags, LIGHTRAY_BITS_IS_FPS_CAPPED);
	}
}

u64 lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(const lightray_vulkan_vram_arena_allocation_data_t* allocation_data)
{
	return sunder_compute_aligned_allocation_size(sizeof(u64), allocation_data->suballocation_count, 8);
}

u32 lightray_vulkan_get_graphics_queue_family_index(const lightray_vulkan_core_t* core)
{
	for (u32 i = 0; i < core->swapchain_presentation_supported_queue_family_index_count; i++)
	{
		if (core->supported_queue_family_properties[core->swapchain_presentation_supported_queue_family_indices[i]].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			return core->swapchain_presentation_supported_queue_family_indices[i];
		}
	}

	return UINT32_MAX;
}

u32 lightray_vulkan_get_present_queue_family_index(const lightray_vulkan_core_t* core)
{
	const u32 graphics_queue_index = lightray_vulkan_get_graphics_queue_family_index(core);

	for (u32 i = 0; i < core->swapchain_presentation_supported_queue_family_index_count; i++)
	{
		if (core->swapchain_presentation_supported_queue_family_indices[i] != graphics_queue_index)
		{
			return core->swapchain_presentation_supported_queue_family_indices[i];
		}
	}

	return UINT32_MAX;
}

u32 lightray_vulkan_get_vram_type_index(const lightray_vulkan_core_t* core, u32 vram_filter, VkMemoryPropertyFlags vram_property_flags)
{
	const u32 vram_type_count = core->gpu_vram_properties.memoryTypeCount;

	for (u32 i = 0; i < vram_type_count; i++)
	{
		if (SUNDER_IS_NTH_BIT_SET(vram_filter, i) && (core->gpu_vram_properties.memoryTypes[i].propertyFlags & vram_property_flags) == vram_property_flags)
		{
			return i;
		}
	}

	return UINT32_MAX;
}

lightray_vulkan_vram_arena_result lightray_vulkan_allocate_vram_arena_debug(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena, const lightray_vulkan_vram_arena_allocation_data_t* allocation_data)
{
	if (allocation_data->suballocation_count == 0) { LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_SUBALLOCATION_COUNT_IS_0; }
	if (allocation_data->metadata_allocation_size < sizeof(u64)) { LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_INVALID_METADATA_ALLOCATION_SIZE; }
	if (allocation_data->allocation_size == 0) { LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_ARENA_ALLOCATION_SIZE_IS_0; }

	const sunder_arena_suballocation_result_t suballocation_offset_buffer_suballocation_result = sunder_suballocate_from_arena_debug(allocation_data->metadata_arena, allocation_data->metadata_allocation_size, 8);
	arena->suballocation_offset_buffer = (u64*)suballocation_offset_buffer_suballocation_result.data;

	VkBufferCreateInfo vram_buffer_description_create_info{};
	vram_buffer_description_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vram_buffer_description_create_info.size = allocation_data->allocation_size;
	vram_buffer_description_create_info.usage = allocation_data->usage_flags;
	vram_buffer_description_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vram_buffer_description_create_info.flags = 0;
	vram_buffer_description_create_info.pQueueFamilyIndices = nullptr;
	vram_buffer_description_create_info.queueFamilyIndexCount = 0;
	vram_buffer_description_create_info.pNext = nullptr;

	const VkResult vram_buffer_decription_creation_result = vkCreateBuffer(core->logical_device, &vram_buffer_description_create_info, nullptr, &arena->vram_buffer_description);
	if (vram_buffer_decription_creation_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_FAILED_TO_CREATE_DESCRIPTOR_BUFFER; }

	VkMemoryRequirements vram_requirements{};
	vkGetBufferMemoryRequirements(core->logical_device, arena->vram_buffer_description, &vram_requirements);

	VkMemoryAllocateInfo vram_buffer_allocation_info{};
	vram_buffer_allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vram_buffer_allocation_info.allocationSize = vram_requirements.size;
	vram_buffer_allocation_info.memoryTypeIndex = lightray_vulkan_get_vram_type_index(core, vram_requirements.memoryTypeBits, allocation_data->vram_properties);
	vram_buffer_allocation_info.pNext = nullptr;

	const VkResult vram_buffer_allocation_result = vkAllocateMemory(core->logical_device, &vram_buffer_allocation_info, nullptr, &arena->vram_buffer);
	if (vram_buffer_allocation_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_OUT_OF_VRAM; }

	const VkResult vram_buffer_description_binding_result = vkBindBufferMemory(core->logical_device, arena->vram_buffer_description, arena->vram_buffer, 0);
	if (vram_buffer_description_binding_result != VK_SUCCESS) { return LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_FAILED_TO_BIND_BUFFER; }

	SUNDER_LOG("\n[vram arena allocation] allocated size: ");
	SUNDER_LOG(vram_buffer_allocation_info.allocationSize);
	SUNDER_LOG("\n[vram arena allocation] required alignment: ");
	SUNDER_LOG(vram_requirements.alignment);
	SUNDER_LOG("\n[vram arena allocation] required vram type index: ");
	SUNDER_LOG(vram_buffer_allocation_info.memoryTypeIndex);
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

	SUNDER_LOG("\n");
		
	arena->vram_type_index = vram_buffer_allocation_info.memoryTypeIndex;
	arena->vram_buffer_alignment = vram_requirements.alignment;
	arena->vram_buffer_capacity = vram_buffer_allocation_info.allocationSize;
	arena->vram_buffer_current_offset = 0;
	arena->max_suballocation_count = allocation_data->suballocation_count;
	arena->suballocation_total_count = 0;
	arena->suballocation_total_size_in_bytes = 0;
	arena->vram_property_flags = allocation_data->vram_properties;

	return LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_SUCCESS;
}

lightray_vulkan_vram_arena_suballocation_result_t lightray_vulkan_suballocate_from_vram_arena_debug(lightray_vulkan_vram_arena_t* arena, u64 suballocation_size)
{
	lightray_vulkan_vram_arena_suballocation_result_t res;
	res.result = LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_UNKOWN_FAILURE;
	res.starting_offset = UINT64_MAX;
	
	res.result = LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_VRAM_BUFFER_IS_NULLPTR;
	if (arena->vram_buffer == nullptr) { return res; }
	res.result = LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_VRAM_BUFFER_DESCRIPTION_IS_NULLPTR;
	if (arena->vram_buffer_description == nullptr) { return res; }

	const u64 aligned_offset = sunder_align64(arena->vram_buffer_current_offset, arena->vram_buffer_alignment);
	const u64 bytes_of_padding = aligned_offset - arena->vram_buffer_current_offset;
	res.result = LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_OUT_OF_VRAM;

	if (aligned_offset > arena->vram_buffer_capacity) { return res; }
	if (aligned_offset + suballocation_size > arena->vram_buffer_capacity) { return res; }

	SUNDER_LOG("\n[vram arena suballocation] suballocation offset buffer:\n");
	for (u32 i = 0; i < arena->suballocation_total_count; i++)
	{
		SUNDER_LOG(arena->suballocation_offset_buffer);
		SUNDER_LOG(" ");
	}

	SUNDER_LOG("\n[vram arena suballocation] alignment: ");
	SUNDER_LOG(arena->vram_buffer_alignment);
	SUNDER_LOG("\n[vram arena suballocation] requested suballocation size: ");
	SUNDER_LOG(suballocation_size);
	SUNDER_LOG("\n[vram arena suballocation] current offset: ");
	SUNDER_LOG(arena->vram_buffer_current_offset);

	SUNDER_LOG("\n[vram arena suballocation] aligned offset: ");
	SUNDER_LOG(aligned_offset);

	SUNDER_LOG("\n[vram arena suballocation] bytes of padding added: ");
	SUNDER_LOG(bytes_of_padding);
	
	res.result = LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_SUCCESS;
	res.starting_offset = aligned_offset;

	SUNDER_LOG("\n[vram arena suballocation] suballocated at offset : ");
	SUNDER_LOG(aligned_offset);

	const u64 post_suballocation_offset = aligned_offset + suballocation_size;
	SUNDER_LOG("\n[vram arena suballocation] post suballocation offset: ");
	SUNDER_LOG(post_suballocation_offset);

	arena->vram_buffer_current_offset = aligned_offset;
	arena->suballocation_total_size_in_bytes += bytes_of_padding;
	arena->suballocation_offset_buffer[arena->suballocation_total_count] = aligned_offset;
	arena->suballocation_total_count++;
	
	SUNDER_LOG("\n");

	return res;
}

lightray_vulkan_vram_arena_result lightray_vulkan_free_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena)
{
	if (arena->vram_buffer == nullptr) { return LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_VRAM_BUFFER_IS_NULLPTR; }
	if (arena->vram_buffer_description == nullptr) { return LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_VRAM_BUFFER_DESCRIPTION_IS_NULLPTR; }

	vkDestroyBuffer(core->logical_device, arena->vram_buffer_description, nullptr);
	vkFreeMemory(core->logical_device, arena->vram_buffer, nullptr);

	arena->max_suballocation_count = 0;
	arena->suballocation_total_count = 0;
	arena->suballocation_total_size_in_bytes = 0;
	arena->vram_buffer_alignment = 0;
	arena->vram_buffer_capacity = 0;
	arena->vram_buffer_current_offset = 0;
	arena->vram_property_flags = 0;
	arena->vram_type_index = 0;

	return LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_SUCCESS;
}

void lightray_vulkan_create_swapchain(lightray_vulkan_core_t* core)
{
	lightray_vulkan_get_supported_swapchain_image_count(core);

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
	lightray_vulkan_get_supported_swapchain_image_count(core);

	for (u32 i = 0; i < core->swapchain_image_in_use_count; i++)
	{
		vkDestroyImageView(core->logical_device, core->swapchain_image_views[i], nullptr);
	}

	vkDestroySwapchainKHR(core->logical_device, core->swapchain, nullptr);
}

lightray_vulkan_shader_result lightray_vulkan_create_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader, VkShaderStageFlagBits stage_flags)
{
	if (stage_flags == 0) { return LIGHTRAY_VULKAN_SHADER_RESULT_INVALID_SHADER_STAGE_FLAGS; }
	if (shader->byte_code == nullptr) { return LIGHTRAY_VULKAN_SHADER_RESULT_UNINITIALIZED_BYTE_CODE_BUFFER; }
	if (shader->byte_code_size == 0) { return LIGHTRAY_VULKAN_SHADER_RESULT_INVALID_BYTE_CODE_SIZE; }

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

	return LIGHTRAY_VULKAN_SHADER_RESULT_SUCCESS;
}

lightray_vulkan_shader_result lightray_vulkan_free_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader)
{
	if (shader->module == nullptr) { return LIGHTRAY_VULKAN_SHADER_RESULT_UNINITIALIZED_SHADER_MODULE; }

	vkDestroyShaderModule(core->logical_device, shader->module, nullptr);
	shader->byte_code_size = 0;

	return LIGHTRAY_VULKAN_SHADER_RESULT_SUCCESS;
}

void lightray_vulkan_create_cpu_side_texture(cstring_literal* path, lightray_vulkan_texture_t* texture, VkImageType type, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage_flags)
{

}

void lightray_vulkan_free_cpu_side_texture(lightray_vulkan_texture_t* texture)
{
	stbi_image_free(texture->buffer);
	texture->format = VK_FORMAT_UNDEFINED;
	texture->type = VK_IMAGE_TYPE_MAX_ENUM;
	texture->tiling = VK_IMAGE_TILING_OPTIMAL;
	texture->width = 0;
	texture->height = 0;
	texture->usage_flags = 0;
	texture->vram_type_index = 0;
}