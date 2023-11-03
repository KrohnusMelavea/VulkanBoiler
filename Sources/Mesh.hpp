#pragma once

#include "Common/Types.hpp"
#include "Vertex3D.hpp"

#pragma warning(push, 0)
#include <vulkan/vulkan.hpp>
#include <span>
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <string_view>
#pragma warning(pop)

namespace API_NAME {
	class Mesh {
	public:
		enum class SupportedFileFormat { Obj, glTF, Null };
		std::array<std::string_view, 2> static constexpr SUPPORTED_FILE_FORMATS{ ".obj", ".gltf" };

		using id_type = std::string;
		std::string_view static constexpr NULL_ID = "";

		Mesh() = default;
		Mesh(id_type const& id, bool load_on_construct = false);

		Mesh(Mesh const& mesh);

		void operator=(Mesh const& mesh);

		void load(std::filesystem::path const& file_path);
		void ensureLoaded();
		void free();

		[[nodiscard]] bool isLoaded() const noexcept;
		[[nodiscard]] id_type const& id() const noexcept;
		[[nodiscard]] std::span<Vertex3D> vertices() noexcept;
		[[nodiscard]] std::span<u32> indices() noexcept;

	private:
		bool m_IsLoaded;
		id_type m_ID;
		std::unique_ptr<Vertex3D[]> m_Vertices;
		std::span<Vertex3D> m_VertexView;
		std::unique_ptr<u32[]> m_Indices;
		std::span<u32> m_IndexView;
	};
}