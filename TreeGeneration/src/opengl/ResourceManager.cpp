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

sp<Image> loadJPG(std::string_view imgPath)
{
	stbi_set_flip_vertically_on_load(false);
	int width, height, nrChannels;
	uint8_t* data = stbi_load(imgPath.data(), &width, &height, &nrChannels, 0);

	if (!data)
	{
		std::cout << "Error while loading texture." << std::endl;
		return nullptr;
	}

	return std::make_shared<Image>(data, width, height, nrChannels);
}

sp<Image> loadPNG(std::string_view imgPath)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	uint8_t* data = stbi_load(imgPath.data(), &width, &height, &nrChannels, 0);

	if (!data)
	{
		std::cout << "Error while loading texture." << std::endl;
		return nullptr;
	}

	return std::make_shared<Image>(data, width, height, nrChannels);
}

sp<Image> ResourceManager::readImageFile(std::string_view path) const
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

sp<Shader> ResourceManager::createShader(std::string_view vertexPath, std::string_view fragmentPath) const
{
	std::string vertexSrc = readTextFile(vertexPath);
	std::string fragSrc = readTextFile(fragmentPath);
	return std::make_shared<Shader>(vertexSrc, fragSrc);
}

sp<Texture> ResourceManager::createTexture(std::string_view imagePath, Texture::TextureCreateData createData, bool dataFromImage) const
{
	sp<Image> img = readImageFile(imagePath);
	if (dataFromImage) {
		createData.size = ivec3(img->width, img->height, 0);
		createData.textureFormat = img->nrChannels == 4 ? Format::R8G8B8A8_SRGB : Format::R8G8B8_SRGB;
	}
	auto tex = std::make_shared<Texture>(createData);
	Texture::TextureUploadData uploadData = {};
	uploadData.size = ivec3(img->width, img->height, 0);
	uploadData.inputFormat = img->nrChannels == 4 ? UploadFormat::RGBA : UploadFormat::RGB;
	uploadData.data = img->data;
	tex->subImage(uploadData);
	tex->genMipMaps();
	return tex;
}

sp<CubemapTexture> ResourceManager::createCubemapTexture(std::array<std::string_view, 6> facePaths, Texture::TextureCreateData createData, bool dataFromImage)
{
	std::array<sp<Image>, 6> faces;
	std::array<const void*, 6> datas;
	Texture::TextureUploadData uploadData = {};
	for (int i = 0; i < 6; i++) {
		sp<Image> img = readImageFile(facePaths[i]);
		if(i == 0) {
			if (dataFromImage) {
				createData.size = ivec3(img->width, img->height, 0);
				createData.textureFormat = img->nrChannels == 4 ? Format::R8G8B8A8_SRGB : Format::R8G8B8_SRGB;
			}
			uploadData.size = ivec3(img->width, img->height, 1);
			uploadData.inputFormat = img->nrChannels == 4 ? UploadFormat::RGBA : UploadFormat::RGB;
		}
		faces[i] = img;
		datas[i] = img->data;
	}
	auto tex = std::make_shared<CubemapTexture>(createData);
	tex->uploadFaces(datas, uploadData);
	return tex;
}
