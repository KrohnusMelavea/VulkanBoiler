#pragma once

#include "Image.hpp"

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#include <span>
#include <array>
#include <string_view>
#pragma warning(pop)

namespace API_NAME {
	class LayeredTexture {
	public:
		using id_type = std::string;
		std::string_view static constexpr NULL_ID = "";

		u32 static constexpr MAX_LAYERS = 2048;

		LayeredTexture();
		LayeredTexture(id_type const& id);
#ifdef _DEBUG
		~LayeredTexture();
#endif

		void addImage(Image::id_type const& id, u32 const layer);	/* Assumes adequate capacity */
		VkResult create(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkQueue const graphics_queue, VkCommandPool const command_pool, f32 const anistropy); /* Assumes all images are of same dimensions, channels, and that they are loaded. */
		void free(VkDevice const logical_device);

		bool isLoaded() const noexcept;
		id_type const& id() const noexcept;
		std::span<Image::id_type const> imageIDs() const noexcept;
		VkImageView image_view() const noexcept;
		VkSampler sampler() const noexcept;

	private:
		bool m_Loaded;
		id_type m_ID;
		u32 m_ImageCount;
		std::array<Image::id_type, MAX_LAYERS> m_ImageIDs;
		VkImage m_Image;
		VkDeviceMemory m_Memory;
		VkImageView m_ImageView;
		VkSampler m_ImageSampler;
	};
}