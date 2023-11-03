#pragma once

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#pragma warning(pop)

namespace API_NAME {
	class UnmappedBuffer {
	public:
		VkResult create(VkDevice const logical_device, VkPhysicalDevice const physical_device, std::size_t const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_flags);
		void free(VkDevice const logical_device);

		VkBuffer buffer() const noexcept;
		VkDeviceMemory memory() const noexcept;

	private:
		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
	};
}