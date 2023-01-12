#include <fstream>
#include <iostream>


#include "ResourceManager.h"

#include <stb_image.h>
#include "../Util.hpp"
#include <optional>


std::string ResourceManager::readTextFile(std::string_view path) const
{
	try
	{
		std::ifstream ifs(path.data());
		return std::string((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
	}
	catch (std::ifstream::failure e)
	{
		std::cerr << "Resource Manager: Couldn't read text file at path: " << path << std::endl;
	}
	return "";
}


ImagePtr loadJPG(std::string_view imgPath)
{
	stbi_set_flip_vertically_on_load(false);
	int width, height, nrChannels;
	uint8_t* data = stbi_load(imgPath.data(), &width, &height, &nrChannels, 0);

	if (!data)
	{
		std::cout << "Error while loading texture." << std::endl;
		return nullptr;
	}

	return std::make_unique<Image>(data, width, height, nrChannels);
}

ImagePtr loadPNG(std::string_view imgPath)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	uint8_t* data = stbi_load(imgPath.data(), &width, &height, &nrChannels, 0);

	if (!data)
	{
		std::cout << "Error while loading texture." << std::endl;
		return nullptr;
	}

	return std::make_unique<Image>(data, width, height, nrChannels);
}

ImagePtr ResourceManager::readImageFile(std::string_view path) const
{
	if (util::endsWith(path, ".jpg") || util::endsWith(path, ".jpeg"))
	{
		return loadJPG(path);
	}
	else if (util::endsWith(path, ".png"))
	{
		return loadPNG(path);
	}
	else
	{
		std::cout << "Image format not supported." << std::endl;
		return nullptr;
	}
}
Texture* ResourceManager::loadTexture(const std::string& name, std::string_view path, const Texture::TextureData& data)
{
	auto found = m_Textures.find(name);
	if (found != m_Textures.end())
		return found->second.get();
	return m_Textures.insert({ name, std::make_unique<Texture>(path, data) }).first->second.get();
}
CubemapTexture* ResourceManager::loadCubemapTexture(const std::string& name, std::string_view right, std::string_view left,
	std::string_view top, std::string_view bottom, std::string_view front, std::string_view back, const CubemapTexture::CubemapTextureData& data)
{
	auto found = m_CubemapTextures.find(name);
	if (found != m_CubemapTextures.end())
		return found->second.get();
	return m_CubemapTextures.insert({ name, std::make_unique<CubemapTexture>(std::array<std::string_view, 6>{right, left, top, bottom, front, back}, data) }).first->second.get();
}
Shader* ResourceManager::loadShader(const std::string& name, std::string_view vertex_path, std::string_view fragment_path)
{
	auto found = m_Shaders.find(name);
	if (found != m_Shaders.end())
		return found->second.get();
	return m_Shaders.insert({ name, std::make_unique<Shader>(vertex_path, fragment_path) }).first->second.get();
}

void ResourceManager::destroyShader(const std::string& name)
{
	auto& map = m_Shaders;
	auto found = map.find(name);
	if (found == map.end())
		return;
	found->second.get()->destroy();
	map.erase(name);
}

void ResourceManager::destroyTexture(const std::string& name)
{
	auto& map = m_Textures;
	auto found = map.find(name);
	if (found == map.end())
		return;
	found->second.get()->destroy();
	map.erase(name);
}

void ResourceManager::destroyCubemapTexture(const std::string& name)
{
	auto& map = m_CubemapTextures;
	auto found = map.find(name);
	if (found == map.end())
		return;
	found->second.get()->destroy();
	map.erase(name);
}

void ResourceManager::ClearResources()
{
	m_Shaders.clear();
}

