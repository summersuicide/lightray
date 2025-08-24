#include "lightray_vulkan_core.h"


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

	const u64 compiled_static_mesh_vertex_shader_path_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_vertex_shader_path);
	const u64 compiled_static_mesh_fragment_shader_path_byte_code_size = lightray_get_shader_byte_code_size(compiled_static_mesh_fragment_shader_path);

	core->window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

	core->required_instance_extensions = glfwGetRequiredInstanceExtensions(&core->required_instance_extension_count);
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
	vkGetPhysicalDeviceProperties(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], &core->gpu_properties);
	vkGetPhysicalDeviceFeatures(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], &core->gpu_features);
	vkGetPhysicalDeviceMemoryProperties(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], &core->gpu_vram_properties);
	const VkResult surface_capabilities_retrieaval_result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], core->surface, &core->surface_capabilities);

	vkGetPhysicalDeviceQueueFamilyProperties(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], &core->supported_queue_family_property_count, nullptr);
	vkGetPhysicalDeviceSurfaceFormatsKHR(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], core->surface, &core->supported_surface_format_count, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], core->surface, &core->supported_swapchain_present_mode_count, nullptr);
	vkEnumerateDeviceExtensionProperties(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], nullptr, &core->available_device_extension_count, nullptr);

	lightray_vulkan_get_supported_swapchain_image_count(core);

	VkBool32 queue_family_supports_swapchain_presentation = VK_FALSE;
	SUNDER_LOG("\n\n");

	for (u32 i = 0; i < core->supported_queue_family_property_count; i++)
	{
		const VkResult queue_family_support_query_result = vkGetPhysicalDeviceSurfaceSupportKHR(core->gpus[LIGHTRAY_MAIN_GPU_INDEX], i, core->surface, &queue_family_supports_swapchain_presentation);

		if (queue_family_support_query_result == VK_SUCCESS && queue_family_supports_swapchain_presentation == VK_TRUE)
		{
			core->swapchain_presentation_supported_queue_family_index_count++;
			queue_family_supports_swapchain_presentation = VK_FALSE;
			SUNDER_LOG(i);
			SUNDER_LOG(" ");
		}
	}
}

void lightray_vulkan_terminate_core(lightray_vulkan_core_t* core)
{
	vkDestroySurfaceKHR(core->instance, core->surface, nullptr);
	vkDestroyInstance(core->instance, nullptr);

	glfwDestroyWindow(core->window);
	glfwTerminate();
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
	return SUNDER_IS_NTH_BIT_SET(core->flags, BITS_IS_FPS_CAPPED);
}

void lightray_vulkan_set_target_fps(lightray_vulkan_core_t* core, f32 desired_fps)
{
	if (!lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_SET_NTH_BIT(core->flags, BITS_IS_FPS_CAPPED);
	}

	lightray_set_target_fps(desired_fps, &core->frame_duration_s, &core->fps);
}

void lightray_vulkan_uncap_fps(lightray_vulkan_core_t* core)
{
	if (lightray_vulkan_is_fps_capped(core))
	{
		SUNDER_ZERO_NTH_BIT(core->flags, BITS_IS_FPS_CAPPED);
	}
}