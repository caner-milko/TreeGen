#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>

#include "opengl/types/Shader.h"
#include "opengl/types/Texture.h"
#include "opengl/types/CubemapTexture.h"
#include "Mesh.h"
#include "Image.h"
#include <tiny_obj_loader.h>
namespace tgen::graphics
{

class ResourceManager
{
public:
	static ResourceManager& getInstance()
	{
		static ResourceManager instance;
		return instance;
	}
	std::string readTextFile(std::string_view path) const;
	ru<Image> readImageFile(std::string_view path) const;

	ru<gl::Shader> createShader(std::string_view vertexPath, std::string_view fragmentPath) const;
	ru<gl::Texture> createTexture(std::string_view imagePath, gl::Texture::TextureCreateData createData = {}, bool sizeFromImage = true, bool formatFromImage = true, bool SRGB = true) const;
	void uploadImageToTexture(gl::Texture& tex, Image& img, gl::Texture::TextureUploadData uploadData = {}) const;
	ru<gl::CubemapTexture> createCubemapTexture(
		std::array<std::string_view, 6> facePaths,
		gl::Texture::TextureCreateData createData = {},
		bool dataFromImage = true);
	ru<CompleteMesh<Vertex, gl::IndexType::UNSIGNED_INT>> objToMesh(const char* modelPath) const;
};
}