#include "AssetManagementSystem.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <algorithm>
#pragma warning(pop)

namespace API_NAME {
	AssetManagementSystem::AssetManagementSystem() {

	}

	Mesh& AssetManagementSystem::insertMesh(Mesh::id_type const& id, std::filesystem::path const& file_path) {
		bool will_resize = std::size(m_Meshes) == m_Meshes.capacity();
		auto&& new_mesh = m_Meshes.emplace_back(id, file_path);
		if (will_resize) {
			for (auto&& mesh : std::span{ std::data(m_Meshes), std::size(m_Meshes) - 1 }) {
				m_MeshMap.at(mesh.id()) = &mesh;
			}
		}
		(void)m_MeshMap.emplace(id, &new_mesh);
		return m_Meshes.back();
	}
	Image& AssetManagementSystem::insertImage(Image::id_type const& id, std::filesystem::path const& file_path) {
		bool will_resize = std::size(m_Images) == m_Images.capacity();
		auto&& new_image = m_Images.emplace_back(id, file_path);
		if (will_resize) {
			for (auto&& image : std::span{ std::data(m_Images), std::size(m_Images) - 1 }) {
				m_ImageMap.at(image.id()) = &image;
			}
		}
		(void)m_ImageMap.emplace(id, &new_image);
		return m_Images.back();
	}
	LayeredTexture& AssetManagementSystem::insertLayeredTexture(LayeredTexture::id_type const& id) {
		bool will_resize = std::size(m_LayeredTextures) == m_LayeredTextures.capacity();
		auto&& new_layered_texture = m_LayeredTextures.emplace_back(id);
		if (will_resize) {
			for (auto&& layered_texture : std::span{ std::data(m_LayeredTextures), std::size(m_LayeredTextures) - 1 }) {
				m_LayeredTextureMap.at(layered_texture.id()) = &layered_texture;
			}
		}
		(void)m_LayeredTextureMap.emplace(id, &new_layered_texture);
		return m_LayeredTextures.back();
	}

	Mesh& AssetManagementSystem::fetchMesh(Mesh::id_type const& id) noexcept {
#ifdef _DEBUG
		if (!m_MeshMap.contains(id)) {
			SPDLOG_ERROR("Mesh name not found: {}", id);
		}
#endif
		return *m_MeshMap.at(id);
	}
	Image& AssetManagementSystem::fetchImage(Image::id_type const& id) noexcept {
#ifdef _DEBUG
		if (!m_ImageMap.contains(id)) {
			SPDLOG_ERROR("Image name not found: {}", id);
		}
#endif
		return *m_ImageMap.at(id);
	}
	LayeredTexture& AssetManagementSystem::fetchLayeredTexture(LayeredTexture::id_type const& id) noexcept {
#ifdef _DEBUG
		if (!m_LayeredTextureMap.contains(id)) {
			SPDLOG_ERROR("Texture name not found: {}", id);
		}
#endif
		return *m_LayeredTextureMap.at(id);
	}

	std::span<Mesh> AssetManagementSystem::meshes() noexcept {
		return m_Meshes;
	}
	std::span<Image> AssetManagementSystem::images() noexcept {
		return m_Images;
	}
	std::span<LayeredTexture> AssetManagementSystem::layered_textures() noexcept {
		return m_LayeredTextures;
	}

	AssetManagementSystem AMS;
}