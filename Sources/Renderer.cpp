#include "Common/Util.hpp"
#include "Common/EnumStrings.hpp"
#include "Image.hpp"
#include "Renderer.hpp"
#include "ShaderProgram.hpp"
#include "UnmappedBuffer.hpp"
#include "AssetManagementSystem.hpp"
#include "PipelineShaderStageCreateInfos.hpp"
#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <array>
#include <iostream>
#include <vector>
#include <limits>
#include <ranges>
#include <algorithm>
#include <type_traits>
#pragma warning(pop)

namespace API_NAME {

	Camera& Renderer::camera() {
		return m_Camera;
	}
	void Renderer::drawFrame() {
		(void)m_FrameTimer.update();
		auto const frame_time_ns = m_FrameTimer.getCycleTime();
		f32 frame_time = static_cast<f32>(frame_time_ns.count()) / 10000000.0f;

#ifdef _DEBUG
		if ((++m_FrameCounter % 100) == 0) {
			SPDLOG_INFO("Frame Time: {}ms", std::chrono::duration_cast<std::chrono::milliseconds>(frame_time_ns).count());
		}
#endif

		VkResult result;

		/* Change Entity  */ {
			if (!m_Paused) {
				m_Entities[0].rotate(glm::vec3(0.0f, 0.0f, 0.01f));
				m_Entities[1].rotate(glm::vec3(0.0f, 0.0f, 0.01f));

				u08* mapped = reinterpret_cast<u08*>(m_PerInstanceBuffer.mapped());
				for (auto [i, entity] : std::views::enumerate(m_Entities)) {
					(void)std::memcpy(mapped + sizeof(EntityInstanceData) * i, &entity.instance_data(), sizeof(EntityInstanceData));
				}
				
				for (auto& ubo : m_UBOs) {
					auto& uniform = ubo.uniform();
					uniform.view = glm::lookAt(m_Camera.translation(), m_Camera.translation() - glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f));
					uniform.projection = glm::perspective(glm::radians(45.0f), static_cast<f32>(m_Extent.width) / static_cast<f32>(m_Extent.height), 0.1f, 1000.0f);
					(void)std::memcpy(ubo.mapped_buffer().mapped(), &ubo.uniform(), ubo.mapped_buffer().size());
				}
			}
		}

		/* Draw Frame */ {
			result = vkWaitForFences(m_LogicalDevice, 1, &m_Fences[m_CurrentFrameIndex], VK_TRUE, std::numeric_limits<u64>::max());
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkWaitForFences failed execution: {}", getEnumString(result));
				return;
			}
#endif

			u32 image_index;
			result = vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain, std::numeric_limits<u64>::max(), m_ImageAvailableSemaphores[m_CurrentFrameIndex], VK_NULL_HANDLE, &image_index);
#pragma warning(push)
#pragma warning(disable : 4061)
			switch (result) {
			case VK_ERROR_OUT_OF_DATE_KHR:
				if (resized) {
					RecreateSwapchain();
					resized = false;
				}
				break;
			case VK_SUBOPTIMAL_KHR:
				break;
			case VK_SUCCESS:
				break;
#ifdef _DEBUG
			default:
				SPDLOG_ERROR("vkAcquireNextImageKHR failed fetch: {}", getEnumString(result));
				return;
#endif
			}
#pragma warning(pop)

			result = vkResetFences(m_LogicalDevice, 1, &m_Fences[m_CurrentFrameIndex]);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkResetFences failed execution: {}", getEnumString(result));
				return;
			}
#endif

			result = vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrameIndex], 0);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkResetCommandBuffer failed reset: {}", getEnumString(result));
				return;
			}
#endif
			RecordCommandBuffer(m_CommandBuffers[m_CurrentFrameIndex], image_index);

			std::array<VkSemaphore, 1> const wait_semaphores{ { m_ImageAvailableSemaphores[m_CurrentFrameIndex] } };
			std::array<VkSemaphore, 1> const signal_semaphores{ { m_RenderFinishedSemaphores[m_CurrentFrameIndex] }
			};
			std::array<VkPipelineStageFlags, 1> static constexpr wait_stages{ { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT } };
			VkSubmitInfo const submit_info{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.pNext = nullptr,
				.waitSemaphoreCount = static_cast<u32>(std::size(wait_semaphores)),
				.pWaitSemaphores = std::data(wait_semaphores),
				.pWaitDstStageMask = std::data(wait_stages),
				.commandBufferCount = 1,
				.pCommandBuffers = &m_CommandBuffers[m_CurrentFrameIndex],
				.signalSemaphoreCount = static_cast<u32>(std::size(signal_semaphores)),
				.pSignalSemaphores = std::data(signal_semaphores)
			};
			result = vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, m_Fences[m_CurrentFrameIndex]);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkQueueSubmit failed submission: {}", getEnumString(result));
				return;
			}
#endif

			VkPresentInfoKHR const present_info{
				.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
				.pNext = nullptr,
				.waitSemaphoreCount = static_cast<u32>(std::size(signal_semaphores)),
				.pWaitSemaphores = std::data(signal_semaphores),
				.swapchainCount = 1,
				.pSwapchains = &m_Swapchain,
				.pImageIndices = &image_index,
				.pResults = nullptr /* use if multiple swapchains are in use */
			};
			result = vkQueuePresentKHR(m_GraphicsQueue, &present_info);
#pragma warning(push)
#pragma warning(disable: 4061)
			switch (result) {
			case VK_ERROR_OUT_OF_DATE_KHR:
				break;
			case VK_SUBOPTIMAL_KHR:
				if (resized) {
					RecreateSwapchain();
					resized = false;
				}
				break;
			case VK_SUCCESS:
				break;
#ifdef _DEBUG
			default:
				SPDLOG_ERROR("vkQueuePresentKHR failed presentation: {}", getEnumString(result));
				return;
#endif
			}
#pragma warning(pop)

			m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % FRAMES_IN_FLIGHT;
		}
	}
	void Renderer::awaitClose() {
		VkResult result;
		result = vkDeviceWaitIdle(m_LogicalDevice);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkDeviceWaitIdle failed... waiting: {}", getEnumString(result));
		}
#endif
	}

	void Renderer::togglePaused() noexcept {
		m_Paused = !m_Paused;
	}

#ifdef _DEBUG
	static u32 __stdcall debugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT severity, [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT type, [[maybe_unused]] VkDebugUtilsMessengerCallbackDataEXT const* callback_data, [[maybe_unused]] void* user_data) {
		/* Utilize callback_data->cmdBufLabels when command buffers are setup */
#pragma warning(push)
#pragma warning(disable : 4061 4062)
		switch (severity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			SPDLOG_TRACE(callback_data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			SPDLOG_INFO(callback_data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			SPDLOG_WARN(callback_data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			SPDLOG_ERROR(callback_data->pMessage);
			break;
		default:
			break;
		}
#pragma warning(pop)

		return VK_FALSE;
	}
#endif

	Renderer::Renderer(HWND window_handle) : resized{ false }, m_Paused{ false }, m_Camera(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f)), m_FrameCounter{ 0 }, m_CurrentFrameIndex { 0 } {
#ifdef _DEBUG
		spdlog::set_level(spdlog::level::debug);
#endif
		VkResult result;

		/* Asset Loading */ {
			AMS.insertMesh(std::filesystem::path{ "C:/SE/Personal/VulkanBoiler/VulkanBoiler/Resources/Meshes/VikingRoom.obj" }, "Viking Room");
			auto& image0 = AMS.insertImage(std::filesystem::path{ "C:/SE/Personal/VulkanBoiler/VulkanBoiler/Resources/Textures/World/VikingRoom.png" }, "Viking Room 0");
			//auto pixels = image0.pixels();
			//for (u32 i = 0; i < image0.size() / 4; ++i) {
			//	Pixel pixel;
			//	(void)std::memcpy(&pixel, &pixels[i * 4], 4);
			//	pixel.r = 255 - pixel.r;
			//	pixel.g = 255 - pixel.g;
			//	pixel.b = 255 - pixel.b;
			//	(void)std::memcpy(&pixels[i * 4], &pixel, 4);
			//}
			auto& image1 = AMS.insertImage(std::filesystem::path{ "C:/SE/Personal/VulkanBoiler/VulkanBoiler/Resources/Textures/World/VikingRoom.png" }, "Viking Room 1");
			auto& layered_texture = AMS.insertLayeredTexture("Viking Room");
			layered_texture.addImage("Viking Room 0", 0);
			layered_texture.addImage("Viking Room 1", 1);

			SPDLOG_DEBUG("Asset Loading Finished");
		}

		/* Entities */ {
			m_Entities[0] = { "Viking Room 0", "Viking Room", "Viking Room", glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0 };
			m_Entities[1] = { "Viking Room 1", "Viking Room", "Viking Room", glm::vec3( 2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.5f, 1.5f, 1.5f), 1 };
		}

		/* Instance Creation */ {
			VkApplicationInfo static constexpr app_info{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
					.pNext{},
					.pApplicationName = "Vulkan Boiler",
					.applicationVersion = VK_MAKE_VERSION(0, 0, 0),
					.pEngineName = "Vulkan Boiler",
					.engineVersion = VK_MAKE_VERSION(0, 0, 0),
					.apiVersion = VK_API_VERSION_1_3,
			};

			VkInstanceCreateInfo static constexpr instance_create_info{
				.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					.pNext{},
					.flags{},
					.pApplicationInfo = &app_info,
#ifdef _DEBUG
					.enabledLayerCount = static_cast<u32>(LAYERS.size()),
					.ppEnabledLayerNames = LAYERS.data(),
#else
					.enabledLayerCount{},
					.ppEnabledLayerNames{},
#endif
					.enabledExtensionCount = static_cast<u32>(EXTENSIONS.size()),
					.ppEnabledExtensionNames = EXTENSIONS.data()
			};
			result = vkCreateInstance(&instance_create_info, nullptr, &m_Instance);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateInstance failed creation: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Instance Creation Finished");
		}

		/* Debug Messenger Creation */ {
#ifdef _DEBUG
			VkDebugUtilsMessengerCreateInfoEXT static constexpr debug_messenger_create_info{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.pNext = nullptr,
				.flags{},
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT,
				.pfnUserCallback = debugCallback,
				.pUserData = nullptr,
			};
#pragma warning(push)
#pragma warning(disable: 4191)
			auto _vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));
#pragma warning(pop)
			if (_vkCreateDebugUtilsMessengerEXT == nullptr) {
				SPDLOG_ERROR("vkGetInstanceProcAddr failed to fetch \"vkCreateDebugUtilsMessengerEXT\"");
			}
			result = _vkCreateDebugUtilsMessengerEXT(m_Instance, &debug_messenger_create_info, nullptr, &m_DebugMessenger);
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateDebugUtilsMessengerEXT failed debug messenger creation: {}", getEnumString(result));
			}

			SPDLOG_DEBUG("Debug Messenger Creation Finished");
#endif
		}

		/* Physical Device Selection */ {
			u32 physical_device_count = 0;
			result = vkEnumeratePhysicalDevices(m_Instance, &physical_device_count, nullptr);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkEnumeratePhysicalDevices failed count fetch: {}", getEnumString(result));
			}
			if (physical_device_count == 0) {
				SPDLOG_ERROR("vkEnumeratePhysicalDevices fetched 0 physical devices.");
			}
#endif
			std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
			result = vkEnumeratePhysicalDevices(m_Instance, &physical_device_count, physical_devices.data());
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkEnumeratePhysicalDevices failed fetch: {}", getEnumString(result));
			}
#endif
			/* When moving to dedicated, select proper device */
			m_PhysicalDevice = physical_devices[0];
			vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDeviceFeatures);
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);

			SPDLOG_DEBUG("Physical Device Selection Finished");
		}

		/* Queue Families Indices */ { /* remember presentation queue. other devices might have multiple queue families needed */
			u32 queue_family_property_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_property_count, nullptr);
			std::vector<VkQueueFamilyProperties> queue_families(queue_family_property_count);
			vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queue_family_property_count, std::data(queue_families));
			m_QueueFamilyIndex = std::numeric_limits<u32>::max();
			for (auto [i, queue_family_properties] : std::views::enumerate(queue_families)) {
				if (queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					m_QueueFamilyIndex = i;
					break;
				}
			}

			SPDLOG_DEBUG("Queue Family Indices Finished");
		}

		/* Logical Device Creation */ {
			float priority = 1.0f;
			VkDeviceQueueCreateInfo const logical_device_queue_create_info{
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.pNext{},
				.flags{},
				.queueFamilyIndex = m_QueueFamilyIndex,
				.queueCount = 1,
				.pQueuePriorities = &priority,
			};
			/* Set extension and layer attributes in older Vulkan versions */
			VkDeviceCreateInfo const logical_device_create_info{
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext{},
				.flags{},
				.queueCreateInfoCount = 1,
				.pQueueCreateInfos = &logical_device_queue_create_info,
				.enabledLayerCount{},
				.ppEnabledLayerNames{},
				.enabledExtensionCount = static_cast<u32>(DEVICE_EXTENSIONS.size()),
				.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
				.pEnabledFeatures = &m_PhysicalDeviceFeatures
			};
			result = vkCreateDevice(m_PhysicalDevice, &logical_device_create_info, nullptr, &m_LogicalDevice);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateDevice failed logical device creation: {}", getEnumString(result));
			}
#endif

			vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndex, 0, &m_GraphicsQueue);

			SPDLOG_DEBUG("Logical Device Creation Finished");
		}

		/* Surface Creation */ {
			VkWin32SurfaceCreateInfoKHR const win32_surface_create_info{
				.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
				.pNext = nullptr,
				.flags{},
				.hinstance = GetModuleHandleW(nullptr),
				.hwnd = window_handle
			};
			result = vkCreateWin32SurfaceKHR(m_Instance, &win32_surface_create_info, nullptr, &m_Surface);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateWin32SurfaceKHR failed to create vulkan surface from win32 surface: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Surface Creation Finished");
		}

		/* Swapchain Creation */ {
			u32 surface_format_count = 0;
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &surface_format_count, nullptr);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR failed count fetch: {}", getEnumString(result));
			}
#endif
			std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
			result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &surface_format_count, surface_formats.data());
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR failed fetch: {}", getEnumString(result));
			}
#endif

			u32 surface_present_mode_count = 0;
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &surface_present_mode_count, nullptr);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR failed count fetch: {}", getEnumString(result));
			}
#endif
			std::vector<VkPresentModeKHR> surface_present_modes(surface_present_mode_count);
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &surface_present_mode_count, surface_present_modes.data());
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR failed fetch: {}", getEnumString(result));
			}
#endif

			VkSurfaceCapabilitiesKHR surface_capabilities;
			result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &surface_capabilities);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed fetch: {}", getEnumString(result));
			}
#endif

			static constexpr u32 CHOSEN_SURFACE_FORMAT_INDEX = 3; /* definitely review */
			VkSurfaceFormatKHR const surface_format = surface_formats[CHOSEN_SURFACE_FORMAT_INDEX];
			m_SurfaceFormat = surface_format.format;
			//VkPresentModeKHR const surface_present_mode = VK_PRESENT_MODE_FIFO_KHR; /* v-sync */
			m_Extent = surface_capabilities.currentExtent.width == std::numeric_limits<u32>::max() || surface_capabilities.currentExtent.height == std::numeric_limits<u32>::max() ? VkExtent2D{ std::clamp(800U, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width), std::clamp(600U, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height) } : surface_capabilities.currentExtent;
			u32 const min_image_count = surface_capabilities.minImageCount + 1;

			VkSwapchainCreateInfoKHR const swapchain_create_info{
				.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.pNext = nullptr,
				.flags{},
				.surface = m_Surface,
				.minImageCount = min_image_count,
				.imageFormat = m_SurfaceFormat,
				.imageColorSpace = surface_format.colorSpace,
				.imageExtent = m_Extent,
				.imageArrayLayers = 1,
				.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount{},
				.pQueueFamilyIndices{},
				.preTransform = surface_capabilities.currentTransform,
				.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = VK_PRESENT_MODE_FIFO_KHR, /* verify */
				.clipped = VK_TRUE,
				.oldSwapchain = VK_NULL_HANDLE,	/* used for swapchain recreation */
			};
			result = vkCreateSwapchainKHR(m_LogicalDevice, &swapchain_create_info, nullptr, &m_Swapchain);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateSwapchainKHR failed creation: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Swapchain Creation Finished");
		}

		/* Image Views */ {
			u32 image_count = 0;
			result = vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &image_count, nullptr);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetSwapchainImagesKHR failed count fetch: {}", getEnumString(result));
			}
#endif
			std::vector<VkImage> images(image_count);
			result = vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &image_count, images.data());
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetSwapchainImagesKHR failed fetch: {}", getEnumString(result));
			}
#endif

			m_ImageViews.resize(image_count);
			for (auto [image_view, image] : std::views::zip(m_ImageViews, images)) {
				std::tie(result, image_view) = createImageView(m_LogicalDevice, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("createImageView failed image view creation: {}", getEnumString(result));
				}
#endif
			}

			SPDLOG_DEBUG("Image View Creation Finished");
		}

		/* Descriptor Set Layout */ {
			std::array<VkDescriptorSetLayoutBinding, 2> static constexpr descriptor_set_layout_bindings{ {
				/* UBO */ {
					.binding = 0,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.pImmutableSamplers = nullptr },
				/* Texture Sampler */ {
					.binding = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = 1,
					.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
					.pImmutableSamplers = nullptr }
			} };
			VkDescriptorSetLayoutCreateInfo static constexpr descriptor_set_layout_create_info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
				.pNext{},
				.flags{},
				.bindingCount = static_cast<u32>(std::size(descriptor_set_layout_bindings)),
				.pBindings = std::data(descriptor_set_layout_bindings)
			};
			result = vkCreateDescriptorSetLayout(m_LogicalDevice, &descriptor_set_layout_create_info, nullptr, &m_DescriptorSetLayout);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateDescriptorSetLayout failed descriptor set layout creation: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Descriptor Set Layout Creation Finished");
		}

		/* Pipeline */ {
			ShaderProgram vertex_shader_program("Resources/Shaders/DefaultShader.vert.cso");
			ShaderProgram fragment_shader_program("Resources/Shaders/DefaultShader.frag.cso");
			auto const vertex_shader_code = vertex_shader_program.getShaderCode();
			auto const fragment_shader_code = fragment_shader_program.getShaderCode();

			VkShaderModuleCreateInfo const vertex_shader_module_create_info{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext{},
				.flags{},
				.codeSize = vertex_shader_code.size() * sizeof(u32),
				.pCode = vertex_shader_code.data()
			};
			VkShaderModuleCreateInfo const fragment_shader_module_create_info{
				.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
				.pNext{},
				.flags{},
				.codeSize = fragment_shader_code.size() * sizeof(u32),
				.pCode = fragment_shader_code.data()
			};

			VkShaderModule vertex_shader_module, fragment_shader_module;
			result = vkCreateShaderModule(m_LogicalDevice, &vertex_shader_module_create_info, nullptr, &vertex_shader_module);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateShaderModule failed creation: {}", getEnumString(result));
			}
#endif
			result = vkCreateShaderModule(m_LogicalDevice, &fragment_shader_module_create_info, nullptr, &fragment_shader_module);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateShaderModule failed creation: {}", getEnumString(result));
			}
#endif
			PipelineShaderStageCreateInfos const shader_stage_create_infos{
			.vertex_stage_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext{},
				.flags{},
				.stage = VK_SHADER_STAGE_VERTEX_BIT,
				.module = vertex_shader_module,
				.pName = "main",
				.pSpecializationInfo{}
			},
			.fragment_stage_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.pNext{},
				.flags{},
				.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
				.module = fragment_shader_module,
				.pName = "main",
				.pSpecializationInfo{}
			}
			};
			std::array<VkVertexInputBindingDescription, 2> vertex_binding_descriptions{ {
				Vertex3D::BINDING_DESCRIPTION,
				{
					.binding = 1,
					.stride = sizeof(glm::vec3) * 3 + sizeof(u32),
					.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
				}
			} };
			std::array<VkVertexInputAttributeDescription, 7> vertex_attribute_descriptions{ {
				Vertex3D::ATTRIBUTE_DESCRIPTION[0], Vertex3D::ATTRIBUTE_DESCRIPTION[1], Vertex3D::ATTRIBUTE_DESCRIPTION[2],
				{ .location = 3, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0 },											/* Translation */
				{ .location = 4, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = static_cast<u32>(sizeof(glm::vec3)) },		/* Rotation */
				{ .location = 5, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = static_cast<u32>(sizeof(glm::vec3) * 2) },	/* Scale */
				{ .location = 6, .binding = 1, .format = VK_FORMAT_R32_UINT, .offset = static_cast<u32>(sizeof(glm::vec3) * 3) }			/* Texture Index */
			} };
			VkPipelineVertexInputStateCreateInfo const vertex_input_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
				.pNext = nullptr,
				.flags{},
				.vertexBindingDescriptionCount = static_cast<u32>(std::size(vertex_binding_descriptions)),
				.pVertexBindingDescriptions = std::data(vertex_binding_descriptions),
				.vertexAttributeDescriptionCount = static_cast<u32>(std::size(vertex_attribute_descriptions)),
				.pVertexAttributeDescriptions = std::data(vertex_attribute_descriptions)
			};
			VkPipelineInputAssemblyStateCreateInfo const input_assembly_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				.primitiveRestartEnable = VK_FALSE
			};
			m_Viewport = {
				.x = 0,
				.y = 0,
				.width = static_cast<f32>(m_Extent.width),
				.height = static_cast<f32>(m_Extent.height),
				.minDepth = 0.f,
				.maxDepth = 1.f
			};
			m_Scissor = {
				.offset{},
				.extent = m_Extent
			};

			VkPipelineViewportStateCreateInfo const viewport_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.viewportCount = 1,
				.pViewports = &m_Viewport,
				.scissorCount = 1,
				.pScissors = &m_Scissor,
			};
			VkPipelineRasterizationStateCreateInfo const rasterization_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.depthClampEnable = VK_FALSE,
				.rasterizerDiscardEnable = VK_FALSE,
				.polygonMode = VK_POLYGON_MODE_FILL,
				//.cullMode = VK_CULL_MODE_BACK_BIT,
				.cullMode = VK_CULL_MODE_NONE,
				.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
				.depthBiasEnable = VK_FALSE,
				.depthBiasConstantFactor{},
				.depthBiasClamp{},
				.depthBiasSlopeFactor{},
				.lineWidth = 1.0f,
			};
			VkPipelineMultisampleStateCreateInfo const multisample_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
				.sampleShadingEnable = VK_FALSE,
				.minSampleShading = 1.0f,
				.pSampleMask{},
				.alphaToCoverageEnable{},
				.alphaToOneEnable{}
			};
			VkPipelineDepthStencilStateCreateInfo const depth_stencil_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.depthTestEnable{},
				.depthWriteEnable{},
				.depthCompareOp{},
				.depthBoundsTestEnable{},
				.stencilTestEnable{},
				.front{},
				.back{},
				.minDepthBounds{},
				.maxDepthBounds{}
			};
			/* modify for alpha blending */
			VkPipelineColorBlendAttachmentState const colour_blend_attachment_state{
				.blendEnable = VK_FALSE,
				.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
				.colorBlendOp = VK_BLEND_OP_ADD,
				.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
				.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
				.alphaBlendOp = VK_BLEND_OP_ADD,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
			};
			VkPipelineColorBlendStateCreateInfo const colour_blend_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.logicOpEnable = VK_FALSE,
				.logicOp = VK_LOGIC_OP_COPY,
				.attachmentCount = 1,
				.pAttachments = &colour_blend_attachment_state,
				.blendConstants{}
			};
			std::array<VkDynamicState, 2> static constexpr dynamic_states{ { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } };
			VkPipelineDynamicStateCreateInfo const dynamic_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.dynamicStateCount = static_cast<u32>(dynamic_states.size()),
				.pDynamicStates = dynamic_states.data(),
			};
			VkPipelineLayoutCreateInfo const pipeline_layout_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.pNext{},
				.flags{},
				.setLayoutCount = 1,
				.pSetLayouts = &m_DescriptorSetLayout,
				.pushConstantRangeCount{},
				.pPushConstantRanges{}
			};
			result = vkCreatePipelineLayout(m_LogicalDevice, &pipeline_layout_create_info, nullptr, &m_GraphicsPipelineLayout);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreatePielineLayout failed creation: {}", getEnumString(result));
			}
#endif

			static constexpr std::size_t ATTACHMENT_COUNT = 2;
			std::array<VkAttachmentDescription, ATTACHMENT_COUNT> attachment_descriptions{ {
				/* Colour Attachment Description */
				{
					.flags{},
					.format = m_SurfaceFormat,
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				},
				/* Depth Attachment Description */
				{
					.flags{},
					.format = VK_FORMAT_D32_SFLOAT, /* review: apply more scrutiny */
					.samples = VK_SAMPLE_COUNT_1_BIT,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
					.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
					.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			} };
			std::array<VkAttachmentReference, ATTACHMENT_COUNT> attachment_references{ {
				/* Colour Attachment Reference */
				{
					.attachment = 0,
					.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
				},
				/* Depth Attachment Reference */
				{
					.attachment = 1,
					.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
				}
			} };

			VkSubpassDescription const subpass_description{
				.flags{},
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.inputAttachmentCount{},
				.pInputAttachments{},
				.colorAttachmentCount = 1,
				.pColorAttachments = &attachment_references[0],
				.pResolveAttachments{},
				.pDepthStencilAttachment = &attachment_references[1],
				.preserveAttachmentCount{},
				.pPreserveAttachments{}
			};

			VkSubpassDependency const subpass_dependency{
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				.srcAccessMask{},
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.dependencyFlags{}
			};
			VkRenderPassCreateInfo const render_pass_create_info{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
				.pNext{},
				.flags{},
				.attachmentCount = ATTACHMENT_COUNT,
				.pAttachments = attachment_descriptions.data(),
				.subpassCount = 1,
				.pSubpasses = &subpass_description,
				.dependencyCount = 1,
				.pDependencies = &subpass_dependency
			};
			result = vkCreateRenderPass(m_LogicalDevice, &render_pass_create_info, nullptr, &m_RenderPass);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateRenderPass failed creation: {}", getEnumString(result));
			}
#endif

			VkPipelineDepthStencilStateCreateInfo const graphics_pipeline_depth_stencil_state_create_info{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
				.pNext{},
				.flags{},
				.depthTestEnable = VK_TRUE,
				.depthWriteEnable = VK_TRUE,
				.depthCompareOp = VK_COMPARE_OP_LESS,
				.depthBoundsTestEnable = VK_FALSE,
				.stencilTestEnable = VK_FALSE,
				.front{},
				.back{},
				.minDepthBounds = 0.0f,
				.maxDepthBounds = 1.0f
			};
			VkGraphicsPipelineCreateInfo const graphics_pipeline_create_info{
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.pNext = nullptr,
				.flags{}, /* VK_PIPELINE_CREATE_DERIVATIVE_BIT for faster recreation of similar pipelines */
				.stageCount = sizeof(PipelineShaderStageCreateInfos) / sizeof(VkPipelineShaderStageCreateInfo),
				.pStages = &shader_stage_create_infos.vertex_stage_create_info,
				.pVertexInputState = &vertex_input_state_create_info,
				.pInputAssemblyState = &input_assembly_state_create_info,
				.pTessellationState{},
				.pViewportState = &viewport_state_create_info,
				.pRasterizationState = &rasterization_state_create_info,
				.pMultisampleState = &multisample_state_create_info,
				.pDepthStencilState = &graphics_pipeline_depth_stencil_state_create_info,
				.pColorBlendState = &colour_blend_state_create_info,
				.pDynamicState = &dynamic_state_create_info,
				.layout = m_GraphicsPipelineLayout,
				.renderPass = m_RenderPass,
				.subpass = 0,
				.basePipelineHandle = VK_NULL_HANDLE,
				.basePipelineIndex = -1
			};
			result = vkCreateGraphicsPipelines(m_LogicalDevice, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &m_GraphicsPipeline);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateGraphicsPipeline failed creation: {}", getEnumString(result));
			}
#endif

			/* move elsewhere. maybe */
			vkDestroyShaderModule(m_LogicalDevice, vertex_shader_module, nullptr);
			vkDestroyShaderModule(m_LogicalDevice, fragment_shader_module, nullptr);

			SPDLOG_DEBUG("Pipeline Creation Finished");
		}{}
		
		/* Command Pool */ {
			VkCommandPoolCreateInfo const command_pool_create_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext{},
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = m_QueueFamilyIndex
			};
			result = vkCreateCommandPool(m_LogicalDevice, &command_pool_create_info, nullptr, &m_CommandPool);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateCommandPool failed creation: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Command Pool Creation Finished");
		}

		/* Depth Buffer */ {
			VkFormat const depth_format = VK_FORMAT_D32_SFLOAT; /* review: apply more scrutiny */
			std::tie(result, m_DepthBufferImage, m_DepthBufferMemory) = createImage(m_LogicalDevice, m_PhysicalDevice, m_Extent.width, m_Extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("createImage failed image creation: {}", getEnumString(result));
			}
#endif
			std::tie(result, m_DepthBufferView) = createImageView(m_LogicalDevice, m_DepthBufferImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("createImageView failed image view creation: {}", getEnumString(result));
			}
#endif

			result = transitionImageLayout(m_LogicalDevice, m_CommandPool, m_GraphicsQueue, m_DepthBufferImage, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("transitionImageLayout failed image layout transision: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Depth Buffer Creation Finished");
		}
		
		/* Framebuffers */ {
			m_Framebuffers.resize(std::size(m_ImageViews));
			for (auto [frame_buffer, image_view] : std::views::zip(m_Framebuffers, m_ImageViews)) {
				std::array<VkImageView, 2> const image_view_attachments = { image_view, m_DepthBufferView };

				VkFramebufferCreateInfo const framebuffer_create_info{
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.pNext = nullptr,
					.flags{},
					.renderPass = m_RenderPass,
					.attachmentCount = static_cast<u32>(std::size(image_view_attachments)),
					.pAttachments = std::data(image_view_attachments),
					.width = m_Extent.width,
					.height = m_Extent.height,
					.layers = 1
				};
				result = vkCreateFramebuffer(m_LogicalDevice, &framebuffer_create_info, nullptr, &frame_buffer);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkCreateFramebuffer failed creation: {}", getEnumString(result));
				}
#endif
			}

			SPDLOG_DEBUG("Frame Buffer Creation Finished");
		}

		/* Textures */ {
			result = AMS.fetchLayeredTexture("Viking Room").create(m_LogicalDevice, m_PhysicalDevice, m_GraphicsQueue, m_CommandPool, m_PhysicalDeviceProperties.limits.maxSamplerAnisotropy);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("Texture Load Failed: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Texture Loading Finished");
		}

		/* UBO Creation */ {
			for (auto&& ubo : m_UBOs) {
				result = ubo.create(m_LogicalDevice, m_PhysicalDevice);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("UniformBufferObject::create failed ubo creation: {}", getEnumString(result));
				}
#endif

				auto& uniform = ubo.uniform();
				uniform.view = glm::lookAt(glm::vec3(0.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				uniform.projection = glm::perspective(glm::radians(45.0f), static_cast<f32>(m_Extent.width) / static_cast<f32>(m_Extent.height), 0.1f, 1000.0f);
				(void)std::memcpy(ubo.mapped_buffer().mapped(), &ubo.uniform(), ubo.mapped_buffer().size());
			}
		}

		/* Vertex, Per-Instance, and Index Buffer Creation */ {
			auto& mesh = AMS.fetchMesh("Viking Room");
			auto vertices = mesh.vertices();
			auto indices = mesh.indices();

			result = m_VertexBuffer.create(m_LogicalDevice, m_PhysicalDevice, sizeof(decltype(*std::begin(vertices))) * std::size(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("Vertex Buffer creation failed: {}", getEnumString(result));
			}
#endif
			result = m_PerInstanceBuffer.create(m_LogicalDevice, m_PhysicalDevice, sizeof(EntityInstanceData) * std::size(m_Entities), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("Per-Instance Buffer creation failed: {}", getEnumString(result));
			}
#endif
			result = m_IndexBuffer.create(m_LogicalDevice, m_PhysicalDevice, sizeof(decltype(*std::begin(indices))) * std::size(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("Index Buffer creation failed: {}", getEnumString(result));
			}
#endif
			
			(void)std::memcpy(m_VertexBuffer.mapped(), std::data(vertices), m_VertexBuffer.size());
			auto mapped_per_instance_buffer = reinterpret_cast<u08*>(m_PerInstanceBuffer.mapped());
			for (auto[i, entity] : std::views::enumerate(m_Entities)) {
				(void)std::memcpy(mapped_per_instance_buffer + sizeof(EntityInstanceData) * i, &entity.instance_data(), sizeof(EntityInstanceData));
			}
			(void)std::memcpy(m_IndexBuffer.mapped(), std::data(indices), m_IndexBuffer.size());
		}
		
		/* Descriptor Pools */ {
			std::array<VkDescriptorPoolSize, 2> const descriptor_pool_sizes { {
				/* UBO */
				{
					.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
					.descriptorCount = static_cast<u32>(FRAMES_IN_FLIGHT)
				},
				/* Texture Sampling */
				{
					.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = static_cast<u32>(FRAMES_IN_FLIGHT)
				}
			} };
			VkDescriptorPoolCreateInfo const descriptor_pool_create_info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
				.pNext = nullptr,
				.flags{},
				.maxSets = static_cast<u32>(FRAMES_IN_FLIGHT),
				.poolSizeCount = static_cast<u32>(std::size(descriptor_pool_sizes)),
				.pPoolSizes = std::data(descriptor_pool_sizes)
			};
			result = vkCreateDescriptorPool(m_LogicalDevice, &descriptor_pool_create_info, nullptr, &m_DescriptorPool);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateDescriptorPool failed descriptor pool creation: {}", getEnumString(result));
			}
#endif
		}

		/* Descriptor Sets */ {
			std::fill(std::begin(m_DescriptorSetLayouts), std::end(m_DescriptorSetLayouts), m_DescriptorSetLayout);
			VkDescriptorSetAllocateInfo descriptor_set_allocate_info{
				.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
				.pNext = nullptr,
				.descriptorPool = m_DescriptorPool,
				.descriptorSetCount = static_cast<u32>(std::size(m_DescriptorSets)),
				.pSetLayouts = std::data(m_DescriptorSetLayouts)
			};
			result = vkAllocateDescriptorSets(m_LogicalDevice, &descriptor_set_allocate_info, std::data(m_DescriptorSets));
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkAllocateDescriptorSets failed descriptor set allocation: {}", getEnumString(result));
			}
#endif
			auto& texture = AMS.fetchLayeredTexture("Viking Room");
			for (auto [ubo, descriptor_set] : std::views::zip(m_UBOs, m_DescriptorSets)) {
				VkDescriptorBufferInfo const ubo_info{ .buffer = ubo.mapped_buffer().buffer(), .offset = 0, .range = sizeof(ubo.uniform()) };
				VkDescriptorImageInfo const image_info{ .sampler = texture.sampler(), .imageView = texture.image_view(), .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
				std::array<VkWriteDescriptorSet, 2> const write_descriptor_sets{ {
					/* UBO */ {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = VK_NULL_HANDLE,
					.dstSet = descriptor_set,
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
					.pImageInfo = VK_NULL_HANDLE,
					.pBufferInfo = &ubo_info,
					.pTexelBufferView = VK_NULL_HANDLE },
					/* Texture Sampler */ {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.pNext = VK_NULL_HANDLE,
					.dstSet = descriptor_set,
					.dstBinding = 1,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.pImageInfo = &image_info,
					.pBufferInfo = VK_NULL_HANDLE,
					.pTexelBufferView = VK_NULL_HANDLE }
				} };
				vkUpdateDescriptorSets(m_LogicalDevice, static_cast<u32>(std::size(write_descriptor_sets)), std::data(write_descriptor_sets), 0, nullptr);
			}

			SPDLOG_DEBUG("Descriptor Sets Finished");
		}
		
		/* Command Buffers */ {
			VkCommandBufferAllocateInfo const command_buffer_allocate_info{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.pNext{},
				.commandPool = m_CommandPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = FRAMES_IN_FLIGHT
			};
			result = vkAllocateCommandBuffers(m_LogicalDevice, &command_buffer_allocate_info, std::data(m_CommandBuffers));
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkAllocateCommandBuffers failed creation: {}", getEnumString(result));
			}
#endif

			SPDLOG_DEBUG("Command Buffer Creation Finished");
		}

		/* Sync Objects */ {
			VkSemaphoreCreateInfo static constexpr semaphore_create_info{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = nullptr,
				.flags{}
			};
			VkFenceCreateInfo static constexpr fence_create_info{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.pNext = nullptr,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT
			};

			for (auto [image_available_semaphore, render_finished_semaphore, fence] : std::views::zip(m_ImageAvailableSemaphores, m_RenderFinishedSemaphores, m_Fences)) {
				result = vkCreateSemaphore(m_LogicalDevice, &semaphore_create_info, nullptr, &image_available_semaphore);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkCreateSemaphore failed creation: {}", getEnumString(result));
				}
#endif
				result = vkCreateSemaphore(m_LogicalDevice, &semaphore_create_info, nullptr, &render_finished_semaphore);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkCreateSemaphore failed creation: {}", getEnumString(result));
				}
#endif
				result = vkCreateFence(m_LogicalDevice, &fence_create_info, nullptr, &fence);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkCreateFence failed creation: {}", getEnumString(result));
				}
#endif
			}

			SPDLOG_DEBUG("Sync Object Creation Finished");
		}
	}
	Renderer::~Renderer() {
		/* Release Assets */ {
			for (auto&& image : AMS.images()) {
				image.free();
			}
			for (auto&& mesh : AMS.meshes()) {
				mesh.free();
			}
			for (auto&& layered_texture : AMS.layered_textures()) {
				layered_texture.free(m_LogicalDevice);
			}
		}

		/* Sync Object Cleanup */ {
			for (auto [image_available_semaphore, render_finished_semaphore, fence] : std::views::zip(m_ImageAvailableSemaphores, m_RenderFinishedSemaphores, m_Fences)) {
				vkDestroySemaphore(m_LogicalDevice, image_available_semaphore, nullptr);
				vkDestroySemaphore(m_LogicalDevice, render_finished_semaphore, nullptr);
				vkDestroyFence(m_LogicalDevice, fence, nullptr);
			}
		}

		vkDestroyCommandPool(m_LogicalDevice, m_CommandPool, nullptr);

		/* Frame Buffer and Image View Cleanup */ {
			for (auto [frame_buffer, image_view] : std::views::zip(m_Framebuffers, m_ImageViews)) {
				vkDestroyFramebuffer(m_LogicalDevice, frame_buffer, nullptr);
				vkDestroyImageView(m_LogicalDevice, image_view, nullptr);
			}
		}
		
		vkDestroyPipeline(m_LogicalDevice, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_LogicalDevice, m_GraphicsPipelineLayout, nullptr);
		vkDestroyRenderPass(m_LogicalDevice, m_RenderPass, nullptr);
		vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, nullptr);

		/* Depth Buffer Cleanup */ {
			vkDestroyImageView(m_LogicalDevice, m_DepthBufferView, nullptr);
			vkDestroyImage(m_LogicalDevice, m_DepthBufferImage, nullptr);
			vkFreeMemory(m_LogicalDevice, m_DepthBufferMemory, nullptr);
		}
		
		for (auto&& ubo : m_UBOs) {
			ubo.free(m_LogicalDevice);
		}

		/* Vertex and Index Buffer Cleanup */ {
			m_VertexBuffer.free(m_LogicalDevice);
			m_PerInstanceBuffer.free(m_LogicalDevice);
			m_IndexBuffer.free(m_LogicalDevice);
		}

		vkDestroyDescriptorPool(m_LogicalDevice, m_DescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_LogicalDevice, m_DescriptorSetLayout, nullptr);
		vkDestroyDevice(m_LogicalDevice, nullptr);
#ifdef _DEBUG
		auto _vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (_vkDestroyDebugUtilsMessengerEXT == nullptr) {
			SPDLOG_ERROR("vkGetInstanceProcAddr failed to fetch \"vkDestroyDebugUtilsMessengerEXT\"");
		}
#pragma warning(disable: 6011)
		_vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#pragma warning()
#endif
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}

	void Renderer::RecreateSwapchain() {
		VkResult result;

		result = vkDeviceWaitIdle(m_LogicalDevice);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkDeviceWaiIdle failed... waiting: {}", getEnumString(result));
		}
#endif

		/* Cleanup */ {
			for (auto [frame_buffer, image_view] : std::views::zip(m_Framebuffers, m_ImageViews)) {
				vkDestroyFramebuffer(m_LogicalDevice, frame_buffer, nullptr);
				vkDestroyImageView(m_LogicalDevice, image_view, nullptr);
			}
			vkDestroySwapchainKHR(m_LogicalDevice, m_Swapchain, nullptr);
		}

		/* Swapchain Recreation */ {
			VkSurfaceFormatKHR surface_format;
			VkSurfaceTransformFlagBitsKHR pre_transform;
			u32 min_image_count;
			/* Surface Format Selection */ {
				u32 surface_format_count = 0;
				result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &surface_format_count, nullptr);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR failed count fetch: {}", getEnumString(result));
				}
#endif
				std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
				result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &surface_format_count, surface_formats.data());
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR failed fetch: {}", getEnumString(result));
				}
#endif
				surface_format = surface_formats[3];
				VkSurfaceCapabilitiesKHR surface_capabilities;
				result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &surface_capabilities);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed fetch: {}", getEnumString(result));
				}
#endif
				pre_transform = surface_capabilities.currentTransform;
				min_image_count = surface_capabilities.minImageCount + 1;
				m_Extent = surface_capabilities.currentExtent.width == std::numeric_limits<u32>::max() || surface_capabilities.currentExtent.height == std::numeric_limits<u32>::max() ? VkExtent2D{ std::clamp(800U, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width), std::clamp(600U, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height) } : surface_capabilities.currentExtent;
			}

			VkSwapchainCreateInfoKHR const swapchain_create_info{
				.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
				.pNext = nullptr,
				.flags{},
				.surface = m_Surface,
				.minImageCount = min_image_count,
				.imageFormat = surface_format.format,
				.imageColorSpace = surface_format.colorSpace,
				.imageExtent = m_Extent,
				.imageArrayLayers = 1,
				.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
				.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
				.queueFamilyIndexCount = 0,
				.pQueueFamilyIndices = nullptr,
				.preTransform = pre_transform,
				.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				.presentMode = VK_PRESENT_MODE_FIFO_KHR,
				.clipped = VK_TRUE,
				.oldSwapchain = VK_NULL_HANDLE,	/* used for swapchain recreation. research before use */
			};
			result = vkCreateSwapchainKHR(m_LogicalDevice, &swapchain_create_info, nullptr, &m_Swapchain);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkCreateSwapchainKHR failed creation: {}", getEnumString(result));
			}
#endif
		}

		/* Depth Buffer Recreation */ {
			vkDestroyImageView(m_LogicalDevice, m_DepthBufferView, nullptr);
			vkDestroyImage(m_LogicalDevice, m_DepthBufferImage, nullptr);
			vkFreeMemory(m_LogicalDevice, m_DepthBufferMemory, nullptr);
			VkFormat const depth_format = VK_FORMAT_D32_SFLOAT; /* review: apply more scrutiny */
			std::tie(result, m_DepthBufferImage, m_DepthBufferMemory) = createImage(m_LogicalDevice, m_PhysicalDevice, m_Extent.width, m_Extent.height, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("createImage failed image creation: {}", getEnumString(result));
			}
#endif
			std::tie(result, m_DepthBufferView) = createImageView(m_LogicalDevice, m_DepthBufferImage, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("createImageView failed image view creation: {}", getEnumString(result));
			}
#endif
			result = transitionImageLayout(m_LogicalDevice, m_CommandPool, m_GraphicsQueue, m_DepthBufferImage, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("transitionImageLayout failed image layout transision: {}", getEnumString(result));
			}
#endif
		}

		/* Image View and Frame Buffer Recreation */ {
			u32 image_count = 0;
			result = vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &image_count, nullptr);
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetSwapchainImagesKHR failed count fetch: {}", getEnumString(result), nullptr);
			}
#endif
			std::vector<VkImage> images(image_count);
			result = vkGetSwapchainImagesKHR(m_LogicalDevice, m_Swapchain, &image_count, images.data());
#ifdef _DEBUG
			if (result != VK_SUCCESS) {
				SPDLOG_ERROR("vkGetSwapchainImagesKHR failed fetch: {}", getEnumString(result));
			}
#endif
			for (auto [frame_buffer, image_view, image] : std::views::zip(m_Framebuffers, m_ImageViews, images)) {
				std::tie(result, image_view) = createImageView(m_LogicalDevice, image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("createImageView failed image view creation: {}", getEnumString(result));
				}
#endif

				std::array<VkImageView, 2> const image_view_attachments = { image_view, m_DepthBufferView };
				VkFramebufferCreateInfo const framebuffer_create_info{
					.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
					.pNext = nullptr,
					.flags{},
					.renderPass = m_RenderPass,
					.attachmentCount = static_cast<u32>(std::size(image_view_attachments)),
					.pAttachments = std::data(image_view_attachments),
					.width = m_Extent.width,
					.height = m_Extent.height,
					.layers = 1
				};
				result = vkCreateFramebuffer(m_LogicalDevice, &framebuffer_create_info, nullptr, &frame_buffer);
#ifdef _DEBUG
				if (result != VK_SUCCESS) {
					SPDLOG_ERROR("vkCreateFramebuffer failed creation: {}", getEnumString(result));
				}
#endif
			}
		}
	}
	void Renderer::RecordCommandBuffer(VkCommandBuffer command_buffer, u32 const image_index) {
		VkResult result;

		VkCommandBufferBeginInfo static constexpr command_buffer_begin_info{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext{},
			.flags{}, /* review */
			.pInheritanceInfo{}
		};

		result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkBeginCommandBuffer failed execution: {}", getEnumString(result));
		}
#endif

		vkCmdSetViewport(command_buffer, 0, 1, &m_Viewport);
		vkCmdSetScissor(command_buffer, 0, 1, &m_Scissor);

		std::array<VkClearValue, 2> const clear_values{ {
				/* Colour Clear Value */
				{ 0.f, 0.f, 0.f, 1.f },
				/* Depth Clear Value */
				{ 1.f, 0 }
			} };
		VkRenderPassBeginInfo const render_pass_begin_info{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext{},
			.renderPass = m_RenderPass,
			.framebuffer = m_Framebuffers[image_index],
			.renderArea{
				.offset{},
				.extent = m_Extent
			},
			.clearValueCount = static_cast<u32>(std::size(clear_values)),
			.pClearValues = std::data(clear_values)
		};

		vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
		//std::array<VkBuffer, 1> const vertex_buffers{ { m_VertexBuffer } };

		std::array<VkDeviceSize, 1> const vertex_offsets{ { 0 } };

		std::array<VkDeviceSize, 1> offsets{ { 0 } };
		u32 dynamic_offset = 0;
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipelineLayout, 0, 1, &m_DescriptorSets[m_CurrentFrameIndex], 1, &dynamic_offset);
		VkBuffer buffer;
		buffer = m_VertexBuffer.buffer();
		vkCmdBindVertexBuffers(command_buffer, 0, 1, &buffer, std::data(offsets));
		buffer = m_PerInstanceBuffer.buffer();
		vkCmdBindVertexBuffers(command_buffer, 1, 1, &buffer, std::data(offsets));
		vkCmdBindIndexBuffer(command_buffer, m_IndexBuffer.buffer(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(command_buffer, m_IndexBuffer.size() / 4, 2, 0, 0, 0);
		//vertex_buffer.free(m_LogicalDevice);
		//per_instance_buffer.free(m_LogicalDevice);
		//index_buffer.free(m_LogicalDevice);

		vkCmdEndRenderPass(command_buffer);
		result = vkEndCommandBuffer(command_buffer);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkEndCommandBuffer failed execution: {}", getEnumString(result));
		}
#endif
	}
	VkBuffer Renderer::CopyBuffer(VkBuffer buffer, VkDeviceSize const size) const noexcept {
		auto[result, command_buffer] = recordOnceOffCommand(m_LogicalDevice, m_CommandPool);
		VkBufferCopy const buffer_copy{
			.srcOffset{},
			.dstOffset{},
			.size = size
		};
		VkBuffer dst = VK_NULL_HANDLE;
		vkCmdCopyBuffer(command_buffer, buffer, dst, 1, &buffer_copy);
		(void)destroyOnceOffCommand(m_LogicalDevice, m_CommandPool, command_buffer, m_GraphicsQueue);
		return dst;
	}
}