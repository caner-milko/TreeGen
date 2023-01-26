#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>

#include "./Shader.h"
#include "./Texture.h"
#include "./CubemapTexture.h"
#include "Image.h"


class ResourceManager {
public:
	static ResourceManager& getInstance() {
		static ResourceManager instance;
		return instance;
	}
	std::string readTextFile(std::string_view path) const;
	sp<Image> readImageFile(std::string_view path) const;

	sp<Shader> createShader(std::string_view vertexPath, std::string_view fragmentPath) const;
	sp<Texture> createTexture(std::string_view imagePath, Texture::TextureCreateData createData = {}, bool dataFromImage = true) const;
	sp<CubemapTexture> createCubemapTexture(
		std::array<std::string_view, 6> facePaths,
		Texture::TextureCreateData createData = {},
		bool dataFromImage = true);
};