#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "lightray_core.h"

#define LIGHTRAY_MAX_SUPPORTED_GPU_COUNT 1U
#define LIGHTRAY_VULKAN_MAIN_GPU_INDEX 0U
#define LIGHTRAY_MAX_QUEUES_IN_USE_COUNT 2U
#define LIGHTRAY_QUEUE_PRIORITY 1.0f
#define LIGHTRAY_UNLIMITED_SWAPCHAIN_IMAGE_COUNT_SUPPORTED 0U
#define LIGHTRAY_MAX_SWAPCHAIN_IMAGE_COUNT 3U
#define LIGHTRAY_MAX_FRAMES_IN_FLIGHT_COUNT LIGHTRAY_MAX_SWAPCHAIN_IMAGE_COUNT
#define LIGHTRAY_SHADER_STAGE_COUNT 2U
#define LIGHTRAY_DYNAMIC_STATE_COUNT 2U
#define LIGHTRAY_DOUBLE_BUFFERING_SUPPORTED 1U
#define LIGHTRAY_TRIPPLE_BUFFERING_SUPPORTED 2U
#define LIGHTRAY_ENABLED_INSTANCE_LAYER_COUNT 1U
#define LIGHTRAY_ENABLED_DEVICE_EXTENSION_COUNT 1U
#define LIHGTRAY_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_COUNT 2U
#define LIGHTRAY_VULKAN_UNIFORM_BUFFER_ALIGNMENT 256U
#define LIGHTRAY_VULKAN_VERTEX_BUFFER_ALIGNMENT 16U
#define LIGHTRAY_VULKAN_INDEX_BUFFER_ALIGNMENT 4U
#define LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_VERTEX_BUFFER_INDEX 0U
#define LIGHTRAY_VULKAN_DEVICE_LOCAL_VRAM_ARENA_INDEX_BUFFER_INDEX 1U
#define LIGHTRAY_VULKAN_HOSI_VISIBLE_VRAM_ARENA_VERTEX_BUFFER_INDEX 0U
#define LIGHTRAY_VULKAN_HOSI_VISIBLE_VRAM_ARENA_INDEX_BUFFER_INDEX 1U
#define LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_CVP_INDEX 2U
#define LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_TEXTURE_BUFFER_INDEX 3U
#define LIGHTRAY_VULKAN_HOST_VISIBILE_VRAM_ARENA_INSTANCE_BUFFER_INDEX 4U
#define LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX 0U
#define LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX 1U
#define LIGHTRAY_VULKAN_SKELETAL_MESH_VERTEX_SHADER_INDEX 2U
#define LIGHTRAY_VULKAN_SKELETAL_MESH_FRAGMENT_SHADER_INDEX 3U


/*
#define VULKAN_LIGHTRAY_INSTANCED_INDEXED_DEFAULT_PER_MESH_PASS
#define VULKAN_LIGHTRAY_INSTANCED_INDEXED_DEVELOPMENT_PER_MESH_PASS
#define VULKAN_LIGHTRAY_TEXTURED_PASS
#define VULKAN_LIGHTRAY_VERTEX_COLOR_PASS

#define VULKAN_LIGHTRAY_EXECUTE_MAIN_RENDER_PASS_PER_MESH(passes)

VULKAN_LIGHTRAY_EXECUTE_MAIN_RENDER_PASS_PER_MESH(VULKAN_LIGHTRAY_VERTEX_COLOR_PASS VULKAN_LIGHTRAY_INSTANCED_INDEXED_DEVELOPMENT_PER_MESH_PASS);


*/

struct lightray_vulkan_static_mesh_metadata_t
{
	cstring_literal* path = nullptr;
	u32 instance_count = 0;
	u32 wireframe_count = 0; // how many instances of this mesh will be rendered in wireframe
};

struct lightray_mesh_render_pass_data_t
{
	u64 vertex_buffer_offset = 0;
	u64 vertex_count = 0;
	u32 index_count = 0;
	u32 index_buffer_offset = 0;
	u32 instance_count = 0;
	u32 instance_buffer_offset = 0;
	u32 instance_to_render_count = 0;
};

struct lightray_vulkan_skeletal_mesh_t
{

};

struct lightray_vulkan_texture_metadata_t
{
	cstring_literal* path = nullptr;
	VkImageType type{};
	VkFormat format{};
	VkImageTiling tiling{};
	VkImageUsageFlags usage_flags = 0;
};

struct lightray_vulkan_texture_t
{
	VkImage image = nullptr;
	VkImageView view = nullptr;
	i8* buffer = nullptr;
	VkImageType type{};
	VkFormat format{};
	VkImageTiling tiling{};
	VkImageUsageFlags usage_flags = 0;
	u32 vram_type_index = 0;
	u32 width = 0;
	u32 height = 0;
};

struct lightray_vulkan_shader_t
{
	VkShaderModule module = nullptr;
	VkPipelineShaderStageCreateInfo stage_info{};
	u64 byte_code_size = 0;
	i8* byte_code = nullptr;
	VkShaderStageFlagBits stage{};
};

struct lightray_vulkan_vram_arena_t
{
	VkDeviceMemory vram_buffer = nullptr;
	VkBuffer vram_buffer_description;
	u64* suballocation_offset_buffer = nullptr;
	u64 vram_type_index = 0;
	u64 vram_buffer_alignment = 0;
	u64 vram_buffer_capacity = 0;
	u64 vram_buffer_current_offset = 0;
	u64 max_suballocation_count = 0;
	u64 suballocation_total_count = 0;
	u64 suballocation_total_size_in_bytes = 0;
	VkMemoryPropertyFlags vram_property_flags = 0;
};

struct lightray_vulkan_vram_arena_allocation_data_t
{
	u64 allocation_size = 0;
	VkBufferUsageFlags usage_flags{};
	u32 suballocation_count = 0;
	u32 metadata_allocation_size = 0;
	VkMemoryPropertyFlags vram_properties = 0;
	sunder_arena_t* metadata_arena = nullptr;
};

struct lightray_vulkan_game_resources_setup_data_t
{
	lightray_vulkan_static_mesh_metadata_t* static_mesh_metadata_buffer = nullptr;
	lightray_vulkan_texture_metadata_t* texture_metadata_buffer = nullptr;
	u32 mesh_count = 0;
};

struct lightray_vulkan_vram_texture_arena_t
{
	VkDeviceMemory vram_buffer = nullptr;

	u64* suballocation_offset_buffer = nullptr;
	u64 vram_type_index = 0;
	u64 vram_buffer_alignment = 0;
	u64 vram_buffer_capacity = 0;
	u64 vram_buffer_current_offset = 0;
	u64 max_suballocation_count = 0;
	u64 suballocation_total_count = 0;
	u64 suballocation_total_size_in_bytes = 0;
	VkMemoryPropertyFlags vram_property_flags = 0;
};

// architecture -> team fat struct (plex) )))))))))))))))))))))))))) (arena metabuffer instancing)
struct lightray_vulkan_core_t
{
	lightray_vulkan_vram_arena_t host_visible_vram_arena;
	lightray_vulkan_vram_arena_t device_local_vram_arena;
	void* cpu_side_host_visible_vram_arena_view; // mapped with vkMapMemory
	void* cpu_side_device_local_vram_arena_view; // mapped with vkMapMemory
	sunder_arena_t general_purpose_ram_arena;
	u64* host_visible_vram_arena_suballocation_starting_offsets;
	u64* device_local_vram_arena_suballocation_starting_offsets;
	u32 host_visible_vram_arena_suballocation_starting_offset_count;
	u32 device_local_vram_arena_suballocation_starting_offset_count;		

	/*f32 yaw;
	f32 pitch;
	f64 last_x;
	f64 last_y;
	bool first_camera_tick;
	f64 pos_x;
	f64 pos_y;
	f32 camera_sensetivity;
	glm::vec3 camera_position;
	f32 camera_movement_speed;
	f32 near_plane;
	f32 far_plane;
	f32 fov;*/

	lightray_cvp_t cvp;
	lightray_camera_t main_camera;

	u32* wireframe_mesh_count_buffer;

	//u32* cpu_side_index_count_buffer; // dont need to allocate
	//u32* cpu_side_instance_model_count_buffer; // dont need to allocate
	//u32* cpu_side_instance_model_to_render_count_buffer; // dont need to allocate

	//u64* cpu_side_vertex_buffer_offsets; // dont need to allocate
	//u32* cpu_side_index_buffer_offsets; // dont need to allocate
	//u32* cpu_side_instance_model_buffer_offsets_per_mesh; // dont need to allocate
	lightray_mesh_render_pass_data_t* mesh_render_pass_data_buffer;

	u64 cpu_side_vertex_buffer_offset_count;
	u32 cpu_side_index_buffer_offset_count;
	u32 mesh_count;

	lightray_vertex_t* cpu_side_vertex_buffer;
	u32* cpu_side_index_buffer;
	u32 index_count;
	u64 vertex_count;
	f32 delta_time;
	VkImageView texture_image_view;
	u32 cpu_side_instance_count;
	u32 cpu_side_instance_model_buffer_count;
	lightray_model_t* cpu_side_instance_model_buffer;
	lightray_model_t* cpu_side_hidden_instance_model_buffer;

	/////////////////// animation data ////////////////////////
	glm::mat4* bone_local_transform_matrix_buffer; // per mesh
	u32* bone_local_transform_matrix_offset_buffer; // per mesh

	glm::mat4* parent_bone_transform_matrix_buffer; // per mesh
	u32* parent_bone_transform_matrix_offset_buffer; // per mesh

	glm::mat4* bone_offset_matrix_buffer; // per bone / inverse bind pose
	u32* bone_offset_matrix_offset_buffer; // per mesh

	u32* bone_vertex_id_buffer; // per bone
	u32* bone_vertex_id_offset_buffer; // per bone
	u32* bone_vertex_id_count_buffer; // per bone

	f32* bone_vertex_weight_buffer; //  per bone
	u32* bone_vertex_weight_offset_buffer; // per bone
	u32* bone_vertex_weight_count_buffer; // per bone

	aiNodeAnim* animation_channel_buffer; // per animation
	u32* animation_channel_offset_buffer; // per animation
	u32* animation_channel_count_buffer; // per animation

	f64* animation_duration_buffer; // per animation
	f64* animation_ticks_per_second_buffer;

	glm::mat4* computed_bone_transform_matrix_buffer;

	u32* bone_count_buffer; // per mesh
	u32 total_bone_count;
	/////////////////// animation data ////////////////////////
			 
	////////////////////////////////////////// game resources ///////////////////////////////////////
	///////////////////////////////////////// common /////////////////////////////////////////////////
	f32 initial_time;
	f32 current_time;
	f32 frame_duration_s;
	f32 fps;
	///////////////////////////////////////// common /////////////////////////////////////////////////
	////////////////////////////////////////// vulkan related data //////////////////////////////////
	GLFWwindow* window;
	GLFWmonitor* monitor;
	const GLFWvidmode* video_mode;
	VkInstance instance;
	VkSwapchainKHR swapchain;
	VkDevice logical_device;
	VkPhysicalDevice gpus[LIGHTRAY_MAX_SUPPORTED_GPU_COUNT];
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkSurfaceKHR surface;
	VkSwapchainCreateInfoKHR swapchain_info;
	VkApplicationInfo application_info;
	VkInstanceCreateInfo instance_info;
	VkWin32SurfaceCreateInfoKHR win32_surface_info;
	VkDeviceCreateInfo logical_device_info;
	VkPhysicalDeviceProperties gpu_properties;
	VkPhysicalDeviceFeatures gpu_features;
	VkPhysicalDeviceMemoryProperties gpu_vram_properties;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkDeviceQueueCreateInfo graphics_queue_info;
	VkDeviceQueueCreateInfo present_queue_info;
	VkDeviceQueueCreateInfo queues_in_use_infos[LIGHTRAY_MAX_QUEUES_IN_USE_COUNT];
	u32 queue_family_indices_in_use[LIGHTRAY_MAX_QUEUES_IN_USE_COUNT];
	VkSurfaceFormatKHR chosen_surface_format;
	VkPresentModeKHR chosen_swapchain_present_mode;
	lightray_vulkan_shader_t shaders[2];
	//VkShaderModule static_mesh_vertex_shader_module;
	//VkShaderModule static_mesh_fragment_shader_module;
	//VkShaderModuleCreateInfo static_mesh_vertex_shader_module_info;
	//VkShaderModuleCreateInfo static_mesh_fragment_shader_module_info;
	//VkPipelineShaderStageCreateInfo static_mesh_pipeline_vertex_shader_stage_info;
	//VkPipelineShaderStageCreateInfo static_mesh_pipeline_fragment_shader_stage_info;
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_infos[LIGHTRAY_SHADER_STAGE_COUNT];
	VkDynamicState dynamic_states[LIGHTRAY_DYNAMIC_STATE_COUNT];
	VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_info;
	VkVertexInputBindingDescription vertex_input_binding_descriptions[2];
	VkVertexInputAttributeDescription vertex_input_attribute_descriptions[8];
	VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_info;
	VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_infos[2];
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo pipeline_viewport_state_info;
	VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_infos[2];
	VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_info;
	VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state;
	VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_info;
	VkPipelineLayout pipeline_layout;
	VkPipeline pipelines[2];
	VkCommandPool command_pool;
	VkPipelineLayoutCreateInfo pipeline_layout_info;
	VkAttachmentDescription attachment_description;
	VkAttachmentReference attachment_reference;
	VkSubpassDescription subpass_description;
	VkSubpassDependency subpass_dependency;
	VkRenderPassCreateInfo render_pass_info;
	VkGraphicsPipelineCreateInfo graphics_pipeline_infos[2];
	VkCommandPoolCreateInfo command_pool_info;
	VkCommandBufferAllocateInfo render_command_buffers_allocate_info;
	VkCommandBufferAllocateInfo general_purpose_command_buffer_allocate_info;
	VkSemaphoreCreateInfo semaphore_info;
	VkFenceCreateInfo inflight_fence_info;
			//
	VkSampler sampler;
	VkDescriptorSet descriptor_set;
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info;
	VkDescriptorPool descriptor_pool;
	VkDescriptorPoolCreateInfo descriptor_pool_info;
	VkDescriptorPoolSize descriptor_pool_sizes[2];
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info;
	VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[2];
	VkDescriptorBufferInfo descriptor_buffer_info;
	VkWriteDescriptorSet write_descriptor_sets[2];
	VkDescriptorImageInfo descriptor_image_info;
			//
	VkCommandBufferBeginInfo render_command_buffers_begin_info;
	VkCommandBufferBeginInfo general_purpose_command_buffer_begin_info;
	VkRenderPassBeginInfo render_pass_begin_info;
	VkRenderPass render_pass;
	VkFramebufferCreateInfo swapchain_framebuffer_info;
	VkClearValue clear_values[2];
	VkSubmitInfo submit_info;
	VkPresentInfoKHR present_info;
	VkCommandBuffer general_purpose_command_buffer;
	VkCommandBuffer* render_command_buffers;
	VkImage* swapchain_images;
	VkImageView* swapchain_image_views;
	VkImageViewCreateInfo swapchain_image_view_info;
	VkFramebuffer* swapchain_framebuffers;
	//i8* static_mesh_vertex_shader_byte_code; 
	//i8* static_mesh_fragment_shader_byte_code;
	VkSurfaceFormatKHR* supported_surface_formats;
	VkPresentModeKHR* supported_swapchain_present_modes; 
	VkQueueFamilyProperties* supported_queue_family_properties; 
	u32* swapchain_presentation_supported_queue_family_indices; 
	VkSemaphore* image_finished_rendering_semaphores; 
	VkSemaphore* image_available_for_rendering_semaphores; 
	VkFence* inflight_fences; 
	const char** required_instance_extensions;
	const char* enabled_instance_layers[LIGHTRAY_ENABLED_INSTANCE_LAYER_COUNT];
	const char* enabled_device_extensions[LIGHTRAY_ENABLED_DEVICE_EXTENSION_COUNT];
	VkExtensionProperties* available_device_extensions; 
	u32 gpu_count;
	u32 flags;
	f32 queue_priority;
	u32 queues_in_use_info_count;
	u32 queue_family_indices_in_use_count;
	u32 required_instance_extension_count;
	u32 available_device_extension_count;
	u32 enabled_device_extension_count;
	u32 supported_queue_family_property_count;
	u32 supported_surface_format_count;
	u32 supported_swapchain_present_mode_count;
	u32 swapchain_image_in_use_count;
	u32 vertex_shader_byte_code_size;
	u32 fragment_shader_byte_code_size;
	u32 swapchain_presentation_supported_queue_family_index_count;
	////////////////////////////////////////// vulkan related data //////////////////////////////////
};

enum lightray_vulkan_core_result
{
	LIGHTRAY_VULKAN_CORE_RESULT_SUCCESS = 0,
	LIGHTRAY_VULKAN_CORE_RESULT_FAILED_TO_LOAD_TEXTURE,
	LIGHTRAY_VULKAN_CORE_RESULT_FAILED_TO_LOAD_MESH
};

enum lightray_vulkan_vram_arena_result
{
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_SUCCESS = 0,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_INVALID_METADATA_ALLOCATION_SIZE,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_OUT_OF_VRAM,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_FAILED_TO_CREATE_DESCRIPTOR_BUFFER,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_FAILED_TO_BIND_BUFFER,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_OUT_OF_METADATA_RAM,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_SUBALLOCATION_COUNT_IS_0,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_ARENA_ALLOCATION_SIZE_IS_0,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_VRAM_BUFFER_IS_NULLPTR,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_VRAM_BUFFER_DESCRIPTION_IS_NULLPTR,
	LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_UNKOWN_FAILURE
};

enum lightray_vulkan_shader_result
{
	LIGHTRAY_VULKAN_SHADER_RESULT_SUCCESS = 0,
	LIGHTRAY_VULKAN_SHADER_RESULT_UNINITIALIZED_BYTE_CODE_BUFFER,
	LIGHTRAY_VULKAN_SHADER_RESULT_INVALID_BYTE_CODE_SIZE,
	LIGHTRAY_VULKAN_SHADER_RESULT_INVALID_SHADER_STAGE_FLAGS,
	LIGHTRAY_VULKAN_SHADER_RESULT_UNINITIALIZED_SHADER_MODULE,
	LIGHTRAY_VULKAN_SHADER_RESULT_UNKNOWN_FAILURE
};
		
struct lightray_vulkan_vram_arena_suballocation_result_t
{
	u64 starting_offset = 0;
	lightray_vulkan_vram_arena_result result = LIGHTRAY_VULKAN_VRAM_ARENA_ACTION_RESULT_UNKOWN_FAILURE;
};

struct lightray_vulkan_move_entity_data_t
{
	lightray_vulkan_core_t* core = nullptr;
	lightray_scene_t* scene = nullptr;
};

lightray_vulkan_vram_arena_result								lightray_vulkan_allocate_vram_arena_debug(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena, const lightray_vulkan_vram_arena_allocation_data_t* allocation_data);

																					// returns a struct with a result and offset at which suballocation happend
																					// offset is UINT64_MAX on failure
lightray_vulkan_vram_arena_suballocation_result_t		lightray_vulkan_suballocate_from_vram_arena_debug(lightray_vulkan_vram_arena_t* arena, u64 suballocation_size);
lightray_vulkan_vram_arena_result								lightray_vulkan_allocate_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena, const lightray_vulkan_vram_arena_allocation_data_t* allocation_data);
lightray_vulkan_vram_arena_result								lightray_vulkan_suballocate_from_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena);
lightray_vulkan_vram_arena_result								lightray_vulkan_free_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena);



void																				lightray_vulkan_setup_core(lightray_vulkan_core_t* core, lightray_vulkan_game_resources_setup_data_t* game_resources_setup_data, u32 window_width, u32 window_height, cstring_literal* window_title, u32 target_fps, bool fullscreen_mode);
void																				lightray_vulkan_terminate_core(lightray_vulkan_core_t* core);

																					// returns UINT32_MAX on failure
u32																				lightray_vulkan_get_graphics_queue_family_index(const lightray_vulkan_core_t* core);

																					// returns UINT32_MAX on failure
u32																				lightray_vulkan_get_present_queue_family_index(const lightray_vulkan_core_t* core);

																					// returns UINT32_MAX on failure
u32																				lightray_vulkan_get_vram_type_index(const lightray_vulkan_core_t* core, u32 vram_filter, VkMemoryPropertyFlags vram_property_flags);
u64																				lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(const lightray_vulkan_vram_arena_allocation_data_t* allocation_data);

																					// writes to core->swapchain_image_in_use_count
void																				lightray_vulkan_get_supported_swapchain_image_count(lightray_vulkan_core_t* core);
void																				lightray_vulkan_create_swapchain(lightray_vulkan_core_t* core);
void																				lightray_vulkan_free_swapchain(lightray_vulkan_core_t* core);
void																				lightray_vulkan_create_swapchain_framebuffers(lightray_vulkan_core_t* core);
void																				lightray_vulkan_free_swapchain_framebuffers(lightray_vulkan_core_t* core);
void																				lightray_vulkan_execute_main_render_pass(lightray_vulkan_core_t* core, u32 flags);
void																				lightray_vulkan_copy_buffer(lightray_vulkan_core_t* core, VkBuffer dst, VkBuffer src, u64 dst_offset, u64 src_offset, u64 size);
void																				lightray_vulkan_transition_image_layout(lightray_vulkan_core_t* core, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
void																				lightray_vulkan_copy_buffer_to_image(lightray_vulkan_core_t* core, VkBuffer buffer, u32 texture_width, u32 texture_height, u64 buffer_offset, VkOffset3D image_offset);
void																				lightray_vulkan_set_target_fps(lightray_vulkan_core_t* core, f32 desired_fps);
void																				lightray_vulkan_uncap_fps(lightray_vulkan_core_t* core);
bool																				lightray_vulkan_is_fps_capped(const lightray_vulkan_core_t* core);
VkFormat																		lightray_vulkan_get_supported_format(const lightray_vulkan_core_t* core, VkFormat* formats, u32 format_count, VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat																		lightray_vulkan_get_supported_depth_format(const lightray_vulkan_core_t* core);
bool																				lightray_vulkan_contains_stencil_component(VkFormat format);
void																				lightray_vulkan_free_depth_resources(lightray_vulkan_core_t* core);
void																				lightray_vulkan_set_fov(lightray_vulkan_core_t* core, f32 desired_fov);
void																				lightray_vulkan_hide_entity(lightray_vulkan_core_t* core, u32 entity_index, lightray_scene_t* scene);
void																				lightray_vulkan_unhide_entity(lightray_vulkan_core_t* core, u32 entity_index, lightray_scene_t* scene);
void																				lightray_vulkan_move_entity(lightray_vulkan_move_entity_data_t* move_data);
void																				lightray_vulkan_tick_core_begin(lightray_vulkan_core_t* core);
void																				lightray_vulkan_tick_core_end(lightray_vulkan_core_t* core);
lightray_vulkan_shader_result										lightray_vulkan_create_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader, VkShaderStageFlagBits stage_flags);
lightray_vulkan_shader_result										lightray_vulkan_free_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader);