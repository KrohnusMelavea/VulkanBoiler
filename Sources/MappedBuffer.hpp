#pragma once

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#include <string>
#pragma warning(pop)

namespace API_NAME {
	/* MUST be freed manually */
	class MappedBuffer {
	public:
		MappedBuffer() = default; /* Illegal to perform ANY operations on default-constructed MappedBuffer */
		MappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device) noexcept; /* In a non-created state after construction */
		MappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties);
		MappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties); /* Assumes ownership of the buffer. */

		operator std::string() const noexcept;

		/* Buffer MUST be in freed state prior to calling any create() function. */
		VkResult create();
		VkResult create(VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties);
		VkResult create(VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties); /* Assumes ownership of the buffer. */
		void free();
		void relinquish() noexcept;

		VkBuffer buffer() const noexcept;
		VkDeviceMemory memory() const noexcept;
		void* mapped_memory() const noexcept;
		VkDeviceSize size() const noexcept;
		VkBufferUsageFlags usage() const noexcept;
		VkMemoryPropertyFlags memory_properties() const noexcept;

	private:
		VkDevice m_LogicalDevice;
		VkPhysicalDevice m_PhysicalDevice;

		VkBuffer m_Buffer;
		VkDeviceMemory m_Memory;
		void* m_MappedMemory;
		VkDeviceSize m_Size;
		VkBufferUsageFlags m_Usage;
		VkMemoryPropertyFlags m_MemoryProperties;
	};
}