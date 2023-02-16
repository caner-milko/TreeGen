#include <fstream>
#include <iostream>


#include "ResourceManager.h"

#include <stb_image.h>
#include "util/Util.h"
#include <optional>
namespace tgen::graphics {
using namespace gl;
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

ru<Image> loadJPG(std::string_view imgPath)
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

ru<Image> loadPNG(std::string_view imgPath)
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

ru<Image> ResourceManager::readImageFile(std::string_view path) const
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

ru<Shader> ResourceManager::createShader(std::string_view vertexPath, std::string_view fragmentPath) const
{
	std::string vertexSrc = readTextFile(vertexPath);
	std::string fragSrc = readTextFile(fragmentPath);
	auto shader = std::make_unique<Shader>();
	shader->init(vertexSrc, fragSrc);
	return shader;
}

ru<Texture> ResourceManager::createTexture(std::string_view imagePath, Texture::TextureCreateData createData, bool sizeFromImage, bool formatFromImage, bool SRGB) const
{
	auto img = readImageFile(imagePath);
	if (sizeFromImage)
		createData.size = ivec3(img->width, img->height, 0);
	if (formatFromImage)
		createData.textureFormat = img->nrChannels == 4 ? (SRGB ? Format::R8G8B8A8_SRGB : Format::R8G8B8A8_UNORM) : (SRGB ? Format::R8G8B8_SRGB : Format::R8G8B8_UNORM);
	auto tex = std::make_unique<Texture>();
	tex->init(createData);
	Texture::TextureUploadData uploadData = {};
	uploadData.size = ivec3(img->width, img->height, 0);
	uploadData.inputFormat = img->nrChannels == 4 ? UploadFormat::RGBA : UploadFormat::RGB;
	uploadData.data = img->data;
	tex->subImage(uploadData);
	tex->genMipMaps();
	return tex;
}

ru<CubemapTexture> ResourceManager::createCubemapTexture(std::array<std::string_view, 6> facePaths, Texture::TextureCreateData createData, bool dataFromImage)
{
	std::array<ru<Image>, 6> faces;
	std::array<const void*, 6> datas;
	Texture::TextureUploadData uploadData = {};
	for (int i = 0; i < 6; i++) {
		auto img = readImageFile(facePaths[i]);
		if (i == 0) {
			if (dataFromImage) {
				createData.size = ivec3(img->width, img->height, 0);
				createData.textureFormat = img->nrChannels == 4 ? Format::R8G8B8A8_SRGB : Format::R8G8B8_SRGB;
			}
			uploadData.size = ivec3(img->width, img->height, 1);
			uploadData.inputFormat = img->nrChannels == 4 ? UploadFormat::RGBA : UploadFormat::RGB;
		}
		faces[i] = std::move(img);
		datas[i] = faces[i]->data;
	}
	auto tex = std::make_unique<CubemapTexture>();
	tex->init(createData);
	tex->uploadFaces(datas, uploadData);
	return tex;
}
}