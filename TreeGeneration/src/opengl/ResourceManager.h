#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <string_view>

#include "./Shader.h"
#include "./Texture.h"

struct ImageData {
	uint8_t* data;
	int32 width, height, nrChannels;
	ImageData(uint8_t* data, int32 width, int32 height, int32 nrChannels);
	bool isEmpty();
	~ImageData();
};

using ImageDataPtr = std::unique_ptr<ImageData>;

class ResourceManager {
public:
	static ResourceManager& getInstance() {
		static ResourceManager instance;
		return instance;
	}
	std::string readTextFile(std::string_view path) const;
	ImageDataPtr readImageFile(std::string_view path) const;
	Texture* loadTexture(const std::string& name, std::string_view path);
	Shader* loadShader(const std::string& name, std::string_view vertex_path, std::string_view fragment_path);
	void ClearResources();
private:
	std::unordered_map < std::string, Shader> m_Shaders;
	std::unordered_map < std::string, Texture> m_Textures;
};