#include "Common/Util.hpp"
#include "Common/Types.hpp"
#include "Common/EnumStrings.hpp"
#include "MappedBuffer.hpp"
#include "LayeredTexture.hpp"
#include "AssetManagementSystem.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <limits>
#include <ranges>
#include <algorithm>
#pragma warning(pop)

namespace API_NAME {
	LayeredTexture::LayeredTexture() : m_Loaded{ false }, m_ID{ LayeredTexture::NULL_ID }, m_ImageCount{ 0 }, m_ImageIDs{ Image::id_type(Image::NULL_ID) } {

	}
	LayeredTexture::LayeredTexture(LayeredTexture::id_type const& id) : m_Loaded{ false }, m_ID{ id }, m_ImageCount { 0 }, m_ImageIDs{ Image::id_type(Image::NULL_ID) } {

	}
#ifdef _DEBUG
	LayeredTexture::~LayeredTexture() {
		if (m_Loaded) {
			SPDLOG_ERROR("Layered Texture destructed while still loaded, resulting in memory leak.");
		}
	}
#endif

	void LayeredTexture::addImage(Image::id_type const& id, u32 const layer) {
#ifdef _DEBUG
		if (layer >= MAX_LAYERS) {
			SPDLOG_ERROR("Layered Texture at capacity.");
			return;
		}
#endif
		m_ImageIDs[layer] = id;
	}
	VkResult LayeredTexture::create(VkDevice const logical_device, VkPhysicalDevice const physical_device, VkQueue const graphics_queue, VkCommandPool const command_pool, f32 const anistropy) {
		VkResult result = VK_SUCCESS;

		auto& base_image = AMS.fetchImage(m_ImageIDs[0]);

#ifdef _DEBUG
		base_image.ensureLoaded();
		bool images_valid = std::none_of(std::next(std::cbegin(m_ImageIDs)), std::cend(m_ImageIDs),	
			[&base_image](auto&& id) {
				if (id == Image::NULL_ID) return false;
				auto& image = AMS.fetchImage(id);
				image.ensureLoaded();
				return base_image.width() != image.width() || base_image.height() != image.height() || base_image.size() != image.size();
			});
		if (!images_valid) {
			SPDLOG_ERROR("Layered Texture images invalid");
		}
#endif
		
		auto valid_images = std::views::filter(m_ImageIDs, [](auto&& id) { return id != Image::NULL_ID; });
		std::size_t const valid_image_count = std::ranges::count_if(valid_images, ConstantPredicate<true>);
		
		MappedBuffer image_buffer;
		result = image_buffer.create(logical_device, physical_device, valid_image_count * base_image.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("MappedBuffer::create failed buffer creation: {}", getEnumString(result));
			return result;
		}
#endif
		auto mapped = reinterpret_cast<u08*>(image_buffer.mapped());
		for (auto [i, id] : std::views::enumerate(valid_images)) {
			auto& image = AMS.fetchImage(id);
			(void)std::memcpy(mapped + i * base_image.size(), std::data(image.pixels()), base_image.size());
		}

		std::tie(result, m_Image, m_Memory) = createImage(logical_device, physical_device, base_image.width(), base_image.height(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, valid_image_count);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("createImage failed image creation: {}", getEnumString(result));
			return result;
		}
#endif
		result = transitionImageLayout(logical_device, command_pool, graphics_queue, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, valid_image_count);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("transitionImageLayout failed image layout transition: {}", getEnumString(result));
			return result;
		}
#endif
		result = copyBufferToImage(logical_device, command_pool, graphics_queue, image_buffer.buffer(), m_Image, base_image.width(), base_image.height(), valid_image_count);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("copyBufferToImage failed to copy buffer to image: {}", getEnumString(result));
			return result;
		}
#endif
		result = transitionImageLayout(logical_device, command_pool, graphics_queue, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, valid_image_count);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("transitionImageLayout failed image layout transition: {}", getEnumString(result));
			return result;
		}
#endif
		image_buffer.free(logical_device);

		std::tie(result, m_ImageView) = createImageView(logical_device, m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D_ARRAY, valid_image_count);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("createImageView failed image view creation: {}", getEnumString(result));
			return result;
		}
#endif

		VkSamplerCreateInfo static constexpr sampler_create_info{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.pNext = nullptr,
			.flags{},
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.mipLodBias = 0.0f,
			.anisotropyEnable = VK_TRUE,
			.maxAnisotropy = 1.0f,
			.compareEnable = VK_FALSE,
			.compareOp = VK_COMPARE_OP_NEVER,
			.minLod = 0.0f,
			.maxLod = 0.0f,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates = VK_FALSE
		};
		result = vkCreateSampler(logical_device, &sampler_create_info, nullptr, &m_ImageSampler);
#ifdef _DEBUG
		if (result != VK_SUCCESS) {
			SPDLOG_ERROR("vkCreateSampler failed sampler creation: {}", getEnumString(result));
			return result;
		}
#endif

		return result;
	}

	void LayeredTexture::free(VkDevice const logical_device) {
		vkDestroySampler(logical_device, m_ImageSampler, nullptr);
		vkDestroyImageView(logical_device, m_ImageView, nullptr);
		vkDestroyImage(logical_device, m_Image, nullptr);
		vkFreeMemory(logical_device, m_Memory, nullptr);
	}

	bool LayeredTexture::isLoaded() const noexcept {
		return m_Loaded;
	}
	LayeredTexture::id_type const& LayeredTexture::id() const noexcept {
		return m_ID;
	}
	std::span<Image::id_type const> LayeredTexture::imageIDs() const noexcept {
		return { std::data(m_ImageIDs), m_ImageCount };
	}
	VkImageView LayeredTexture::image_view() const noexcept {
		return m_ImageView;
	}
	VkSampler LayeredTexture::sampler() const noexcept {
		return m_ImageSampler;
	}
}