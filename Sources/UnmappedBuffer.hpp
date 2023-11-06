#pragma once

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#pragma warning(pop)

namespace API_NAME {
	class UnmappedBuffer {
	public:
		UnmappedBuffer() = default;
		UnmappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device) noexcept;
		UnmappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties);
		UnmappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties);

		VkResult create();
		VkResult create(VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties);
		void create(VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) noexcept;

		void free();

		VkBuffer buffer() const noexcept;
		VkDeviceMemory memory() const noexcept;
		VkDeviceSize size() const noexcept;
		VkBufferUsageFlags usage() const noexcept;
		VkMemoryPropertyFlags memory_properties() const noexcept;

	private:
		VkDevice m_LogicalDevice;
		VkPhysicalDevice m_PhysicalDevice;

		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
		VkDeviceSize m_Size;
		VkBufferUsageFlags m_Usage;
		VkMemoryPropertyFlags m_MemoryProperties;
	};
}