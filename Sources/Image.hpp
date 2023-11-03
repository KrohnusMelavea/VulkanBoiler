#pragma once

#include "Common/Types.hpp"
#include "Pixel.hpp"
#pragma warning(push, 0)
#include <span>
#include <memory>
#include <string>
#include <filesystem>
#include <string_view>
#pragma warning(pop)

namespace API_NAME {
	class Image {
	public:
		using id_type = std::string;
		std::string_view static constexpr NULL_ID = "";

		Image();
		Image(std::filesystem::path const& file_path, id_type const& name, bool load_image = true);
		
		Image(Image const& image);
		
		void operator=(Image const& image);

		void load();
		void ensureLoaded();
		void free();

		[[nodiscard]] std::span<u08> operator[](std::size_t const i) noexcept;

		[[nodiscard]] bool isLoaded() const noexcept;
		[[nodiscard]] id_type const& id() const noexcept;
		[[nodiscard]] std::filesystem::path const& filepath() const noexcept;
		[[nodiscard]] u32 width() const noexcept;
		[[nodiscard]] u32 height() const noexcept;
		[[nodiscard]] std::size_t size() const noexcept;
		[[nodiscard]] std::span<u08> pixels() noexcept;

	private:
		bool m_IsLoaded;
		id_type m_ID;
		std::filesystem::path m_FilePath;
		u32 m_Width;
		u32 m_Height;
		std::unique_ptr<u08[], decltype(std::free)*> m_Pixels;
		std::span<u08> m_PixelView;
	};
}