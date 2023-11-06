#include "Common/Util.hpp"
#include "Common/EnumStrings.hpp"
#include "UnmappedBuffer.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <tuple>
#pragma warning(pop)

namespace API_NAME {
	UnmappedBuffer::UnmappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device) noexcept : m_LogicalDevice{ logical_device }, m_PhysicalDevice{ physical_device }, m_Buffer{ VK_NULL_HANDLE }, m_Memory{ VK_NULL_HANDLE }, m_Size{ NULL }, m_Usage{ VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM }, m_MemoryProperties{ VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM } {

	}
	UnmappedBuffer::UnmappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) : m_LogicalDevice{ logical_device }, m_PhysicalDevice{ physical_device }, m_Size{ size }, m_Usage { usage }, m_MemoryProperties{ memory_properties } {
		VkResult result = VK_SUCCESS;

		result = create();
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("UnmappedBuffer failed creation: {}", getEnumString(result));
		}
#endif
	}
	UnmappedBuffer::UnmappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) : m_LogicalDevice{ logical_device }, m_PhysicalDevice{ physical_device }, m_Buffer{ buffer }, m_Memory{ memory }, m_Size{ size }, m_Usage{ usage }, m_MemoryProperties{ memory_properties } {

	}

	VkResult UnmappedBuffer::create() {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) {
			SPDLOG_WARN("UnmappedBuffer attempted buffer creation without destructing previously created buffer.");
			free();
		}
#endif

		VkResult result = VK_SUCCESS;

		std::tie(result, m_Buffer, m_Memory) = createBuffer(m_LogicalDevice, m_PhysicalDevice, m_Size, m_Usage, m_MemoryProperties);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("createBuffer failed buffer creation: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}
	VkResult UnmappedBuffer::create(VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) {
			SPDLOG_WARN("UnmappedBuffer attempted buffer creation without destructing previously created buffer.");
			free();
		}
#endif

		m_Size = size;
		m_Usage = usage;
		m_MemoryProperties = memory_properties;

		return create();
	}
	void UnmappedBuffer::create(VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) noexcept {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) {
			SPDLOG_WARN("UnmappedBuffer attempted buffer creation without destructing previously created buffer.");
			free();
		}
#endif

		m_Buffer = buffer;
		m_Memory = memory;
		m_Size = size;
		m_Usage = usage;
		m_MemoryProperties = memory_properties;
	}

	void UnmappedBuffer::free() {
		vkDestroyBuffer(m_LogicalDevice, m_Buffer, nullptr);
		vkFreeMemory(m_LogicalDevice, m_Memory, nullptr);
	}

	VkBuffer UnmappedBuffer::buffer() const noexcept {
		return m_Buffer;
	}
	VkDeviceMemory UnmappedBuffer::memory() const noexcept {
		return m_Memory;
	}
	VkDeviceSize UnmappedBuffer::size() const noexcept {
		return m_Size;
	}
	VkBufferUsageFlags UnmappedBuffer::usage() const noexcept {
		return m_Usage;
	}
	VkMemoryPropertyFlags UnmappedBuffer::memory_properties() const noexcept {
		return m_MemoryProperties;
	}
}