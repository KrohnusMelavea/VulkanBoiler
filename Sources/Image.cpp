#include "Image.hpp"

#pragma warning(push, 0)
#include <stb_image/stb_image.h>
#include <spdlog/spdlog.h>
#pragma warning(pop)

namespace API_NAME {
	Image::Image() : m_IsLoaded{ false }, m_ID{}, m_FilePath{}, m_Width{ NULL }, m_Height{ NULL }, m_Pixels{ nullptr, std::free } {

	}
	Image::Image(std::filesystem::path const& file_path, Image::id_type const& id, bool load_image) : m_IsLoaded{ false }, m_ID { id }, m_FilePath{ file_path }, m_Width{ NULL }, m_Height{ NULL }, m_Pixels{ nullptr, std::free } {
		if (load_image) {
			load();
		}
	}
	Image::Image(Image const& image) : m_IsLoaded{ image.m_IsLoaded }, m_ID{ image.m_ID }, m_FilePath { image.m_FilePath }, m_Width{ image.m_Width }, m_Height{ image.m_Height }, m_Pixels{ reinterpret_cast<u08*>(std::malloc(std::size(image.m_PixelView))), std::free } {
		m_PixelView = { m_Pixels.get(), std::size(image.m_PixelView) };
		(void)std::copy(std::cbegin(image.m_PixelView), std::cend(image.m_PixelView), std::begin(m_PixelView));
	}

	void Image::operator=(Image const& image) {
		m_IsLoaded = image.m_IsLoaded;
		m_ID = image.m_ID;
		m_FilePath = image.m_FilePath;
		m_Width = image.m_Width;
		m_Height = image.m_Height;
		if (std::size(m_PixelView) != std::size(image.m_PixelView)) {
			m_Pixels = { reinterpret_cast<u08*>(std::malloc(std::size(image.m_PixelView))), std::free };
		}
		m_PixelView = { m_Pixels.get(), std::size(image.m_PixelView) };
		(void)std::copy(std::cbegin(image.m_PixelView), std::cend(image.m_PixelView), std::begin(m_PixelView));
	}

	void Image::load() {
		i32 width{}, height{}, channels{};
		auto const file_path = m_FilePath.string();
		m_Pixels = { stbi_load(file_path.c_str(), &width, &height, &channels, 4), std::free};
#ifdef _DEBUG
		if (m_Pixels == nullptr) {
			auto file_path = m_FilePath.string();
			SPDLOG_ERROR("stb_image failed to read image: {}", file_path.c_str());
		}
#endif
		m_PixelView = { m_Pixels.get(), static_cast<std::size_t>(width * height * 4) };
		m_Width = static_cast<u32>(width);
		m_Height = static_cast<u32>(height);
		m_IsLoaded = true;
	}
	void Image::ensureLoaded() {
		if (!m_IsLoaded) {
			load();
		}
	}
	void Image::free() {
		m_IsLoaded = false;
		m_Width = NULL;
		m_Height = NULL;
		m_Pixels.reset();
		m_PixelView = { m_Pixels.get(), NULL };
	}

	std::span<u08> Image::operator[](std::size_t const i) noexcept {
		return { &m_Pixels[i * m_Width * 4], m_Width * 4 };
	}

	bool Image::isLoaded() const noexcept {
		return m_IsLoaded;
	}
	Image::id_type const& Image::id() const noexcept {
		return m_ID;
	}
	std::filesystem::path const& Image::filepath() const noexcept {
		return m_FilePath;
	}
	u32 Image::width() const noexcept {
		return m_Width;
	}
	u32 Image::height() const noexcept {
		return m_Height;
	}
	std::size_t Image::size() const noexcept {
		return std::size(m_PixelView);
	}

	std::span<u08> Image::pixels() noexcept {
		return m_PixelView;
	}
}