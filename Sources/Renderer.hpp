#pragma once

#include "Common/Types.hpp"
#include "Camera.hpp"
#include "Vertex3D.hpp"
#include "Entity.hpp"
#include "CycleTimer.hpp"
#include "MappedBuffer.hpp"
#include "UnmappedBuffer.hpp"
#include "UniformBufferObject.hpp"
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.hpp>
#include <array>	//std::array
#pragma warning(pop)


namespace API_NAME {
	class Renderer {
	public:
		Renderer(HWND window_handle);
		~Renderer();
		Renderer(Renderer const&) = delete;
		Renderer(Renderer&&) = delete;
		void operator=(Renderer const&) = delete;
		void operator=(Renderer&&) = delete;

		void drawFrame();
		void awaitClose();

		void togglePaused() noexcept;

		Camera& camera() noexcept;

		bool resized;
		

	private:
		void RecreateSwapchain();
		void RecordCommandBuffer(VkCommandBuffer command_buffer, u32 const image_index);

#ifdef _DEBUG
		std::array<char const* const, 1> static constexpr LAYERS{ {
			"VK_LAYER_KHRONOS_validation",
		} };
		std::array<char const* const, 3> static constexpr EXTENSIONS{ {
			"VK_KHR_win32_surface",
			"VK_KHR_surface",
			"VK_EXT_debug_utils",
		} };

#else 
		std::array<char const* const, 2> static constexpr EXTENSIONS{ {
			"VK_KHR_win32_surface",
			"VK_KHR_surface",
		} };
#endif
		std::array<char const* const, 1> static constexpr DEVICE_EXTENSIONS{ "VK_KHR_swapchain" };
		u32 static constexpr FRAMES_IN_FLIGHT = 2;
		u32 static constexpr MODEL_COUNT = 2;
		std::array<VkFormat, 2> static constexpr ACCEPTABLE_FORMATS{ VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB };

		VkInstance m_Instance;
#ifdef _DEBUG
		VkDebugUtilsMessengerEXT m_DebugMessenger;
#endif
		bool m_Paused;
		CycleTimer m_FrameTimer;
		Camera m_Camera;
		u64 m_FrameCounter;
		
		std::array<Entity, 2> m_Entities;
		std::array<UniformBufferObject, 2> m_UBOs;
		VkPhysicalDevice m_PhysicalDevice;
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
		VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
		VkDevice m_LogicalDevice;
		u32 m_QueueFamilyIndex; /*m_QueueFamilyIndices*/
		VkQueue m_GraphicsQueue;
		VkExtent2D m_Extent;
		VkSurfaceKHR m_Surface;
		VkFormat m_SurfaceFormat;
		VkSwapchainKHR m_Swapchain;
		std::vector<VkImage> m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;
		VkViewport m_Viewport;
		VkRect2D m_Scissor;
		VkRenderPass m_RenderPass;
		VkDescriptorSetLayout m_DescriptorSetLayout;
		VkPipelineLayout m_GraphicsPipelineLayout;
		VkPipeline m_GraphicsPipeline;
		std::vector<VkFramebuffer> m_Framebuffers;
		VkCommandPool m_ResettableCommandPool;
		VkCommandPool m_DestructibleCommandPool;
		VkImage m_DepthBufferImage;
		VkDeviceMemory m_DepthBufferMemory;
		VkImageView m_DepthBufferView;
		UnmappedBuffer m_VertexBuffer;
		UnmappedBuffer m_IndexBuffer;
		MappedBuffer m_PerInstanceBuffer;
		VkDescriptorPool m_DescriptorPool;
		std::array<VkDescriptorSetLayout, FRAMES_IN_FLIGHT> m_DescriptorSetLayouts;
		std::array<VkDescriptorSet, FRAMES_IN_FLIGHT> m_DescriptorSets;
		std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> m_DrawingCommandBuffers;
		std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_ImageAvailableSemaphores;
		std::array<VkSemaphore, FRAMES_IN_FLIGHT> m_RenderFinishedSemaphores;
		std::array<VkFence, FRAMES_IN_FLIGHT> m_Fences;
		u32 m_CurrentFrameIndex;
	};
}