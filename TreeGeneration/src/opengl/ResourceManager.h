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
	ImagePtr readImageFile(std::string_view path) const;
	Texture* loadTexture(const std::string& name, std::string_view path, const Texture::TextureData& data);
	CubemapTexture* loadCubemapTexture(const std::string& name, std::string_view right, std::string_view left,
		std::string_view top, std::string_view bottom, std::string_view front, std::string_view back, const CubemapTexture::CubemapTextureData& data);
	Shader* loadShader(const std::string& name, std::string_view vertex_path, std::string_view fragment_path);

	void destroyShader(const std::string& name);
	void destroyTexture(const std::string& name);
	void destroyCubemapTexture(const std::string& name);


	void ClearResources();
private:
	std::unordered_map < std::string, std::unique_ptr<Shader>> m_Shaders;
	std::unordered_map < std::string, std::unique_ptr<Texture>> m_Textures;
	std::unordered_map < std::string, std::unique_ptr<CubemapTexture>> m_CubemapTextures;
};