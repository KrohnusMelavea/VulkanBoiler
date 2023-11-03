#pragma once

#include "Vertex3D.hpp"
#include "MappedBuffer.hpp"

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#include <span>
#pragma warning(pop)

namespace API_NAME {
	class VertexBufferObject {
	public:
		VkResult create(VkDevice const logical_device, VkPhysicalDevice const physical_device, std::span<Vertex3D> const& data);
		void free(VkDevice const logical_device);

		MappedBuffer& mapped_buffer() noexcept;

	private:
		MappedBuffer m_VertexBuffer;
	};
}