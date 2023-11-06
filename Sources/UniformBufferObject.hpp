#pragma once

#include "UniformData.hpp"
#include "MappedBuffer.hpp"

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#pragma warning(pop)

namespace API_NAME {
	class UniformBufferObject {
	public:
		UniformBufferObject() = default;
		UniformBufferObject(VkDevice const logical_device, VkPhysicalDevice const physical_device) noexcept;

		VkResult create();
		void free();

		[[nodiscard]] UniformData& uniform() noexcept;
		[[nodiscard]] MappedBuffer& mapped_buffer() noexcept;

	private:
		VkDevice m_LogicalDevice;
		VkPhysicalDevice m_PhysicalDevice;

		UniformData m_Uniform;
		MappedBuffer m_MappedBuffer;
	};
}