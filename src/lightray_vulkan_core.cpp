#include "lightray_vulkan_core.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void lightray_vulkan_setup_core(lightray_vulkan_core_t* core, lightray_vulkan_core_setup_data_t* setup_data)
{
	sunder_initialize_time();
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, SUNDER_TRUE);

	core->initial_time = (f32)sunder_get_elapsed_time_in_seconds();

	u32 supported_vulkan_api_version = VK_API_VERSION_1_0;
	const VkResult vulkan_api_version_query_result = vkEnumerateInstanceVersion(&supported_vulkan_api_version);

	const u32 vulkan_api_version_major = VK_API_VERSION_MAJOR(supported_vulkan_api_version);
	const u32 vulkan_api_version_minor = VK_API_VERSION_MINOR(supported_vulkan_api_version);
	const u32 vulkan_api_version_patch = VK_API_VERSION_PATCH(supported_vulkan_api_version);

	cstring_literal* compiled_static_mesh_vertex_shader_path = "shaders/lightray_static_mesh_vertex_shader.vert.spv";
	cstring_literal* compiled_static_mesh_fragment_shader_path = "shaders/lightray_static_mesh_fragment_shader.frag.spv";
	cstring_literal* compiled_textured_static_mesh_vertex_shader_path = "shaders/lightray_textured_static_mesh_vertex_shader.vert.spv";
	cstring_literal* compiled_textured_static_mesh_fragment_shader_path = "shaders/lightray_textured_static_mesh_fragment_shader.frag.spv";

	const u64 compiled_static_mesh_vertex_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_vertex_shader_path);
	const u64 compiled_static_mesh_fragment_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_fragment_shader_path);
	const u64 compiled_textured_static_mesh_vertex_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_textured_static_mesh_vertex_shader_path);
	const u64 compiled_textured_static_mesh_fragment_shader_byte_code_size = lightray_get_shader_byte_code_size(compiled_textured_static_mesh_fragment_shader_path);

	core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size = compiled_static_mesh_vertex_shader_byte_code_size;
	core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size = compiled_static_mesh_fragment_shader_byte_code_size;
	core->shaders[2].byte_code_size = compiled_textured_static_mesh_vertex_shader_byte_code_size;
	core->shaders[3].byte_code_size = compiled_textured_static_mesh_fragment_shader_byte_code_size;

	core->monitor = glfwGetPrimaryMonitor();
	core->video_mode = glfwGetVideoMode(core->monitor);

	if (setup_data->fullscreen_mode)
	{
		core->window = glfwCreateWindow(core->video_mode->width, core->video_mode->height, setup_data->window_title, core->monitor, nullptr);
	}

	else
	{
		core->window = glfwCreateWindow(setup_data->window_width, setup_data->window_height, setup_data->window_title, nullptr, nullptr);
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

	core->texture_count = setup_data->texture_count;
	const u32 mesh_count = setup_data->mesh_count;
	core->mesh_count = mesh_count;
	const u64 mesh_import_data_per_buffer_allocation_size = sunder_compute_array_size_in_bytes(8, mesh_count);

	const u32 device_local_vram_arena_allocation_count = 2; // vertex buffer / index buffer
	const u32 host_visible_vram_arena_allocation_count = 4u + core->texture_count; // vertex buffer / index buffer / cvp / instance model buffer / texture staging buffer[..]

	Assimp::Importer* importers = nullptr;
	const aiScene** scenes = nullptr;
	const aiMesh** meshes = nullptr;

	const sunder_arena_suballocation_result_t importers_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, mesh_import_data_per_buffer_allocation_size, 8);
	importers = (Assimp::Importer*)importers_suballocation_result.data;

	const sunder_arena_suballocation_result_t scenes_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, mesh_import_data_per_buffer_allocation_size, 8);
	scenes = (const aiScene**)scenes_suballocation_result.data;

	const sunder_arena_suballocation_result_t meshes_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, mesh_import_data_per_buffer_allocation_size, 8);
	meshes = (const aiMesh**)meshes_suballocation_result.data;

	const sunder_arena_suballocation_result_t texture_buffer_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vulkan_texture_t), core->texture_count + 1), 8);
	core->texture_buffer = (lightray_vulkan_texture_t*)texture_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t texture_vram_requirements_buffer_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(VkMemoryRequirements), core->texture_count + 1), 8);
	VkMemoryRequirements* texture_vram_requirements_buffer = (VkMemoryRequirements*)texture_vram_requirements_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t vram_type_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->texture_count + 1), 4);
	u32* vram_type_index_buffer = (u32*)vram_type_index_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t vram_type_index_filter_buffer_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(u32), core->texture_count + 1), 4);
	u32* vram_type_index_filter_buffer = (u32*)vram_type_index_filter_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t texture_filtering_filter_buffer_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(VkFilter), core->texture_count), 4);
	VkFilter* texture_filtering_filter_buffer = (VkFilter*)texture_filtering_filter_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t descriptor_sets_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(VkDescriptorSet), core->texture_count), alignof(VkDescriptorSet));
	core->descriptor_sets = (VkDescriptorSet*)descriptor_sets_suballocation_result.data;

	const sunder_arena_suballocation_result_t copy_descriptor_set_layouts_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(VkDescriptorSetLayout), core->texture_count), alignof(VkDescriptorSetLayout));
	core->copy_descriptor_set_layouts = (VkDescriptorSetLayout*)copy_descriptor_set_layouts_suballocation_result.data;

	const sunder_arena_suballocation_result_t descriptor_combined_sampler_infos_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(VkDescriptorImageInfo), core->texture_count), alignof(VkDescriptorImageInfo));
	core->descriptor_combined_sampler_infos = (VkDescriptorImageInfo*)descriptor_combined_sampler_infos_suballocation_result.data;

	const sunder_arena_suballocation_result_t host_visible_vram_arena_suballocation_starting_offsets_suballocation_result = sunder_suballocate_from_arena_debug(setup_data->arena, sunder_compute_array_size_in_bytes(sizeof(u64), host_visible_vram_arena_allocation_count), alignof(u64));
	core->host_visible_vram_arena_suballocation_starting_offsets = (u64*)host_visible_vram_arena_suballocation_starting_offsets_suballocation_result.data;

	for (u32 i = 0; i < mesh_count; i++)
	{
		new(&importers[i]) Assimp::Importer();
	}

	for (u32 i = 0; i < mesh_count; i++)
	{
		scenes[i] = importers[i].ReadFile(setup_data->static_mesh_metadata_buffer[i].path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_FlipUVs | aiProcess_CalcTangentSpace| aiProcess_GenNormals);
		meshes[i] = scenes[i]->mMeshes[0];
	}

	u32 index_count_iter = 0;

	for (u32 i = 0; i < mesh_count; i++)
	{
		index_count_iter = lightray_assimp_get_mesh_index_count(meshes[i]);

		SUNDER_LOG("\nindex count: ");
		SUNDER_LOG(index_count_iter);
		SUNDER_LOG("\nvertex count: ");
		SUNDER_LOG(meshes[i]->mNumVertices);
		SUNDER_LOG("\n\n");

		total_index_count += index_count_iter;
		total_vertex_count += meshes[i]->mNumVertices;
		total_instance_model_count += setup_data->static_mesh_metadata_buffer[i].instance_count;

		index_count_iter = 0;
	}

	core->cpu_side_instance_count = total_instance_model_count;

	//const u64 general_purpose_ram_arena_proper_alignment = 8;

	const u64 cpu_side_vertex_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, setup_data->arena_alignment);
	const u64 cpu_side_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, setup_data->arena_alignment);
	const u64 cpu_side_instance_model_allocation_size = sunder_align64(sunder_compute_aligned_allocation_size(sizeof(lightray_render_instance_t), total_instance_model_count, setup_data->arena_alignment), setup_data->arena_alignment);

	const u64 device_local_vram_arena_vertex_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);
	const u64 device_local_vram_arena_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);
	const u64 device_local_vram_arena_allocation_size = sunder_align64(device_local_vram_arena_vertex_buffer_allocation_size + device_local_vram_arena_index_buffer_allocation_size, LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT);

	const u64 host_visible_vram_arena_vertex_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vertex_t), total_vertex_count, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), total_index_count, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_instance_model_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_render_instance_t), total_instance_model_count, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_cvp_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_cvp_t), 1, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	const u64 host_visible_vram_arena_allocation_size = sunder_align64(host_visible_vram_arena_vertex_buffer_allocation_size + host_visible_vram_arena_index_buffer_allocation_size + host_visible_vram_arena_instance_model_buffer_allocation_size + host_visible_vram_arena_cvp_allocation_size, LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT);
	
	core->device_local_vram_arena_suballocation_starting_offset_count = device_local_vram_arena_allocation_count;
	core->host_visible_vram_arena_suballocation_starting_offset_count = host_visible_vram_arena_allocation_count;

	const u64 device_local_vram_arena_starting_offsets_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), device_local_vram_arena_allocation_count, setup_data->arena_alignment);

	const u64 available_device_extensions_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkExtensionProperties), core->available_device_extension_count, setup_data->arena_alignment);
	const u64 swapchain_presentation_supported_queue_family_indices_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), core->swapchain_presentation_supported_queue_family_index_count, setup_data->arena_alignment);
	const u64 swapchain_images_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkImage), core->swapchain_image_in_use_count, setup_data->arena_alignment);
	const u64 swapchain_image_related_data_allocation_size = sunder_compute_aligned_allocation_size(swapchain_images_allocation_size, 7, setup_data->arena_alignment);
	const u64 shader_byte_code_combined_allocation_size = sunder_align64(compiled_static_mesh_vertex_shader_byte_code_size + compiled_static_mesh_fragment_shader_byte_code_size + compiled_textured_static_mesh_vertex_shader_byte_code_size + compiled_textured_static_mesh_fragment_shader_byte_code_size, setup_data->arena_alignment);
	const u64 queue_family_properties_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkQueueFamilyProperties), core->supported_queue_family_property_count, setup_data->arena_alignment);
	const u64 supported_surface_formats_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkSurfaceFormatKHR), core->supported_surface_format_count, setup_data->arena_alignment);
	const u64 supported_swapchain_present_modes_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkPresentModeKHR), core->supported_swapchain_present_mode_count, setup_data->arena_alignment);

	const u64 mesh_render_pass_data_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_mesh_render_pass_data_t), mesh_count, setup_data->arena_alignment);
	const u64 cpu_side_textured_mesh_render_pass_data_index_buffer = sunder_compute_aligned_allocation_size(sizeof(u32), mesh_count, setup_data->arena_alignment);
	const u64 cpu_side_untextured_mesh_render_pass_data_index_buffer = sunder_compute_aligned_allocation_size(sizeof(u32), mesh_count, setup_data->arena_alignment);

	const u64 host_visible_vram_arena_metadata_allocation_size = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(host_visible_vram_arena_allocation_count, setup_data->arena_alignment);
	const u64 device_local_vram_arena_metadata_allocation_size = lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(device_local_vram_arena_allocation_count, setup_data->arena_alignment);

	const u64 general_purpose_ram_arena_allocation_size = sunder_align64(available_device_extensions_allocation_size + swapchain_presentation_supported_queue_family_indices_allocation_size + swapchain_image_related_data_allocation_size + shader_byte_code_combined_allocation_size + queue_family_properties_allocation_size + supported_surface_formats_allocation_size + supported_swapchain_present_modes_allocation_size + cpu_side_vertex_buffer_allocation_size + cpu_side_index_buffer_allocation_size + cpu_side_instance_model_allocation_size + cpu_side_textured_mesh_render_pass_data_index_buffer + cpu_side_untextured_mesh_render_pass_data_index_buffer + mesh_render_pass_data_buffer_allocation_size + device_local_vram_arena_starting_offsets_allocation_size + device_local_vram_arena_metadata_allocation_size + host_visible_vram_arena_metadata_allocation_size, setup_data->arena_alignment);

	const sunder_arena_result general_purpose_ram_arena_allocation_result = sunder_allocate_arena(&core->general_purpose_ram_arena, general_purpose_ram_arena_allocation_size, setup_data->arena_alignment, SUNDER_ARENA_TYPE_STATIC);
	const u64 general_purpose_ram_arena_buffer_bytes_initialized = sunder_init_buffer(core->general_purpose_ram_arena.buffer, core->general_purpose_ram_arena.capacity, 0, core->general_purpose_ram_arena.capacity);

	const sunder_arena_suballocation_result_t available_device_extensions_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(VkExtensionProperties), core->available_device_extension_count), alignof(VkExtensionProperties));
	core->available_device_extensions = (VkExtensionProperties*)available_device_extensions_suballocation_result.data;

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

	const sunder_arena_suballocation_result_t textured_static_mesh_vertex_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_textured_static_mesh_vertex_shader_byte_code_size, alignof(i8));
	core->shaders[2].byte_code = (i8*)textured_static_mesh_vertex_shader_byte_code_suballocation_result.data;

	const sunder_arena_suballocation_result_t textured_static_mesh_fragment_shader_byte_code_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, compiled_textured_static_mesh_fragment_shader_byte_code_size, alignof(i8));
	core->shaders[3].byte_code = (i8*)textured_static_mesh_fragment_shader_byte_code_suballocation_result.data;

	const sunder_arena_suballocation_result_t cpu_side_vertex_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count), alignof(lightray_vertex_t));
	core->cpu_side_vertex_buffer = (lightray_vertex_t*)cpu_side_vertex_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t cpu_side_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count), alignof(u32));
	core->cpu_side_index_buffer = (u32*)cpu_side_index_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t instance_render_instance_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), total_instance_model_count), alignof(lightray_render_instance_t));
	core->cpu_side_render_instance_buffer = (lightray_render_instance_t*)instance_render_instance_suballocation_result.data;

	const sunder_arena_suballocation_result_t mesh_render_pass_data_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_mesh_render_pass_data_t), mesh_count), alignof(lightray_mesh_render_pass_data_t));
	core->mesh_render_pass_data_buffer = (lightray_mesh_render_pass_data_t*)mesh_render_pass_data_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t textured_mesh_render_pass_data_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), mesh_count), alignof(u32));
	core->textured_mesh_render_pass_data_index_buffer = (u32*)textured_mesh_render_pass_data_index_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t untextured_mesh_render_pass_data_index_buffer_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), mesh_count), alignof(u32));
	core->untextured_mesh_render_pass_data_index_buffer = (u32*)untextured_mesh_render_pass_data_index_buffer_suballocation_result.data;

	const sunder_arena_suballocation_result_t device_local_vram_arena_suballocation_starting_offsetss_suballocation_result = sunder_suballocate_from_arena_debug(&core->general_purpose_ram_arena, sunder_compute_array_size_in_bytes(sizeof(u64), device_local_vram_arena_allocation_count), alignof(u64));
	core->device_local_vram_arena_suballocation_starting_offsets = (u64*)device_local_vram_arena_suballocation_starting_offsetss_suballocation_result.data;

	vkGetPhysicalDeviceQueueFamilyProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], &core->supported_queue_family_property_count, core->supported_queue_family_properties);
	const VkResult available_device_extensions_enumeration_result = vkEnumerateDeviceExtensionProperties(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], nullptr, &core->available_device_extension_count, core->available_device_extensions);
	const VkResult surface_formats_retrieval_result = vkGetPhysicalDeviceSurfaceFormatsKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_surface_format_count, core->supported_surface_formats);
	const VkResult swapchain_present_modes_retrieval_result = vkGetPhysicalDeviceSurfacePresentModesKHR(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], core->surface, &core->supported_swapchain_present_mode_count, core->supported_swapchain_present_modes);

	VkFormat possible_image_formats[4]{};
	possible_image_formats[0] = VK_FORMAT_R8G8B8A8_UNORM;
	possible_image_formats[1] = VK_FORMAT_B8G8R8A8_UNORM;
	possible_image_formats[2] = VK_FORMAT_R8G8B8A8_SRGB;
	possible_image_formats[3] = VK_FORMAT_B8G8R8A8_SRGB;

	const VkFormat supported_image_format = lightray_vulkan_get_supported_image_format(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], possible_image_formats, 4, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT | VK_FORMAT_FEATURE_TRANSFER_DST_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_SRGB);

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

	const u8 texture_creation_mask = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_IMAGE_BIT}, 1);
	u64 host_visible_vram_arena_staging_texture_buffer_allocation_size = 0;

	// excluding depth image that resides at index 0
	for (u32 i = 1; i < core->texture_count + 1; i++)
	{
		const lightray_vulkan_result texture_loading_result = lightray_vulkan_load_texture(setup_data->texture_metadata_buffer[i - 1].path, &core->texture_buffer[i]);

		lightray_vulkan_texture_creation_data_t texture_creation_data{};
		texture_creation_data.width = core->texture_buffer[i].width;
		texture_creation_data.height = core->texture_buffer[i].height;
		texture_creation_data.format = supported_image_format;
		texture_creation_data.tiling = VK_IMAGE_TILING_OPTIMAL; //game_resources_setup_data->texture_metadata_buffer[i - 1].tiling;
		texture_creation_data.usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT; //game_resources_setup_data->texture_metadata_buffer[i - 1].usage_flags;
		texture_creation_data.aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
		texture_creation_data.layout = VK_IMAGE_LAYOUT_UNDEFINED;
		texture_creation_data.filter = setup_data->texture_metadata_buffer[i - 1].filter;
		texture_creation_data.layer_count = setup_data->texture_metadata_buffer[i - 1].layer_count;
		texture_creation_data.creation_flags = texture_creation_mask;

		const lightray_vulkan_result texture_image_creation_result = lightray_vulkan_create_texture(core, &core->texture_buffer[i], &texture_creation_data);
		//host_visible_vram_arena_staging_texture_buffer_allocation_size += core->texture_buffer[i].size; // host_visible_vram_arena_staging_texture_buffer_allocation_size += sunder_align64(host_visible_vram_arena_staging_texture_buffer_allocation_size += sunder_align64(core->texture_buffer[i].size, 256), 256); // do that in multiple steps (lines)
		host_visible_vram_arena_staging_texture_buffer_allocation_size += sunder_align64(core->texture_buffer[i].size, 256);
		host_visible_vram_arena_staging_texture_buffer_allocation_size = sunder_align64(host_visible_vram_arena_staging_texture_buffer_allocation_size, 256);

		vkGetImageMemoryRequirements(core->logical_device, core->texture_buffer[i].image, &texture_vram_requirements_buffer[i]);
		vram_type_index_buffer[i] = lightray_vulkan_get_vram_type_index(core, texture_vram_requirements_buffer[i].memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	/////////////////////////// depth texture creation ///////////////////////////
	VkFormat possible_depth_formats[3]{};
	possible_depth_formats[0] = VK_FORMAT_D32_SFLOAT;
	possible_depth_formats[1] = VK_FORMAT_D32_SFLOAT_S8_UINT;
	possible_depth_formats[2] = VK_FORMAT_D24_UNORM_S8_UINT;

	VkFormat supported_depth_fomat = lightray_vulkan_get_supported_image_format(core->gpus[LIGHTRAY_VULKAN_MAIN_GPU_INDEX], possible_depth_formats, 3, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,VK_FORMAT_D32_SFLOAT);

	lightray_vulkan_texture_creation_data_t depth_texture_creation_data{};
	depth_texture_creation_data.width = 3840; // chnage that to actual monitor resolution queired from some glfw function idk
	depth_texture_creation_data.height = 2160; // chnage that to actual monitor resolution queired from some glfw function idk
	depth_texture_creation_data.format = supported_depth_fomat;
	depth_texture_creation_data.tiling = VK_IMAGE_TILING_OPTIMAL;
	depth_texture_creation_data.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	depth_texture_creation_data.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	depth_texture_creation_data.layout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_texture_creation_data.filter = VK_FILTER_LINEAR;
	depth_texture_creation_data.layer_count = 1;
	depth_texture_creation_data.creation_flags = texture_creation_mask;

	const lightray_vulkan_result depth_texture_image_creation_result = lightray_vulkan_create_texture(core, &core->texture_buffer[0], &depth_texture_creation_data);

	vkGetImageMemoryRequirements(core->logical_device, core->texture_buffer[0].image, &texture_vram_requirements_buffer[0]);

	vram_type_index_buffer[0] = lightray_vulkan_get_vram_type_index(core, texture_vram_requirements_buffer[0].memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	/////////////////////////// depth texture creation ///////////////////////////

	SUNDER_LOG("\n\nvram type index buffer : ");
	for (u32 i = 0; i < core->texture_count + 1; i++)
	{
		SUNDER_LOG(vram_type_index_buffer[i]);
		SUNDER_LOG(" ");
	}

	// filtering out duplicated vram type indices, so that the final filtered buffer looks like : [7, 8] and matches vram texture arena count, instead of [7, 7, 8, 8] and being the count of textures
	u32 vram_type_index_filter_buffer_size = 0;

	for (u32 i = 0; i < core->texture_count + 1; i++)
	{
		//bool vram_type_index_has_already_been_encountered = sunder_exists_u32(vram_type_index_filter_buffer, vram_type_index_filter_buffer_size, vram_type_index_buffer[i]);
		bool vram_type_index_has_already_been_encountered = false;
		SUNDER_CONDITIONAL_EXECUTION(u32, vram_type_index_filter_buffer, vram_type_index_filter_buffer_size, == vram_type_index_buffer[i], vram_type_index_has_already_been_encountered = true; break;);

		if (!vram_type_index_has_already_been_encountered)
		{
			vram_type_index_filter_buffer[vram_type_index_filter_buffer_size] = vram_type_index_buffer[i];
			vram_type_index_filter_buffer_size++;
		}
	}

	core->vram_texture_arena_count = vram_type_index_filter_buffer_size;
	const u64 vram_texture_arena_buffer_suballocation_size = sunder_compute_array_size_in_bytes(sizeof(lightray_vulkan_vram_texture_arena_t), core->vram_texture_arena_count);

	const sunder_arena_suballocation_result_t vram_texture_arena_buffer_suballocation_result = sunder_suballocate_from_arena(setup_data->arena, vram_texture_arena_buffer_suballocation_size, alignof(lightray_vulkan_vram_texture_arena_t));
	core->vram_texture_arena_buffer = (lightray_vulkan_vram_texture_arena_t*)vram_texture_arena_buffer_suballocation_result.data;
	
	SUNDER_LOG("\n\nvram type index filter buffer : ");
	for (u32 i = 0; i < core->vram_texture_arena_count; i++)
	{
		SUNDER_LOG(vram_type_index_filter_buffer[i]);
		SUNDER_LOG(" ");
	}

	// properly allocating vram texture arenas
	for (u32 i = 0; i < core->vram_texture_arena_count; i++)
	{
		u64 current_vram_texture_arena_allocation_size = 0;
		u64 current_alignment = 0;

		for (u32 j = 0; j < core->texture_count + 1; j++)
		{
			if (vram_type_index_filter_buffer[i] == vram_type_index_buffer[j])
			{
				const u64 possible_new_alignment = texture_vram_requirements_buffer[j].alignment;

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
	const u8 texture_view_creation_flags = sunder_to_bit_mask8({LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_VIEW_BIT}, 1);
	lightray_vulkan_texture_creation_data_t texture_view_creation_data{};
	texture_view_creation_data.creation_flags = texture_view_creation_flags;
	texture_view_creation_data.view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	// texture suballocation / view creation
	for (u32 i = 0; i < core->texture_count + 1; i++)
	{
		const lightray_buffer_index_query_result_t res = lightray_query_buffer_index(vram_type_index_filter_buffer, 0, core->vram_texture_arena_count, vram_type_index_buffer[i], false);
		core->texture_buffer[i].vram_texture_arena_index = res.return_index;
		const lightray_vulkan_result texture_suballocation_result = lightray_vulkan_suballocate_texture(core, &core->vram_texture_arena_buffer[res.return_index], &core->texture_buffer[i], texture_vram_requirements_buffer[i].size);
		const lightray_vulkan_result texture_view_creation_result = lightray_vulkan_create_texture(core, &core->texture_buffer[i], &texture_view_creation_data);
	}

	// filtering out duplicated sampler filtering of each texture so that the final filter buffer looks like : [VK_FILTER_NEAREST, VK_FILTER_LINEAR] and matches the sampler count, instead of [VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_FILTER_LINEAR, VK_FILTER_LINEAR] and being the texture count
	u32 texture_filtering_filter_buffer_size = 0;

	for (u32 i = 1; i < core->texture_count + 1; i++)
	{
		bool filter_has_already_been_encountered = false;
		SUNDER_CONDITIONAL_EXECUTION(u32, texture_filtering_filter_buffer, core->texture_count, == core->texture_buffer[i].filter, filter_has_already_been_encountered = true; break;);

		if (!filter_has_already_been_encountered)
		{
			texture_filtering_filter_buffer[texture_filtering_filter_buffer_size] = core->texture_buffer[i].filter;
			texture_filtering_filter_buffer_size++;
		}
	}

	core->sampler_count = texture_filtering_filter_buffer_size;
	const u64 sampler_buffer_suballocation_size = sunder_compute_array_size_in_bytes(sizeof(VkSampler), core->texture_count);

	const sunder_arena_suballocation_result_t sampler_buffer_suballocation_result = sunder_suballocate_from_arena(setup_data->arena, sampler_buffer_suballocation_size, alignof(VkSampler));
	core->sampler_buffer = (VkSampler*)sampler_buffer_suballocation_result.data;

	SUNDER_LOG("\n\ntexture filtering filter buffer: ");
	for (u32 i = 0; i < core->sampler_count; i++)
	{
		SUNDER_LOG(texture_filtering_filter_buffer[i]);
		SUNDER_LOG(" ");
	}

	// samplers creation
	for (u32 i = 0; i < core->sampler_count; i++)
	{
		VkSamplerCreateInfo sampler_create_info{};
		sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler_create_info.magFilter = texture_filtering_filter_buffer[i];
		sampler_create_info.minFilter = texture_filtering_filter_buffer[i];
		sampler_create_info.anisotropyEnable = core->gpu_features.samplerAnisotropy;
		sampler_create_info.maxAnisotropy = core->gpu_properties.limits.maxSamplerAnisotropy;
		sampler_create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		sampler_create_info.unnormalizedCoordinates = VK_FALSE;
		sampler_create_info.compareEnable = VK_FALSE;
		sampler_create_info.compareOp = VK_COMPARE_OP_ALWAYS;
		sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		sampler_create_info.mipLodBias = 0.0f;
		sampler_create_info.minLod = 0.0f;
		sampler_create_info.maxLod = 0.0f;
		sampler_create_info.flags = 0;
		sampler_create_info.pNext = nullptr;

		const VkResult sampler_creation_result = vkCreateSampler(core->logical_device, &sampler_create_info, nullptr, &core->sampler_buffer[i]);
	}

	// load images with stbi_load - done
	// create images/image views for them - done
	// get image vram requirements for each - done
	// get the requireed vram type index for each - done
	// get the number of texture arenas - done
	// compute the allocation size for each - done

	lightray_vulkan_vram_arena_allocation_data_t host_visible_vram_arena_allocation_data{};
	host_visible_vram_arena_allocation_data.allocation_size = sunder_align64(host_visible_vram_arena_allocation_size + sunder_align64(host_visible_vram_arena_staging_texture_buffer_allocation_size, 256), 256);
	host_visible_vram_arena_allocation_data.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	host_visible_vram_arena_allocation_data.metadata_arena = &core->general_purpose_ram_arena;
	host_visible_vram_arena_allocation_data.suballocation_count = host_visible_vram_arena_allocation_count;
	host_visible_vram_arena_allocation_data.metadata_allocation_size = (u32)host_visible_vram_arena_metadata_allocation_size;
	host_visible_vram_arena_allocation_data.vram_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	lightray_vulkan_vram_arena_allocation_data_t device_local_vram_arena_allocation_data{};
	device_local_vram_arena_allocation_data.allocation_size = device_local_vram_arena_allocation_size;
	device_local_vram_arena_allocation_data.usage_flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	device_local_vram_arena_allocation_data.metadata_arena = &core->general_purpose_ram_arena;
	device_local_vram_arena_allocation_data.suballocation_count = device_local_vram_arena_allocation_count;
	device_local_vram_arena_allocation_data.metadata_allocation_size = (u32)device_local_vram_arena_metadata_allocation_size;
	device_local_vram_arena_allocation_data.vram_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	const lightray_vulkan_result host_visible_vram_arena_allocation_result = lightray_vulkan_allocate_vram_arena_debug(core, &core->host_visible_vram_arena, &host_visible_vram_arena_allocation_data);
	const lightray_vulkan_result device_local_vram_arena_allocation_result = lightray_vulkan_allocate_vram_arena_debug(core, &core->device_local_vram_arena, &device_local_vram_arena_allocation_data);

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_vertex_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count));
	core->host_visible_vram_arena_suballocation_starting_offsets[0] = host_visible_vram_arena_vertex_buffer_suballocation_result.starting_offset;

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_index_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count));
	core->host_visible_vram_arena_suballocation_starting_offsets[1] = host_visible_vram_arena_index_buffer_suballocation_result.starting_offset;

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_cvp_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sizeof(lightray_cvp_t));
	core->host_visible_vram_arena_suballocation_starting_offsets[2] = host_visible_vram_arena_cvp_suballocation_result.starting_offset;

	const lightray_vulkan_vram_arena_suballocation_result_t host_visible_vram_arena_instance_model_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), total_instance_model_count));
	core->host_visible_vram_arena_suballocation_starting_offsets[3] = host_visible_vram_arena_instance_model_suballocation_result.starting_offset;

	u32 host_visible_vram_arena_suballocation_starting_offsets_iter = 4;

	for (u32 i = 1; i < core->texture_count + 1; i++)
	{
		const lightray_vulkan_vram_arena_suballocation_result_t raw_texture_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->host_visible_vram_arena, core->texture_buffer[i].size);
		core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_suballocation_starting_offsets_iter] = raw_texture_suballocation_result.starting_offset;
		host_visible_vram_arena_suballocation_starting_offsets_iter++;
	}

	SUNDER_LOG("\n\nhost visible vram arena suballocation starting offsets : ");
	for (u32 i = 0; i < core->host_visible_vram_arena_suballocation_starting_offset_count; i++)
	{
		SUNDER_LOG(core->host_visible_vram_arena_suballocation_starting_offsets[i]);
		SUNDER_LOG(" ");
	}

	const VkResult host_visible_vram_arena_vram_mapping_result = vkMapMemory(core->logical_device, core->host_visible_vram_arena.device_memory, 0, core->host_visible_vram_arena.capacity, 0, &core->cpu_side_host_visible_vram_arena_view);

	const lightray_vulkan_vram_arena_suballocation_result_t device_local_vram_arena_vertex_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->device_local_vram_arena, sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count));
	core->device_local_vram_arena_suballocation_starting_offsets[0] = device_local_vram_arena_vertex_buffer_suballocation_result.starting_offset;

	const lightray_vulkan_vram_arena_suballocation_result_t device_local_vram_arena_index_buffer_suballocation_result = lightray_vulkan_suballocate_from_vram_arena_debug(&core->device_local_vram_arena, sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count));
	core->device_local_vram_arena_suballocation_starting_offsets[1] = device_local_vram_arena_index_buffer_suballocation_result.starting_offset;

	SUNDER_LOG("\n\ndevice local vram arena suballocation starting offsets : ");
	for (u32 i = 0; i < core->device_local_vram_arena_suballocation_starting_offset_count; i++)
	{
		SUNDER_LOG(core->device_local_vram_arena_suballocation_starting_offsets[i]);
		SUNDER_LOG(" ");
	}

	lightray_vulkan_create_swapchain(core);

	lightray_get_shader_byte_code(compiled_static_mesh_vertex_shader_path, core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX].byte_code_size, 600'000ULL);
	lightray_get_shader_byte_code(compiled_static_mesh_fragment_shader_path, core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX].byte_code_size, 600'000ULL);
	lightray_get_shader_byte_code(compiled_textured_static_mesh_vertex_shader_path, core->shaders[2].byte_code, &core->shaders[2].byte_code_size, 600'000ULL);
	lightray_get_shader_byte_code(compiled_textured_static_mesh_fragment_shader_path, core->shaders[3].byte_code, &core->shaders[3].byte_code_size, 600'000ULL);

	const lightray_vulkan_result static_mesh_vertex_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX], VK_SHADER_STAGE_VERTEX_BIT);
	const lightray_vulkan_result static_mesh_fragment_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX], VK_SHADER_STAGE_FRAGMENT_BIT);
	const lightray_vulkan_result textured_static_mesh_vertex_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[2], VK_SHADER_STAGE_VERTEX_BIT);
	const lightray_vulkan_result textured_static_mesh_fragment_shader_creation_result = lightray_vulkan_create_shader(core, &core->shaders[3], VK_SHADER_STAGE_FRAGMENT_BIT);

	core->descriptor_set_layout_bindings[0].binding = 0;
	core->descriptor_set_layout_bindings[0].descriptorCount = 1;
	core->descriptor_set_layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	core->descriptor_set_layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	core->descriptor_set_layout_bindings[0].pImmutableSamplers = nullptr;

	core->descriptor_set_layout_bindings[1].binding = 1;
	core->descriptor_set_layout_bindings[1].descriptorCount = 1;
	core->descriptor_set_layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	core->descriptor_set_layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	core->descriptor_set_layout_bindings[1].pImmutableSamplers = nullptr;

	core->descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	core->descriptor_set_layout_info.bindingCount = 2;
	core->descriptor_set_layout_info.pBindings = core->descriptor_set_layout_bindings;
	core->descriptor_set_layout_info.flags = 0;
	core->descriptor_set_layout_info.pNext = nullptr;

	const VkResult descriptor_set_layout_creation_result = vkCreateDescriptorSetLayout(core->logical_device, &core->descriptor_set_layout_info, nullptr, &core->main_descriptor_set_layout);

	for (u32 i = 0; i < core->texture_count; i++)
	{
		core->copy_descriptor_set_layouts[i] = core->main_descriptor_set_layout;
	}

	core->descriptor_pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	core->descriptor_pool_sizes[0].descriptorCount = 1;

	core->descriptor_pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	core->descriptor_pool_sizes[1].descriptorCount = core->texture_count;

	core->descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	core->descriptor_pool_info.maxSets = core->texture_count;
	core->descriptor_pool_info.pPoolSizes = core->descriptor_pool_sizes;
	core->descriptor_pool_info.poolSizeCount = 2;
	core->descriptor_pool_info.flags = 0;
	core->descriptor_pool_info.pNext = nullptr;

	const VkResult descriptor_pool_creation_result = vkCreateDescriptorPool(core->logical_device, &core->descriptor_pool_info, nullptr, &core->descriptor_pool);

	core->descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	core->descriptor_set_allocate_info.descriptorPool = core->descriptor_pool;
	core->descriptor_set_allocate_info.pSetLayouts = core->copy_descriptor_set_layouts;
	core->descriptor_set_allocate_info.descriptorSetCount = core->texture_count;
	core->descriptor_set_allocate_info.pNext = nullptr;

	const VkResult descriptor_set_allocation_result = vkAllocateDescriptorSets(core->logical_device, &core->descriptor_set_allocate_info, core->descriptor_sets);

	core->descriptor_buffer_info.buffer = core->host_visible_vram_arena.buffer;
	core->descriptor_buffer_info.offset = core->host_visible_vram_arena_suballocation_starting_offsets[2];
	core->descriptor_buffer_info.range = sizeof(lightray_cvp_t);

	for (u32 i = 1; i < core->texture_count + 1; i++)
	{
		const lightray_buffer_index_query_result_t sampler_index_query_result = lightray_query_buffer_index((u32*)texture_filtering_filter_buffer, 0, core->sampler_count, (u32)core->texture_buffer[i].filter, false);
		core->descriptor_combined_sampler_infos[i - 1].imageView = core->texture_buffer[i].view;
		core->descriptor_combined_sampler_infos[i - 1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //core->texture_buffer[i].layout;
		core->descriptor_combined_sampler_infos[i - 1].sampler = core->sampler_buffer[sampler_index_query_result.return_index];
	}

	for (u32 i = 0; i < core->texture_count; i++)
	{
		VkWriteDescriptorSet cvp_write_descriptor_set{};
		cvp_write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		cvp_write_descriptor_set.dstSet = core->descriptor_sets[i];
		cvp_write_descriptor_set.descriptorCount = 1;
		cvp_write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cvp_write_descriptor_set.dstBinding = 0;
		cvp_write_descriptor_set.dstArrayElement = 0;
		cvp_write_descriptor_set.pBufferInfo = &core->descriptor_buffer_info;

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

		VkWriteDescriptorSet write_descriptor_sets[2]{};
		write_descriptor_sets[0] = cvp_write_descriptor_set;
		write_descriptor_sets[1] = combined_sampler_write_descriptor_set;

		vkUpdateDescriptorSets(core->logical_device, 2, write_descriptor_sets, 0, nullptr);
	}

	core->pipeline_shader_stage_infos[0] = core->shaders[0].stage_info;
	core->pipeline_shader_stage_infos[1] = core->shaders[1].stage_info;
	core->pipeline_shader_stage_infos[2] = core->shaders[2].stage_info;
	core->pipeline_shader_stage_infos[3] = core->shaders[3].stage_info;

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

	core->vertex_input_attribute_descriptions[0].binding = 0;
	core->vertex_input_attribute_descriptions[0].location = 0;
	core->vertex_input_attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	core->vertex_input_attribute_descriptions[0].offset = offsetof(lightray_vertex_t, position);

	core->vertex_input_attribute_descriptions[1].binding = 0;
	core->vertex_input_attribute_descriptions[1].location = 1;
	core->vertex_input_attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
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

	core->vertex_input_attribute_descriptions[8].binding = 1;
	core->vertex_input_attribute_descriptions[8].location = 8;
	core->vertex_input_attribute_descriptions[8].format = VK_FORMAT_R32_SFLOAT;
	core->vertex_input_attribute_descriptions[8].offset = offsetof(lightray_render_instance_t, layer_index);

	core->pipeline_vertex_input_state_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	core->pipeline_vertex_input_state_infos[0].pVertexBindingDescriptions = core->vertex_input_binding_descriptions;
	core->pipeline_vertex_input_state_infos[0].vertexBindingDescriptionCount = 2;
	core->pipeline_vertex_input_state_infos[0].pVertexAttributeDescriptions = core->vertex_input_attribute_descriptions;
	core->pipeline_vertex_input_state_infos[0].vertexAttributeDescriptionCount = 8;

	core->pipeline_vertex_input_state_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	core->pipeline_vertex_input_state_infos[1].pVertexBindingDescriptions = core->vertex_input_binding_descriptions;
	core->pipeline_vertex_input_state_infos[1].vertexBindingDescriptionCount = 2;
	core->pipeline_vertex_input_state_infos[1].pVertexAttributeDescriptions = core->vertex_input_attribute_descriptions;
	core->pipeline_vertex_input_state_infos[1].vertexAttributeDescriptionCount = 9;

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

	core->pipeline_layout_infos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	core->pipeline_layout_infos[0].pSetLayouts = &core->main_descriptor_set_layout;
	core->pipeline_layout_infos[0].setLayoutCount = 1;
	core->pipeline_layout_infos[0].pushConstantRangeCount = 0;
	core->pipeline_layout_infos[0].pPushConstantRanges = nullptr;
	core->pipeline_layout_infos[0].flags = 0;
	core->pipeline_layout_infos[0].pNext = nullptr;

	core->pipeline_layout_infos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	core->pipeline_layout_infos[1].pSetLayouts = &core->main_descriptor_set_layout;
	core->pipeline_layout_infos[1].setLayoutCount = 1;
	core->pipeline_layout_infos[1].pushConstantRangeCount = 0;
	core->pipeline_layout_infos[1].pPushConstantRanges = nullptr;
	core->pipeline_layout_infos[1].flags = 0;
	core->pipeline_layout_infos[1].pNext = nullptr;

	const VkResult  pipeline_layout_creation_result = vkCreatePipelineLayout(core->logical_device, &core->pipeline_layout_infos[0], nullptr, &core->pipeline_layouts[0]);
	const VkResult  pipeline_layout_creation_result2 = vkCreatePipelineLayout(core->logical_device, &core->pipeline_layout_infos[1], nullptr, &core->pipeline_layouts[1]);

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

	core->graphics_pipeline_infos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[0].stageCount = 2;
	core->graphics_pipeline_infos[0].pStages = core->pipeline_shader_stage_infos;
	core->graphics_pipeline_infos[0].pVertexInputState = &core->pipeline_vertex_input_state_infos[0];
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

	core->graphics_pipeline_infos[1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[1].stageCount = 2;
	core->graphics_pipeline_infos[1].pStages = core->pipeline_shader_stage_infos;
	core->graphics_pipeline_infos[1].pVertexInputState = &core->pipeline_vertex_input_state_infos[0];
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

	core->graphics_pipeline_infos[2].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	core->graphics_pipeline_infos[2].stageCount = 2;
	core->graphics_pipeline_infos[2].pStages = core->pipeline_shader_stage_infos + 2; // starting from the 3rd shader ([2]) which is textured static mesh vertex one
	core->graphics_pipeline_infos[2].pVertexInputState = &core->pipeline_vertex_input_state_infos[1];
	core->graphics_pipeline_infos[2].pViewportState = &core->pipeline_viewport_state_info;
	core->graphics_pipeline_infos[2].pInputAssemblyState = &core->pipeline_input_assembly_state_infos[0];
	core->graphics_pipeline_infos[2].pRasterizationState = &core->pipeline_rasterization_state_infos[0];
	core->graphics_pipeline_infos[2].pMultisampleState = &core->pipeline_multisample_state_info;
	core->graphics_pipeline_infos[2].pDepthStencilState = &depth_stencil_state_info;
	core->graphics_pipeline_infos[2].pColorBlendState = &core->pipeline_color_blend_state_info;
	core->graphics_pipeline_infos[2].pDynamicState = &core->pipeline_dynamic_state_info;
	core->graphics_pipeline_infos[2].layout = core->pipeline_layouts[1];
	core->graphics_pipeline_infos[2].renderPass = core->render_pass;
	core->graphics_pipeline_infos[2].subpass = 0;
	core->graphics_pipeline_infos[2].basePipelineHandle = nullptr;
	core->graphics_pipeline_infos[2].basePipelineIndex = -1;
	core->graphics_pipeline_infos[2].flags = 0;
	core->graphics_pipeline_infos[2].pNext = nullptr;

	const VkResult graphics_pipelines_creation_result = vkCreateGraphicsPipelines(core->logical_device, nullptr, 3, core->graphics_pipeline_infos, nullptr, core->pipelines);

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

	u64 host_visible_vram_arena_texture_staging_buffer_current_index = 4u;
	u64 current_host_visible_vram_arena_buffer_copying_offset = core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_texture_staging_buffer_current_index];

	for (u32 i = 1; i < core->texture_count + 1; i++)
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
		current_host_visible_vram_arena_buffer_copying_offset = core->host_visible_vram_arena_suballocation_starting_offsets[host_visible_vram_arena_texture_staging_buffer_current_index];

		lightray_vulkan_transition_texture_layout(core->graphics_queue, core->general_purpose_command_buffer, &core->general_purpose_command_buffer_begin_info, &core->texture_buffer[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		lightray_vulkan_populate_texture(&core->texture_buffer[i], &texture_population_data);
		lightray_vulkan_transition_texture_layout(core->graphics_queue, core->general_purpose_command_buffer, &core->general_purpose_command_buffer_begin_info, &core->texture_buffer[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		host_visible_vram_arena_texture_staging_buffer_current_index++;
	}

	const u8 texture_ram_buffer_free_flags = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_RAM_BUFFER_BIT }, 1);

	for (u32 i = 1; i < core->texture_count + 1; i++)
	{
		const lightray_vulkan_result texture_ram_buffer_free_result = lightray_vulkan_free_texture(core, &core->texture_buffer[i], texture_ram_buffer_free_flags);
	}

	// populating mesh render pass data
	u32 index_buffer_offset_iter = 0;
	u32 instance_model_buffer_offset_iter = 0;
	u32 vertex_buffer_offset_iter = 0;

	for(u32 i = 0; i < mesh_count; i++)
	{
		// index buffer related
		core->mesh_render_pass_data_buffer[i].index_count = lightray_assimp_get_mesh_index_count(meshes[i]);
		core->mesh_render_pass_data_buffer[i].index_buffer_offset = index_buffer_offset_iter;
		index_buffer_offset_iter += core->mesh_render_pass_data_buffer[i].index_count;

		// instance buffer related
		core->mesh_render_pass_data_buffer[i].instance_count = setup_data->static_mesh_metadata_buffer[i].instance_count;
		core->mesh_render_pass_data_buffer[i].instance_to_render_count = setup_data->static_mesh_metadata_buffer[i].instance_count;
		core->mesh_render_pass_data_buffer[i].instance_buffer_offset = instance_model_buffer_offset_iter;
		instance_model_buffer_offset_iter += setup_data->static_mesh_metadata_buffer[i].instance_count;

		// vertex buffer related
		core->mesh_render_pass_data_buffer[i].vertex_count = meshes[i]->mNumVertices;
		core->mesh_render_pass_data_buffer[i].vertex_buffer_offset = vertex_buffer_offset_iter;
		vertex_buffer_offset_iter += meshes[i]->mNumVertices;

		core->mesh_render_pass_data_buffer[i].texture_index = LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX;
	}

	SUNDER_LOG("\n\n");
	for (u32 i = 0; i < mesh_count; i++)
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
		SUNDER_LOG("\n\n");
	}

	// parsing mesh index buffers
	u32 index_buffer_iter = 0;

	for(u32 m = 0; m < mesh_count; m++)
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

	for (u32 i = 0; i < core->cpu_side_instance_count; i++)
	{
		core->cpu_side_render_instance_buffer[i].model.model = glm::mat4(1.0f);
		core->cpu_side_render_instance_buffer[i].layer_index = LIGHTRAY_VULKAN_INVALID_TEXTURE_LAYER_INDEX;
	}

	// parsing mesh vertex buffers
	u32 vertex_buffer_iter = 0;

	for (u32 m = 0; m < core->mesh_count; m++)
	{
		for (u32 v = 0; v < meshes[m]->mNumVertices; v++)
		{
			core->cpu_side_vertex_buffer[vertex_buffer_iter].position.x = meshes[m]->mVertices[v].x;
			core->cpu_side_vertex_buffer[vertex_buffer_iter].position.y = meshes[m]->mVertices[v].y;
			core->cpu_side_vertex_buffer[vertex_buffer_iter].position.z = meshes[m]->mVertices[v].z;

			core->cpu_side_vertex_buffer[vertex_buffer_iter].texture_coordinates.x = meshes[m]->mTextureCoords[0][v].x;
			core->cpu_side_vertex_buffer[vertex_buffer_iter].texture_coordinates.y = meshes[m]->mTextureCoords[0][v].y;
			
			core->cpu_side_vertex_buffer[vertex_buffer_iter].normal.x = meshes[m]->mNormals[v].x;
			core->cpu_side_vertex_buffer[vertex_buffer_iter].normal.y = meshes[m]->mNormals[v].y;
			core->cpu_side_vertex_buffer[vertex_buffer_iter].normal.z = meshes[m]->mNormals[v].z;

			core->cpu_side_vertex_buffer[vertex_buffer_iter].alpha = 1.0f;

			vertex_buffer_iter++;
		}
	}

	lightray_vulkan_set_fov(core, 100.0f);
	lightray_vulkan_set_target_fps(core, setup_data->target_fps);

	glfwGetCursorPos(core->window, &core->cursor.last_position.x, &core->cursor.last_position.y);

	core->main_camera.first_camera_tick = true;
	core->main_camera.position = glm::vec3(0.5f, 0.0f, 0.0f);
	core->cvp.view = glm::lookAt(core->main_camera.position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	const u64 bytes_to_write_vertex_buffer = sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count);
	const u64 bytes_to_write_index_buffer = sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count);
	const u64 bytes_to_write_cvp = sizeof(lightray_cvp_t);
	const u64 bytes_to_write_instance_model_buffer = sunder_compute_array_size_in_bytes(sizeof(lightray_render_instance_t), total_instance_model_count);

	sunder_buffer_copy_data_t buffer_copy_data{};
	buffer_copy_data.dst_size = core->host_visible_vram_arena.capacity;
	buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[0];
	buffer_copy_data.src_size = bytes_to_write_vertex_buffer;
	buffer_copy_data.src_offset = 0;
	buffer_copy_data.bytes_to_write = bytes_to_write_vertex_buffer;

	const u64 vertex_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->cpu_side_vertex_buffer, &buffer_copy_data); // vertex buffer

	buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[1];
	buffer_copy_data.src_size = bytes_to_write_index_buffer;
	buffer_copy_data.bytes_to_write = bytes_to_write_index_buffer;

	const u64 index_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->cpu_side_index_buffer, &buffer_copy_data); // index buffer

	buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[2];
	buffer_copy_data.src_size = bytes_to_write_cvp;
	buffer_copy_data.bytes_to_write = bytes_to_write_cvp;

	const u64 cvp_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, &core->cvp, &buffer_copy_data); // cvp

	buffer_copy_data.dst_offset = core->host_visible_vram_arena_suballocation_starting_offsets[3];
	buffer_copy_data.src_size = bytes_to_write_instance_model_buffer;
	buffer_copy_data.bytes_to_write = bytes_to_write_instance_model_buffer;

	const u64 instance_model_buffer_bytes_written = sunder_copy_buffer(core->cpu_side_host_visible_vram_arena_view, core->cpu_side_render_instance_buffer, &buffer_copy_data); // instance buffer

	lightray_vulkan_copy_buffer(core, core->device_local_vram_arena.buffer, core->host_visible_vram_arena.buffer, core->device_local_vram_arena_suballocation_starting_offsets[0], core->host_visible_vram_arena_suballocation_starting_offsets[0], sunder_compute_array_size_in_bytes(sizeof(lightray_vertex_t), total_vertex_count)); // vertex buffer
	lightray_vulkan_copy_buffer(core, core->device_local_vram_arena.buffer, core->host_visible_vram_arena.buffer, core->device_local_vram_arena_suballocation_starting_offsets[1], core->host_visible_vram_arena_suballocation_starting_offsets[1], sunder_compute_array_size_in_bytes(sizeof(u32), total_index_count)); // index buffer

	// parse mesh data and write it to vertex, index buffers - done
	// copy that to gpu side buffers - done
	// populate the mesh render pass data structs - done
	// construct a projection matrix - done
	// set fov to more than 0 so that it doesnt get clamped to 0 anytime i wanna change it later and shit renders correctly - done
	// set initial time for delta time computation - done
	// and also initial cursor position or whatever - done
	// lock fps - done
	// do the fullscreen stuff - done
	// finish render pass - done
	// add a layer index in instance vertex data - done
	// consider aligning my vertex struct by 16 - pending
	// consider doing that for instance struct aswell (16) - done
	
	for (u32 i = 0; i < setup_data->mesh_count; i++)
	{
		importers[i].~Importer();
	}

	/////////////////////////// setup core end ///////////////////////////
}

void lightray_vulkan_terminate_core(lightray_vulkan_core_t* core)
{
	const u8 texture_buffer_texture_free_mask = sunder_to_bit_mask8({ LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_IMAGE_BIT, LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_VIEW_BIT }, 2);

	for (u32 i = 0; i < core->texture_count + 1; i++)
	{
		const lightray_vulkan_result texture_ram_buffer_free_result = lightray_vulkan_free_texture(core, &core->texture_buffer[i], texture_buffer_texture_free_mask);
	}

	for (u32 i = 0; i < core->vram_texture_arena_count; i++)
	{
		lightray_vulkan_free_vram_texture_arena(core, &core->vram_texture_arena_buffer[i]);
	}

	vkUnmapMemory(core->logical_device, core->host_visible_vram_arena.device_memory);
	lightray_vulkan_free_vram_arena(core, &core->host_visible_vram_arena);
	lightray_vulkan_free_vram_arena(core, &core->device_local_vram_arena);

	for (u32 i = 0; i < core->sampler_count; i++)
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

	for (u32 i = 0; i < 2; i++)
	{
		vkDestroyPipelineLayout(core->logical_device, core->pipeline_layouts[i], nullptr);
	}

	for (u32 i = 0; i < 4; i++)
	{
		lightray_vulkan_free_shader(core, &core->shaders[i]);
	}

	for (u32 i = 0; i < 3; i++)
	{
		vkDestroyPipeline(core->logical_device, core->pipelines[i], nullptr);
	}

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
	return SUNDER_IS_ANY_BIT_SET(core->flags, LIGHTRAY_BITS_IS_FPS_CAPPED, 1U);
}

void lightray_vulkan_set_target_fps(lightray_vulkan_core_t* core, f32 desired_fps)
{
	if (!lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_SET_BIT(core->flags, LIGHTRAY_BITS_IS_FPS_CAPPED, 1U);
	}

	lightray_set_target_fps(desired_fps, &core->frame_duration_s, &core->fps);
}

void lightray_vulkan_uncap_fps(lightray_vulkan_core_t* core)
{
	if (lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_ZERO_BIT(core->flags, LIGHTRAY_BITS_IS_FPS_CAPPED, 1U);
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

	SUNDER_LOG("\n");
		
	arena->vram_type_index = device_memory_allocation_info.memoryTypeIndex;
	arena->alignment = vram_requirements.alignment;
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

u64 lightray_vulkan_compute_required_user_chosen_arena_suballocation_size(u32 mesh_count, u32 texture_count, u32 alignment)
{
	const u64 mesh_import_data_per_buffer_allocation_size = sunder_compute_aligned_allocation_size(8, mesh_count, alignment);
	const u64 mesh_import_data_allocation_size = mesh_import_data_per_buffer_allocation_size * 3;
	const u64 texture_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vulkan_texture_t), texture_count + 1, alignment); // + 1 for depth image
	const u64 texture_vram_requirements_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkMemoryRequirements), texture_count + 1, alignment); // + 1 for depth image
	const u64 vram_type_index_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), texture_count + 1, alignment); // + 1 for depth image
	const u64 vram_type_index_filter_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u32), texture_count + 1, alignment); // + 1 for depth image
	const u64 vram_texture_arena_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(lightray_vulkan_vram_texture_arena_t), texture_count + 1, alignment); // + 1 for depth image
	const u64 sampler_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkSampler), texture_count, alignment);
	const u64 texture_filtering_filter_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkFilter), texture_count, alignment);
	const u64 descriptor_combined_sampler_info_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorImageInfo), texture_count, alignment);
	const u64 descriptor_set_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorSet), texture_count, alignment);
	const u64 copy_descriptor_set_layout_buffer_allocation_size = sunder_compute_aligned_allocation_size(sizeof(VkDescriptorSetLayout), texture_count, alignment);
	const u64 host_visible_vram_arena_suballocation_starting_offsets_allocation_size = sunder_compute_aligned_allocation_size(sizeof(u64), 4u + texture_count, alignment);
	
	return sunder_align64(texture_buffer_allocation_size + mesh_import_data_allocation_size + texture_vram_requirements_buffer_allocation_size + vram_type_index_buffer_allocation_size + vram_type_index_filter_buffer_allocation_size + vram_texture_arena_buffer_allocation_size + sampler_buffer_allocation_size + texture_filtering_filter_buffer_allocation_size + descriptor_combined_sampler_info_buffer_allocation_size + descriptor_set_buffer_allocation_size + copy_descriptor_set_layout_buffer_allocation_size + host_visible_vram_arena_suballocation_starting_offsets_allocation_size, alignment);
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
		image_info.imageType = VK_IMAGE_TYPE_2D;
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

lightray_vulkan_result lightray_vulkan_allocate_vram_texture_arena_debug(lightray_vulkan_core_t* core, lightray_vulkan_vram_texture_arena_t* arena, u64 allocation_size, u64 alignment, u32 vram_type_index)
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
		attachments[1] = core->texture_buffer[0].view;

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

void lightray_vulkan_execute_main_render_pass(lightray_vulkan_core_t* core, u32 flags)
{
	static u32 current_frame = 0;
	u32 index_of_acquired_swapchain_image = 0;

	vkWaitForFences(core->logical_device, 1, &core->inflight_fences[current_frame], VK_TRUE, UINT64_MAX);

	vkAcquireNextImageKHR(core->logical_device, core->swapchain, UINT64_MAX, core->image_available_for_rendering_semaphores[current_frame], nullptr, &index_of_acquired_swapchain_image);

	// recreate swapchain if needed

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
	vkCmdBindPipeline(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipelines[0]);

	u64 vertex_buffer_offsets[2]{};
	vertex_buffer_offsets[0] = core->device_local_vram_arena_suballocation_starting_offsets[0];
	vertex_buffer_offsets[1] = core->host_visible_vram_arena_suballocation_starting_offsets[3];

	VkBuffer vertex_buffers[2]{};
	vertex_buffers[0] = core->device_local_vram_arena.buffer;
	vertex_buffers[1] = core->host_visible_vram_arena.buffer;

	vkCmdBindVertexBuffers(core->render_command_buffers[current_frame], 0, 2, vertex_buffers, vertex_buffer_offsets);
	vkCmdBindIndexBuffer(core->render_command_buffers[current_frame], core->device_local_vram_arena.buffer, core->device_local_vram_arena_suballocation_starting_offsets[1], VK_INDEX_TYPE_UINT32);
	vkCmdSetViewport(core->render_command_buffers[current_frame], 0, 1, &core->viewport);
	vkCmdSetScissor(core->render_command_buffers[current_frame], 0, 1, &core->scissor);

	vkCmdBindDescriptorSets(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipeline_layouts[0], 0, 1, &core->descriptor_sets[0], 0, nullptr);

	for (u32 i = 0; i < core->untextured_mesh_render_pass_data_index_buffer_size; i++)
	{
		u32 current_mesh_index = core->untextured_mesh_render_pass_data_index_buffer[i];
		vkCmdDrawIndexed(core->render_command_buffers[current_frame], core->mesh_render_pass_data_buffer[current_mesh_index].index_count, core->mesh_render_pass_data_buffer[current_mesh_index].instance_count, core->mesh_render_pass_data_buffer[current_mesh_index].index_buffer_offset, 0, core->mesh_render_pass_data_buffer[current_mesh_index].instance_buffer_offset);
	}

	// opaque textured static meshes
	vkCmdBindPipeline(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipelines[2]);

	for (u32 i = 0; i < core->textured_mesh_render_pass_data_index_buffer_size; i++)
	{
		u32 current_mesh_index = core->textured_mesh_render_pass_data_index_buffer[i];
		u32 current_texture_index = core->mesh_render_pass_data_buffer[current_mesh_index].texture_index;

		vkCmdBindDescriptorSets(core->render_command_buffers[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, core->pipeline_layouts[1], 0, 1, &core->descriptor_sets[current_texture_index], 0, nullptr);
		vkCmdDrawIndexed(core->render_command_buffers[current_frame], core->mesh_render_pass_data_buffer[current_mesh_index].index_count, core->mesh_render_pass_data_buffer[current_mesh_index].instance_count, core->mesh_render_pass_data_buffer[current_mesh_index].index_buffer_offset, 0, core->mesh_render_pass_data_buffer[current_mesh_index].instance_buffer_offset);
	}

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

	current_frame = (current_frame + 1) % core->swapchain_image_in_use_count;;
}

VkFormat lightray_vulkan_get_supported_image_format(VkPhysicalDevice gpu, VkFormat* formats, u32 format_count, VkImageTiling tiling, VkFormatFeatureFlags features, VkImageUsageFlags usage, VkFormat prefered_format)
{
	VkPhysicalDeviceImageFormatInfo2 image_format_info{};
	image_format_info.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	image_format_info.type = VK_IMAGE_TYPE_2D;
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

void lightray_vulkan_set_fov(lightray_vulkan_core_t* core, f32 desired_fov)
{
	core->cvp.projection = lightray_construct_projection_matrix(desired_fov, lightray_compute_aspect_ratio((f32)core->swapchain_info.imageExtent.width, (f32)core->swapchain_info.imageExtent.height), 0.01f, 20.0f);
}