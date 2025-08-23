#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "lightray_core.h"
#include "lightray_entity_system.h"

#define LIGHTRAY_MAX_SUPPORTED_GPU_COUNT 1U
#define LIGHTRAY_MAIN_GPU_INDEX 0U
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
#define LIGHTRAY_MIN_VRAM_ARENA_ALLOCATION_SIZE 4096U
#define LIGHTRAY_UNIFORM_BUFFER_ALIGNMENT 256U
#define LIGHTRAY_VERTEX_BUFFER_ALIGNMENT 16U
#define LIGHTRAY_INDEX_BUFFER_ALIGNMENT 4U

/*
#define VULKAN_LIGHTRAY_INSTANCED_INDEXED_DEFAULT_PER_MESH_PASS
#define VULKAN_LIGHTRAY_INSTANCED_INDEXED_DEVELOPMENT_PER_MESH_PASS
#define VULKAN_LIGHTRAY_TEXTURED_PASS
#define VULKAN_LIGHTRAY_VERTEX_COLOR_PASS

#define VULKAN_LIGHTRAY_EXECUTE_MAIN_RENDER_PASS_PER_MESH(passes)

VULKAN_LIGHTRAY_EXECUTE_MAIN_RENDER_PASS_PER_MESH(VULKAN_LIGHTRAY_VERTEX_COLOR_PASS VULKAN_LIGHTRAY_INSTANCED_INDEXED_DEVELOPMENT_PER_MESH_PASS);


*/

namespace lightray
{
	namespace vulkan
	{	
		typedef VkBuffer vram_buffer_description_t;
		typedef VkDeviceMemory vram_buffer_t;

		enum vram_arena_type
		{
			VRAM_ARENA_TYPE_UNDEFINED = 0,
			VRAM_ARENA_TYPE_BUFFER,
			VRAM_ARENA_TYPE_IMAGE
		};

		struct vram_arena_t
		{
			vram_buffer_t vram_buffer = nullptr;
			vram_arena_type arena_type;

			union
			{
				VkImage image;
				vram_buffer_description_t vram_buffer_description;
			};

			union
			{
				VkBufferCreateInfo* vram_buffer_description_create_info;
				VkImageCreateInfo* image_create_info;
			};

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

		struct vram_arena_allocation_data_t
		{
			u32 suballocation_count = 0;
			u32 metadata_allocation_size = 0;
			u32 custom_alignment = 0;
			VkMemoryPropertyFlags vram_properties = 0;
			vram_arena_type arena_type;
			sunder::dynamic_alignment_arena_t* metadata_arena = nullptr;

			union
			{
				VkImageCreateInfo* image_create_info;
				VkBufferCreateInfo* vram_buffer_description_create_info;
			};
		};

		// will be used (maybe) for making texture vram allocation and the later use of them easier
		struct texture_family_t
		{

		};

		// not used 
		struct image_description_t
		{
			const char* path = nullptr;
			// format, tiling and other bullshit
		};

		struct game_resources_setup_data_t
		{
			u32* wireframe_mesh_count_buffer = nullptr;
			u32* instance_count_buffer = nullptr;
			u32 mesh_count = 0;
			const char** mesh_path_buffer = nullptr;
			const char** texture_path_buffer = nullptr;
		};

		struct vram_texture_arena_t
		{
			vram_buffer_t vram_buffer = nullptr;

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
		struct core_t
		{
			////////////////////////////////////////// game resources ///////////////////////////////////////
			vram_arena_t host_visible_vram_arena;
			vram_arena_t device_local_vram_arena;
			vram_arena_t image_vram_arena;
			sunder::dynamic_alignment_arena_t general_purpose_ram_arena;
			sunder::string_buffer_t string_buffer;
			vram_arena_allocation_data_t host_visible_vram_arena_allocation_data;
			vram_arena_allocation_data_t device_local_vram_arena_allocation_data;
			vram_arena_allocation_data_t image_vram_arena_allocation_data;
			VkBufferCreateInfo host_visible_vram_arena_buffer_description;
			VkBufferCreateInfo device_local_vram_arena_buffer_description;
			VkImageCreateInfo image_vram_arena_description;
			u64* host_visible_vram_arena_suballocation_starting_offsets;
			u64* device_local_vram_arena_suballocation_starting_offsets;
			u32 host_visible_vram_arena_suballocation_starting_offset_count;
			u32 device_local_vram_arena_suballocation_starting_offset_count;

			///////////////// depth buffering related //////////////////
			VkImage depth_image;
			VkImageView depth_image_view;
			VkDeviceMemory depth_image_vram;
			///////////////// depth buffering related //////////////////

			////////////////////////////////////////////////// camera////////////////////////////////////////////

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// just have a second orthographic camera to render 2d stuff with a different pipeline in a different render pass																						 //
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			cvp_t cvp;
			f32 yaw;
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
			f32 fov;
			////////////////////////////////////////////////// camera////////////////////////////////////////////

			////////////////////////////////////////// game resources ///////////////////////////////////////
			u32* wireframe_mesh_count_buffer;

			u32* cpu_side_index_count_buffer;
			u32* cpu_side_instance_model_count_buffer;
			u32* cpu_side_instance_model_to_render_count_buffer;

			u64* cpu_side_vertex_buffer_offsets;
			u32* cpu_side_index_buffer_offsets;
			u32* cpu_side_instance_model_buffer_offsets_per_mesh;
			u64 cpu_side_vertex_buffer_offset_count;
			u32 cpu_side_index_buffer_offset_count;
			u32 mesh_count;

			vertex_t* cpu_side_vertex_buffer;
			u32* cpu_side_index_buffer;
			u32 index_count;
			u64 vertex_count;
			f32 delta_time;
			VkImageView texture_image_view;
			const char* texture_path;
			u32 cpu_side_instance_count;
			u32 cpu_side_instance_model_buffer_count;
			model_t* cpu_side_instance_model_buffer;
			model_t* cpu_side_hidden_instance_model_buffer;

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
			u32 target_fps;
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
			VkShaderModule vertex_shader_module;
			VkShaderModule fragment_shader_module;
			VkShaderModuleCreateInfo vertex_shader_module_info;
			VkShaderModuleCreateInfo fragment_shader_module_info;
			VkPipelineShaderStageCreateInfo pipeline_vertex_shader_stage_info;
			VkPipelineShaderStageCreateInfo pipeline_fragment_shader_stage_info;
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
			VkCommandBufferAllocateInfo copy_command_buffer_allocate_info;
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
			void* cpu_side_host_visible_vram_arena_view;
			//
			VkCommandBufferBeginInfo render_command_buffers_begin_info;
			VkCommandBufferBeginInfo copy_command_buffer_begin_info;
			VkRenderPassBeginInfo render_pass_begin_info;
			VkRenderPass render_pass;
			VkFramebufferCreateInfo swapchain_framebuffer_info;
			VkClearValue clear_values[2];
			VkSubmitInfo submit_info;
			VkPresentInfoKHR present_info;
			VkCommandBuffer copy_command_buffer;
			VkCommandBuffer* render_command_buffers;
			VkImage* swapchain_images;
			VkImageView* swapchain_image_views;
			VkImageViewCreateInfo swapchain_image_view_info;
			VkFramebuffer* swapchain_framebuffers;
			i8* vertex_shader_byte_code; 
			i8* fragment_shader_byte_code;
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
			flags32 flags;
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

		enum core_result
		{
			CORE_RESULT_SUCCESS = 0,
			CORE_RESULT_FAILED_TO_LOAD_TEXTURE,
			CORE_RESULT_FAILED_TO_LOAD_MESH
		};

		enum vram_arena_result
		{
			VRAM_ARENA_ACTION_RESULT_SUCCESS = 0,
			VRAM_ARENA_ACTION_RESULT_INVALID_METADATA_ALLOCATION_SIZE,
			VRAM_ARENA_ACTION_RESULT_OUT_OF_VRAM,
			VRAM_ARENA_ACTION_RESULT_FAILED_TO_CREATE_DESCRIPTOR_BUFFER,
			VRAM_ARENA_ACTION_RESULT_FAILED_TO_BIND_BUFFER,
			VRAM_ARENA_ACTION_RESULT_OUT_OF_METADATA_RAM,
			VRAM_ARENA_ACTION_RESULT_SUBALLOCATION_SIZE_IS_0,
			VRAM_ARENA_ACTION_RESULT_VRAM_ARENA_ALLOCATION_SIZE_NEEDS_TO_BE_ATLEAST_4096_BYTES,
			VRAM_ARENA_ACTION_RESULT_UNKOWN_FAILURE
		};
		
		struct vram_arena_suballocation_result_t
		{
			u64 starting_offset = 0;
			vram_arena_result result = VRAM_ARENA_ACTION_RESULT_UNKOWN_FAILURE;
		};

		struct move_entity_data_t
		{
			core_t* core = nullptr;
			es::scene_t* scene = nullptr;
		};

		vram_arena_result allocate_vram_arena_debug(const core_t* core, vram_arena_t* arena, const vram_arena_allocation_data_t* allocation_data);
		// returns a struct with a result and offset at which suballocation happend
		// offset is UINT64_MAX on failure
		vram_arena_suballocation_result_t suballocate_from_vram_arena_debug(vram_arena_t* arena, u64 suballocation_size);
		vram_arena_result allocate_vram_arena(const core_t* core, vram_arena_t* arena, const vram_arena_allocation_data_t* allocation_data, sunder::dynamic_alignment_arena_t* metadata_arena);
		vram_arena_result suballocate_from_vram_arena(const core_t* core, vram_arena_t* arena);
		vram_arena_result free_vram_arena(const core_t* core, vram_arena_t* arena);

		void setup_core(core_t* core, game_resources_setup_data_t* game_resources_setup_data, u32 window_width, u32 window_height, cstring_literal* window_title, u32 target_fps, bool fulscreen_mode);
		void terminate_core(core_t* core);
		void log_enabled_instance_extensions(const core_t* core);
		void log_enabled_instance_layers(const core_t* core);
		void log_gpu_properties(const core_t* core);
		void log_queue_family_properties(const core_t* core);
		void log_swapchain_presentation_supported_queue_family_indices(const core_t* core);
		void log_available_device_extensions(const core_t* core);
		void log_enabled_device_extensions(const core_t* core);
		void log_surface_capabilities(const core_t* core);
		void log_surface_formats_properties(const core_t* core);
		void log_surface_present_modes_properties(const core_t* core);
		// returns UINT32_MAX on failure
		u32 get_graphics_family_queue_index(const core_t* core);
		// returns UINT32_MAX on failure
		u32 get_present_family_queue_index(const core_t* core);
		// returns UINT32_MAX on failure
		u32 get_vram_type_index(const core_t* core, u32 vram_filter, VkMemoryPropertyFlags vram_property_flags);
		u32 get_vram_arena_required_metadata_allocation_size(const vram_arena_allocation_data_t* allocation_data);
		void recreate_swapchain(core_t* core);
		void write_to_vram_buffer(const core_t* core, vram_buffer_t vram_buffer, u64 vram_buffer_starting_offset, u64 bytes_to_write, const void* data_to_write);
		void get_supported_swapchain_image_count(core_t* core);
		void create_swapchain(core_t* core);
		void free_swapchain(core_t* core);
		void create_swapchain_framebuffers(core_t* core);
		void free_swapchain_framebuffers(core_t* core);
		void execute_instanced_render_pass_per_mesh(core_t* core);
		void copy_vram_buffer(core_t* core, vram_buffer_description_t dst, vram_buffer_description_t src, u64 dst_offset, u64 src_offset, u64 size);
		void create_vram_buffer_description(const core_t* core, vram_buffer_description_t* buffer_description, VkBufferUsageFlags buffer_usage, u64 buffer_size);
		void destroy_vram_buffer_description(const core_t* core, vram_buffer_description_t buffer_description);
		void transition_image_layout(core_t* core, VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout);
		void copy_buffer_to_image(core_t* core, VkBuffer buffer, u32 texture_width, u32 texture_height, u64 buffer_offset, VkOffset3D image_offset);
		void set_target_fps(core_t* core, u32 fps);
		void uncap_fps(core_t* core);
		bool is_fps_capped(const core_t* core);
		void tick_core(core_t* core, es::scene_t* scene);
		VkFormat get_supported_format(const core_t* core, VkFormat* formats, u32 format_count, VkImageTiling tiling, VkFormatFeatureFlags features);
		VkFormat get_supported_depth_format(const core_t* core);
		bool contains_stencil_component(VkFormat format);
		void create_depth_resources(core_t* core);
		void free_depth_resources(core_t* core);
		void set_fov(core_t* core, degrees desired_fov);
		void hide_entity(core_t* core, index entity_index, es::scene_t* scene);
		void unhide_entity(core_t* core, index entity_index, es::scene_t* scene);
		void move_entity(move_entity_data_t* move_data);
		void tick_core_begin(core_t* core);
		void tick_core_end(core_t* core);
	} // vulkan namespace brace
} // lightray namespace brace