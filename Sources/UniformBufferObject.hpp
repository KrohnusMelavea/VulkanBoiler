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
		VkResult create(VkDevice const logical_device, VkPhysicalDevice const physical_device);
		void free(VkDevice const logical_device);

		[[nodiscard]] UniformData& uniform() noexcept;
		[[nodiscard]] MappedBuffer& mapped_buffer() noexcept;

	private:
		UniformData m_Uniform;
		MappedBuffer m_MappedBuffer;
	};
}