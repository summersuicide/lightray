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
#define LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_INSTANCE_MODEL_BUFFER_INDEX 3U
#define LIGHTRAY_VULKAN_HOST_VISIBLE_VRAM_ARENA_TEXTURE_BUFFER_INDEX 4U
#define LIGHTRAY_VULKAN_STATIC_MESH_VERTEX_SHADER_INDEX 0U
#define LIGHTRAY_VULKAN_STATIC_MESH_FRAGMENT_SHADER_INDEX 1U
#define LIGHTRAY_VULKAN_SKELETAL_MESH_VERTEX_SHADER_INDEX 2U
#define LIGHTRAY_VULKAN_SKELETAL_MESH_FRAGMENT_SHADER_INDEX 3U
#define LIGHTRAY_VULKAN_INVALID_TEXTURE_INDEX 2048u
#define LIGHTRAY_VULKAN_INVALID_TEXTURE_LAYER_INDEX 256

// used for grouping unrelated meshes that have the same number of instances, together in one batch. gathered at runtime
struct lightray_vulkan_render_batch_t
{

};

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
	u32 texture_index = 0;
};

struct lightray_vulkan_skeletal_mesh_t
{

};

enum lightray_vulkan_result : u32
{
	LIGHTRAY_VULKAN_RESULT_SUCCESS = 0u,
	LIGHTRAY_VULKAN_RESULT_FAILURE = 1u,
	LIGHTRAY_VULKAN_RESULT_FAILED_TO_LOAD_TEXTURE = 2u,
	LIGHTRAY_VULKAN_RESULT_FAILED_TO_LOAD_MESH = 3u,
	LIGHTRAY_VULKAN_RESULT_INVALID_METADATA_ALLOCATION_SIZE = 4u,
	LIGHTRAY_VULKAN_RESULT_OUT_OF_VRAM = 5u,
	LIGHTRAY_VULKAN_RESULT_FAILED_TO_CREATE_BUFFER = 6u,
	LIGHTRAY_VULKAN_RESULT_FAILED_TO_BIND_BUFFER = 7u,
	LIGHTRAY_VULKAN_RESULT_SUBALLOCATION_COUNT_IS_0 = 8u,
	LIGHTRAY_VULKAN_RESULT_ARENA_ALLOCATION_SIZE_IS_0 = 9u,
	LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_DEVICE_MEMORY = 10u,
	LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_BUFFER = 11u,
	LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_BYTE_CODE_BUFFER = 12u,
	LIGHTRAY_VULKAN_RESULT_INVALID_BYTE_CODE_SIZE = 13u,
	LIGHTRAY_VULKAN_RESULT_INVALID_SHADER_STAGE_FLAGS = 14u,
	LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_SHADER_MODULE = 15u,
	LIGHTRAY_VULKAN_RESULT_FAILED_TO_LOAD_IMAGE = 16u,
	LIGHTRAY_VULKAN_RESULT_INVALID_FLAGS = 17u,
	LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_TEXTURE_BUFFER = 18u,
	LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_IMAGE = 19u,
	LIGHTRAY_VULKAN_RESULT_UNINITIALIZED_VIEW = 20u,
	LIGHTRAY_VULKAN_RESULT_FAILED_TO_BIND_IMAGE = 21u
};

enum lightray_vulkan_texture_creation_bits : u8
{
	LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_IMAGE_BIT = 0,
	LIGHTRAY_VULKAN_TEXTURE_CREATION_BITS_VIEW_BIT = 1
};


enum lightray_vulkan_texture_free_bits : u8
{
	LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_RAM_BUFFER_BIT = 0,
	LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_IMAGE_BIT = 1,
	LIGHTRAY_VULKAN_TEXTURE_FREE_BITS_VIEW_BIT = 2
};

struct lightray_vulkan_texture_creation_data_t
{
	i32 width = 0;
	i32 height = 0;;
	VkFormat format{};
	VkImageTiling tiling{};
	VkImageUsageFlags usage_flags{};
	VkImageAspectFlags aspect_flags{};
	VkImageLayout layout{};
	VkFilter filter;
	u32 layer_count = 0;
	VkImageViewType view_type{};
	u8 creation_flags = 0;
};

struct lightray_vulkan_texture_population_data_t
{
	VkQueue queue = nullptr;
	VkCommandBuffer command_buffer = nullptr;
	const VkCommandBufferBeginInfo* command_buffer_begin_info = nullptr;
	VkBuffer buffer = nullptr;
	u64 buffer_offset = 0;
	u32 starting_layer = 0;
	u32 layer_count = 0;
};

struct lightray_vulkan_texture_metadata_t
{
	cstring_literal* path = nullptr;
	VkFilter filter{};
	u32 layer_count = 0;
};

struct lightray_vulkan_raw_texture_buffer_t
{
	i8* buffer = nullptr;
	u32* texture_offset_buffer = nullptr;
	u32 texture_count = 0;
};

struct lightray_vulkan_texture_t
{
	VkImage image = nullptr;
	VkImageView view = nullptr;
	u8* buffer = nullptr;
	u64 size = 0;
	VkImageTiling tiling{};
	VkImageUsageFlags usage_flags = 0;
	VkFormat format{};
	VkFilter filter{};
	VkImageLayout layout{};
	VkImageAspectFlags aspect_flags = 0;
	u32 vram_texture_arena_index = UINT32_MAX;
	i32 width = 0;
	i32 height = 0;
	u32 layer_count = 0;
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
	VkDeviceMemory device_memory = nullptr;
	VkBuffer buffer;
	u64* suballocation_offset_buffer = nullptr;
	u64 alignment = 0;
	u64 capacity = 0;
	u64 current_offset = 0;
	u64 max_suballocation_count = 0;
	u64 suballocation_total_count = 0;
	u64 suballocation_total_size_in_bytes = 0;
	u32 vram_type_index = 0;
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
	sunder_arena_t* arena = nullptr;
	lightray_vulkan_static_mesh_metadata_t* static_mesh_metadata_buffer = nullptr;
	lightray_vulkan_texture_metadata_t* texture_metadata_buffer = nullptr;
	u32 mesh_count = 0;
	u32 texture_count = 0;
};

// use this later in lightray_vulkan_setup_core function instead of lightray_vulkan_game_resources_setup_data_t and other goofy arguments
struct lightray_vulkan_core_setup_data_t
{
	u32 window_width;
	u32 window_height;
	cstring_literal* window_title;
	f32 target_fps; 
	bool fullscreen_mode;

	sunder_arena_t* arena = nullptr;
	u64 arena_alignment = 0; // alignment at which the arena has been allocated with _aligned_malloc(). crucial for allocation/suballocation computations
	lightray_vulkan_static_mesh_metadata_t* static_mesh_metadata_buffer = nullptr;
	lightray_vulkan_texture_metadata_t* texture_metadata_buffer = nullptr;
	u32 mesh_count = 0;
	u32 texture_count = 0;
};

struct lightray_vulkan_vram_texture_arena_t
{
	VkDeviceMemory device_memory = nullptr;

	u64 alignment = 0;
	u64 capacity = 0;
	u64 current_offset = 0;
	u64 suballocation_total_count = 0;
	u64 suballocation_total_size_in_bytes = 0;
	u32 vram_type_index = 0;
};

struct lightray_vulkan_runtime_texture_descriptor_indices_t
{
	u32 sampler_descriptor_buffer_index = 0;
	u32 image_descriptor_buffer_index = 0;
};

// architecture -> team fat struct (plex) )))))))))))))))))))))))))) (arena metabuffer instancing)
struct lightray_vulkan_core_t
{
	cstring_literal* relative_path;

	lightray_vulkan_vram_arena_t host_visible_vram_arena;
	lightray_vulkan_vram_arena_t device_local_vram_arena;
	void* cpu_side_host_visible_vram_arena_view; // mapped with vkMapMemory
	sunder_arena_t general_purpose_ram_arena;
	u64* host_visible_vram_arena_suballocation_starting_offsets;
	u64* device_local_vram_arena_suballocation_starting_offsets;
	u32 host_visible_vram_arena_suballocation_starting_offset_count;
	u32 device_local_vram_arena_suballocation_starting_offset_count;		

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	lightray_vulkan_texture_t* texture_buffer;
	lightray_vulkan_vram_texture_arena_t* vram_texture_arena_buffer;
	VkSampler* sampler_buffer;
	u32 texture_count;
	u32 vram_texture_arena_count;
	u32 sampler_count;
	////////////////////////////////////////////////////////////////////////////////////////////////////////

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
	lightray_cursor_t cursor;

	u32* wireframe_mesh_count_buffer;

	//u32* cpu_side_index_count_buffer; // dont need to allocate
	//u32* cpu_side_instance_model_count_buffer; // dont need to allocate
	//u32* cpu_side_instance_model_to_render_count_buffer; // dont need to allocate

	//u64* cpu_side_vertex_buffer_offsets; // dont need to allocate
	//u32* cpu_side_index_buffer_offsets; // dont need to allocate
	//u32* cpu_side_instance_model_buffer_offsets_per_mesh; // dont need to allocate
	lightray_mesh_render_pass_data_t* mesh_render_pass_data_buffer;
	u32* untextured_mesh_render_pass_data_index_buffer;
	u32* textured_mesh_render_pass_data_index_buffer;
	u32 untextured_mesh_render_pass_data_index_buffer_size;
	u32 textured_mesh_render_pass_data_index_buffer_size;

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
	lightray_render_instance_t* cpu_side_render_instance_buffer;

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
	lightray_vulkan_shader_t shaders[4];
	//VkShaderModule static_mesh_vertex_shader_module;
	//VkShaderModule static_mesh_fragment_shader_module;
	//VkShaderModuleCreateInfo static_mesh_vertex_shader_module_info;
	//VkShaderModuleCreateInfo static_mesh_fragment_shader_module_info;
	//VkPipelineShaderStageCreateInfo static_mesh_pipeline_vertex_shader_stage_info;
	//VkPipelineShaderStageCreateInfo static_mesh_pipeline_fragment_shader_stage_info;
	VkPipelineShaderStageCreateInfo pipeline_shader_stage_infos[4];
	VkDynamicState dynamic_states[LIGHTRAY_DYNAMIC_STATE_COUNT];
	VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_info;
	VkVertexInputBindingDescription vertex_input_binding_descriptions[2];
	VkVertexInputAttributeDescription vertex_input_attribute_descriptions[9];
	VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_state_infos[2];
	VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_state_infos[2];
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo pipeline_viewport_state_info;
	VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_infos[2];
	VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_info;
	VkPipelineColorBlendAttachmentState pipeline_color_blend_attachment_state;
	VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_info;
	VkPipelineLayout pipeline_layouts[2];
	VkPipeline pipelines[3];
	VkCommandPool command_pool;
	VkPipelineLayoutCreateInfo pipeline_layout_infos[2];
	VkAttachmentDescription attachment_description;
	VkAttachmentReference attachment_reference;
	VkSubpassDescription subpass_description;
	VkSubpassDependency subpass_dependency;
	VkRenderPassCreateInfo render_pass_info;
	VkGraphicsPipelineCreateInfo graphics_pipeline_infos[3];
	VkCommandPoolCreateInfo command_pool_info;
	VkCommandBufferAllocateInfo render_command_buffers_allocate_info;
	VkCommandBufferAllocateInfo general_purpose_command_buffer_allocate_info;
	VkSemaphoreCreateInfo semaphore_info;
	VkFenceCreateInfo inflight_fence_info;
			//
	VkDescriptorSet* descriptor_sets;
	VkDescriptorSetAllocateInfo descriptor_set_allocate_info;
	VkDescriptorPool descriptor_pool;
	VkDescriptorPoolCreateInfo descriptor_pool_info;
	VkDescriptorPoolSize descriptor_pool_sizes[2];
	VkDescriptorSetLayout main_descriptor_set_layout;
	VkDescriptorSetLayout* copy_descriptor_set_layouts;
	VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info;
	VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[2];
	VkDescriptorBufferInfo descriptor_buffer_info;
	VkDescriptorImageInfo* descriptor_combined_sampler_infos;
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
		
struct lightray_vulkan_vram_arena_suballocation_result_t
{
	u64 starting_offset = 0;
	lightray_vulkan_result result = LIGHTRAY_VULKAN_RESULT_FAILURE;
};

struct lightray_vulkan_move_entity_data_t
{
	lightray_vulkan_core_t* core = nullptr;
	lightray_scene_t* scene = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																																				EXTERNAL API FUNCTIONS																																												 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void																				lightray_vulkan_set_target_fps(lightray_vulkan_core_t* core, f32 desired_fps);
void																				lightray_vulkan_uncap_fps(lightray_vulkan_core_t* core);
bool																				lightray_vulkan_is_fps_capped(const lightray_vulkan_core_t* core);
void																				lightray_vulkan_setup_core(lightray_vulkan_core_t* core, lightray_vulkan_core_setup_data_t* setup_data);
void																				lightray_vulkan_terminate_core(lightray_vulkan_core_t* core);
void																				lightray_vulkan_set_fov(lightray_vulkan_core_t* core, f32 desired_fov);
void																				lightray_vulkan_hide_entity(lightray_vulkan_core_t* core, u32 entity_index, lightray_scene_t* scene);
void																				lightray_vulkan_unhide_entity(lightray_vulkan_core_t* core, u32 entity_index, lightray_scene_t* scene);
void																				lightray_vulkan_move_entity(lightray_vulkan_move_entity_data_t* move_data);
void																				lightray_vulkan_tick_core_begin(lightray_vulkan_core_t* core);
void																				lightray_vulkan_tick_core_end(lightray_vulkan_core_t* core);
void																				lightray_vulkan_set_relative_path(lightray_vulkan_core_t* core, cstring_literal* path);
u64																				lightray_vulkan_compute_required_user_chosen_arena_suballocation_size(u32 mesh_count, u32 texture_count, u32 alignment);
void																				lightray_vulkan_execute_main_render_pass(lightray_vulkan_core_t* core, u32 flags);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//																																				INTERNAL API FUNCTIONS																																												 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

lightray_vulkan_result													lightray_vulkan_allocate_vram_arena_debug(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena, const lightray_vulkan_vram_arena_allocation_data_t* allocation_data);

																					// returns a struct with a result and offset at which suballocation happend
																					// offset is UINT64_MAX on failure
lightray_vulkan_vram_arena_suballocation_result_t		lightray_vulkan_suballocate_from_vram_arena_debug(lightray_vulkan_vram_arena_t* arena, u64 suballocation_size);
lightray_vulkan_result													lightray_vulkan_allocate_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena, const lightray_vulkan_vram_arena_allocation_data_t* allocation_data);
lightray_vulkan_result													lightray_vulkan_suballocate_from_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena);
lightray_vulkan_result													lightray_vulkan_free_vram_arena(const lightray_vulkan_core_t* core, lightray_vulkan_vram_arena_t* arena);

																					// returns UINT32_MAX on failure
u32																				lightray_vulkan_get_graphics_queue_family_index(const VkQueueFamilyProperties* supported_queue_family_properties, const u32* swapchain_presentation_supported_queue_family_indices, u32 swapchain_presentation_supported_queue_family_index_count);

																					// returns UINT32_MAX on failure
u32																				lightray_vulkan_get_present_queue_family_index(const VkQueueFamilyProperties* supported_queue_family_properties, const u32* swapchain_presentation_supported_queue_family_indices, u32 swapchain_presentation_supported_queue_family_index_count);

u64																				lightray_vulkan_compute_vram_arena_required_metadata_allocation_size(u32 suballocation_count, u64 alignment);
																					// returns UINT32_MAX on failure
u32																				lightray_vulkan_get_vram_type_index(const lightray_vulkan_core_t* core, u32 vram_filter, VkMemoryPropertyFlags vram_property_flags);
u32																				lightray_vulkan_get_supported_swapchain_image_count(const VkSurfaceCapabilitiesKHR* surface_capabilities);
void																				lightray_vulkan_create_swapchain(lightray_vulkan_core_t* core);
void																				lightray_vulkan_free_swapchain(lightray_vulkan_core_t* core);
void																				lightray_vulkan_create_swapchain_framebuffers(lightray_vulkan_core_t* core);
void																				lightray_vulkan_free_swapchain_framebuffers(lightray_vulkan_core_t* core);
void																				lightray_vulkan_copy_buffer(lightray_vulkan_core_t* core, VkBuffer dst, VkBuffer src, u64 dst_offset, u64 src_offset, u64 size);

																					// returns VK_FORMAT_UNDEFINED on failure
VkFormat																		lightray_vulkan_get_supported_image_format(VkPhysicalDevice gpu, VkFormat* formats, u32 format_count, VkImageTiling tiling, VkFormatFeatureFlags features, VkImageUsageFlags usage, VkFormat prefered_format);
bool																				lightray_vulkan_is_format_supported(VkPhysicalDevice gpu, VkFormat format);

lightray_vulkan_result													lightray_vulkan_create_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader, VkShaderStageFlagBits stage_flags);
lightray_vulkan_result													lightray_vulkan_free_shader(lightray_vulkan_core_t* core, lightray_vulkan_shader_t* shader);

lightray_vulkan_result													lightray_vulkan_load_texture(cstring_literal* path, lightray_vulkan_texture_t* texture);
lightray_vulkan_result													lightray_vulkan_create_texture(lightray_vulkan_core_t* core, lightray_vulkan_texture_t* texture, const lightray_vulkan_texture_creation_data_t* creation_data);
void																				lightray_vulkan_transition_texture_layout(VkQueue queue, VkCommandBuffer command_buffer, const VkCommandBufferBeginInfo* command_buffer_begin_info, lightray_vulkan_texture_t* texture, VkImageLayout new_layout);
void																				lightray_vulkan_populate_texture(lightray_vulkan_texture_t* texture, const lightray_vulkan_texture_population_data_t* population_data);
lightray_vulkan_result													lightray_vulkan_free_texture(lightray_vulkan_core_t* core, lightray_vulkan_texture_t* texture, u8 free_flags);
lightray_vulkan_result													lightray_vulkan_allocate_vram_texture_arena_debug(lightray_vulkan_core_t* core, lightray_vulkan_vram_texture_arena_t* arena, u64 allocation_size, u64 alignment, u32 vram_type_index);
lightray_vulkan_result													lightray_vulkan_suballocate_texture(lightray_vulkan_core_t* core, lightray_vulkan_vram_texture_arena_t* arena, lightray_vulkan_texture_t* texture, u64 texture_size);
lightray_vulkan_result													lightray_vulkan_free_vram_texture_arena(lightray_vulkan_core_t* core, lightray_vulkan_vram_texture_arena_t* arena);

void																				lightray_vulkan_begin_command_buffer_recording(VkCommandBuffer command_buffer, const VkCommandBufferBeginInfo* begin_info);
void																				lightray_vulkan_end_command_buffer_recording(VkCommandBuffer command_buffer, VkQueue queue);