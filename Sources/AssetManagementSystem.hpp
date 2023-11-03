#pragma once

#include "Common/Types.hpp"
#include "Mesh.hpp"
#include "Image.hpp"
#include "LayeredTexture.hpp"

#pragma warning(push, 0)
#include <limits>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#pragma warning(pop)

namespace API_NAME {
	class AssetManagementSystem {
	public:
		AssetManagementSystem();

		Mesh& insertMesh(std::filesystem::path const& file_path, Mesh::id_type const& id);
		Image& insertImage(std::filesystem::path const& file_path, Image::id_type const& id);
		LayeredTexture& insertLayeredTexture(LayeredTexture::id_type const& id);

		Mesh& fetchMesh(Mesh::id_type const& id) noexcept;
		Image& fetchImage(Image::id_type const& id) noexcept;
		LayeredTexture& fetchLayeredTexture(LayeredTexture::id_type const& id) noexcept;

		std::span<Mesh> meshes() noexcept;
		std::span<Image> images() noexcept;
		std::span<LayeredTexture> layered_textures() noexcept;

	private:
		std::unordered_map<Mesh::id_type, Mesh*> m_MeshMap;
		std::vector<Mesh> m_Meshes;
		std::unordered_map<Image::id_type, Image*> m_ImageMap;
		std::vector<Image> m_Images;
		std::unordered_map<LayeredTexture::id_type, LayeredTexture*> m_LayeredTextureMap;
		std::vector<LayeredTexture> m_LayeredTextures;
	};

	extern AssetManagementSystem AMS;
}