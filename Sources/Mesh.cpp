#include "Common/Util.hpp"
#include "Mesh.hpp"

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <tiny_gltf/tiny_gltf.h>
#include <tiny_obj_loader/tiny_obj_loader.h>
#include <ranges>
#include <numeric>
#include <algorithm>
#pragma warning(pop)

namespace API_NAME {
	Mesh::Mesh(Mesh::id_type const& id, std::filesystem::path const& file_path) : m_IsLoaded{ false }, m_ID{ id } {
		load(file_path);
	}
	Mesh::Mesh(Mesh const& mesh) : m_IsLoaded{ mesh.m_IsLoaded }, m_ID{ mesh.m_ID }, m_Vertices { std::make_unique<Vertex3D[]>(std::size(mesh.m_VertexView)) }, m_Indices{ std::make_unique<u32[]>(std::size(mesh.m_IndexView)) } {
		m_VertexView = { m_Vertices.get(), std::size(mesh.m_VertexView) };
		m_IndexView = { m_Indices.get(), std::size(mesh.m_IndexView) };
		(void)std::copy(std::cbegin(mesh.m_VertexView), std::cend(mesh.m_VertexView), std::begin(mesh.m_VertexView));
		(void)std::copy(std::cbegin(mesh.m_IndexView), std::cend(mesh.m_IndexView), std::begin(mesh.m_IndexView));
	}
	
	void Mesh::operator=(Mesh const& mesh) {
		m_IsLoaded = mesh.m_IsLoaded;
		m_ID = mesh.m_ID;
		if (std::size(m_VertexView) != std::size(mesh.m_VertexView)) {
			m_Vertices = std::make_unique<Vertex3D[]>(std::size(mesh.m_VertexView));
			m_VertexView = { m_Vertices.get(), std::size(mesh.m_VertexView) };
		}
		if (std::size(m_IndexView) != std::size(mesh.m_IndexView)) {
			m_Indices = std::make_unique<u32[]>(std::size(mesh.m_IndexView));
			m_IndexView = { m_Indices.get(), std::size(mesh.m_IndexView) };
		}
		(void)std::copy(std::cbegin(mesh.m_VertexView), std::cend(mesh.m_VertexView), std::begin(mesh.m_VertexView));
		(void)std::copy(std::cbegin(mesh.m_IndexView), std::cend(mesh.m_IndexView), std::begin(mesh.m_IndexView));
	}

	void Mesh::load(std::filesystem::path const& file_path) {
		if (m_IsLoaded) {
			free();
		}

		SupportedFileFormat file_format = SupportedFileFormat::Null;
		auto const file_format_string = file_path.extension().string();
		if (file_format_string == ".obj") {
			file_format = SupportedFileFormat::Obj;
		}
#ifdef _DEBUG
		if (file_format == SupportedFileFormat::Null) {
			SPDLOG_ERROR("Unsupported Mesh File Format: {}", file_format_string);
		}
#endif

		switch (file_format) {
		case SupportedFileFormat::Obj: {
			tinyobj::attrib_t attributes;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string warn, err;


			[[maybe_unused]] bool result = tinyobj::LoadObj(&attributes, &shapes, &materials, &warn, &err, file_path.string().c_str());
#ifdef _DEBUG
			if (!result) {
				SPDLOG_ERROR("tinyobj::LoadObject failed to load object:\n{}\n{}", warn, err);
			}
#endif
			/* Review for performance */
			auto const index_count = std::accumulate(std::cbegin(shapes), std::cend(shapes), 0ULL, [](auto&& sum, auto&& shape) { return sum + std::size(shape.mesh.indices); });
			m_Indices = std::make_unique<u32[]>(index_count);
			m_IndexView = { m_Indices.get(), index_count };
			std::unordered_map<Vertex3D, u32> vertex_index_map;
			vertex_index_map.reserve(index_count);
			std::unique_ptr<Vertex3D[]> ordered_vertices = std::make_unique<Vertex3D[]>(index_count);
			std::size_t unique_count = 0;
			for (std::size_t i = 0; auto && shape : shapes) {
				for (auto&& index : shape.mesh.indices) {
					Vertex3D const vertex = {
						.XYZ = { attributes.vertices[3ULL * index.vertex_index], attributes.vertices[3ULL * index.vertex_index + 1], attributes.vertices[3ULL * index.vertex_index + 2] },
						.colour = { 1.0f, 1.0f, 1.0f },
						.UV = { attributes.texcoords[2ULL * index.texcoord_index], 1.0f - attributes.texcoords[2ULL * index.texcoord_index + 1] }
					};
					if (!vertex_index_map.contains(vertex)) {
						vertex_index_map.emplace(vertex, unique_count);
						ordered_vertices[unique_count++] = vertex;
					}
					m_Indices[i++] = vertex_index_map.at(vertex);
				}
			}
			m_Vertices = std::make_unique<Vertex3D[]>(unique_count);
			std::memcpy(m_Vertices.get(), ordered_vertices.get(), unique_count * sizeof(Vertex3D));
			m_VertexView = { m_Vertices.get(), unique_count };
			m_IsLoaded = true;
		}
		case SupportedFileFormat::glTF: {
			tinygltf::TinyGLTF instance;
			tinygltf::Model model;

			std::string warning, error;

			bool result = instance.LoadASCIIFromFile(&model, &error, &warning, file_path.string());
			//if (!result) {
			//	SPDLOG_ERROR("Failed to load glTF file \"{}\":\n{}\n{}", file_path, )
			//}

			break;
		}
		case SupportedFileFormat::Null:
			break;
		}
	}
	void Mesh::free() {
		m_IsLoaded = false;
		m_Vertices.reset();
		m_VertexView = { m_Vertices.get(), NULL };
		m_Indices.reset();
		m_IndexView = { m_Indices.get(), NULL };
	}

	bool Mesh::isLoaded() const noexcept {
		return m_IsLoaded;
	}
	Mesh::id_type const& Mesh::id() const noexcept {
		return m_ID;
	}
	std::span<Vertex3D> Mesh::vertices() noexcept {
		return m_VertexView;
	}
	std::span<u32> Mesh::indices() noexcept {
		return m_IndexView;
	}
	
}