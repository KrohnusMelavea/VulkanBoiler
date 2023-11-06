#include "Common/Util.hpp"
#include "Common/EnumStrings.hpp"
#include "MappedBuffer.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <tuple>
#pragma warning(pop)

namespace API_NAME {
	MappedBuffer::MappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device) noexcept : m_LogicalDevice{ logical_device }, m_PhysicalDevice{ physical_device }, m_Buffer{ VK_NULL_HANDLE }, m_Memory{ VK_NULL_HANDLE }, m_MappedMemory{ nullptr }, m_Size{ NULL }, m_Usage{ VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM }, m_MemoryProperties{ VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM } {

	}
	MappedBuffer::MappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) : m_LogicalDevice{ logical_device }, m_PhysicalDevice{ physical_device }, m_Size{ size }, m_Usage{ usage }, m_MemoryProperties{ memory_properties } {
		VkResult result = VK_SUCCESS;

		result = create();
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("MappedBuffer failed creation: {}", getEnumString(result));
		}
#endif
	}
	MappedBuffer::MappedBuffer(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) : m_LogicalDevice{ logical_device }, m_PhysicalDevice{ physical_device }, m_Buffer{ buffer }, m_Memory{ memory }, m_Size { size }, m_Usage{ usage }, m_MemoryProperties{ memory_properties } {

	}

	MappedBuffer::operator std::string() const noexcept {
		return fmt::format("MappedBuffer:\n- m_LogicalDevice: {}\n- m_PhysicalDevice: {}\n- m_Buffer: {}\n- m_Memory: {}\n- m_MappedBuffer: {}\n- m_Size: {}\n- m_Usage: {}\n- m_MemoryProperties: {}", (u64)m_LogicalDevice, (u64)m_PhysicalDevice, (u64)m_Buffer, (u64)m_Memory, (u64)m_MappedMemory, m_Size, getEnumString(m_Usage, FlagType::BufferUsage), getEnumString(m_MemoryProperties, FlagType::MemoryProperty));
	}

	VkResult MappedBuffer::create() {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) {
			SPDLOG_WARN("MappedBuffer attempted buffer creation without destructing previously created buffer.");
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
		result = vkMapMemory(m_LogicalDevice, m_Memory, 0, m_Size, 0, &m_MappedMemory);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkMapMemory failed memory mapping: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}
	VkResult MappedBuffer::create(VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) {
			SPDLOG_WARN("MappedBuffer attempted buffer creation without destructing previously created buffer.");
			free();
		}
#endif

		m_Size = size;
		m_Usage = usage;
		m_MemoryProperties = memory_properties;

		return create();
	}
	VkResult MappedBuffer::create(VkBuffer const buffer, VkDeviceMemory const memory, VkDeviceSize const size, VkBufferUsageFlags const usage, VkMemoryPropertyFlags const memory_properties) {
#ifdef _DEBUG
		if (m_Buffer != VK_NULL_HANDLE) {
			SPDLOG_WARN("MappedBuffer attempted buffer creation without destructing previously created buffer.");
			free();
		}
#endif

		VkResult result = VK_SUCCESS;

		m_Buffer = buffer;
		m_Memory = memory;
		m_Size = size;
		m_Usage = usage;
		m_MemoryProperties = memory_properties;

		result = vkMapMemory(m_LogicalDevice, m_Memory, 0, m_Size, 0, &m_MappedMemory);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkMapMemory failed memory mapping: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}

	void MappedBuffer::free() {
		vkUnmapMemory(m_LogicalDevice, m_Memory);
		vkDestroyBuffer(m_LogicalDevice, m_Buffer, nullptr);
		vkFreeMemory(m_LogicalDevice, m_Memory, nullptr);
	}
	void MappedBuffer::relinquish() noexcept {
		vkUnmapMemory(m_LogicalDevice, m_Memory);
		m_Buffer = VK_NULL_HANDLE;
		m_Memory = VK_NULL_HANDLE;
		m_MappedMemory = nullptr;
		m_Size = NULL;
		m_Usage = VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
		m_MemoryProperties = VK_MEMORY_PROPERTY_FLAG_BITS_MAX_ENUM;
	}

	VkBuffer MappedBuffer::buffer() const noexcept {
		return m_Buffer;
	}
	VkDeviceMemory MappedBuffer::memory() const noexcept {
		return m_Memory;
	}
	void* MappedBuffer::mapped_memory() const noexcept {
		return m_MappedMemory;
	}
	VkDeviceSize MappedBuffer::size() const noexcept {
		return m_Size;
	}
	VkBufferUsageFlags MappedBuffer::usage() const noexcept {
		return m_Usage;
	}
	VkMemoryPropertyFlags MappedBuffer::memory_properties() const noexcept {
		return m_MemoryProperties;
	}

}