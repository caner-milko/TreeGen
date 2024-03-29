#include <fstream>
#include <iostream>


#include "ResourceManager.h"

#include <stb_image.h>
#include "util/Util.h"
#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
namespace std
{
template<> struct hash<tgen::graphics::Vertex>
{
	size_t operator()(tgen::graphics::Vertex const& vertex) const
	{
		return ((hash<glm::vec3>()(vertex.pos) ^
			(hash<glm::vec2>()(vertex.texCoords) << 1)) >> 1) ^
			(hash<glm::vec3>()(vertex.normal) << 1);
	}
};
}
namespace tgen::graphics
{
using namespace gl;
std::string ResourceManager::readTextFile(std::string_view path) const
{
	try
	{
		std::ifstream ifs(path.data());
		return std::string((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
	} catch (std::ifstream::failure e)
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
	uploadImageToTexture(*tex, *img);
	return tex;
}

void ResourceManager::uploadImageToTexture(Texture& tex, Image& img, gl::Texture::TextureUploadData uploadData) const
{
	uploadData.size = ivec3(img.width, img.height, 0);
	uploadData.inputFormat = img.nrChannels == 4 ? UploadFormat::RGBA : UploadFormat::RGB;
	uploadData.data = img.data;
	tex.subImage(uploadData);
	tex.genMipMaps();
}

ru<CubemapTexture> ResourceManager::createCubemapTexture(std::array<std::string_view, 6> facePaths, Texture::TextureCreateData createData, bool dataFromImage)
{
	std::array<ru<Image>, 6> faces;
	std::array<const void*, 6> datas;
	Texture::TextureUploadData uploadData = {};
	for (int i = 0; i < 6; i++)
	{
		auto img = readImageFile(facePaths[i]);
		if (i == 0)
		{
			if (dataFromImage)
			{
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
ru<CompleteMesh<Vertex, IndexType::UNSIGNED_INT>> ResourceManager::objToMesh(const char* modelPath) const
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath))
	{
		std::cout << err << std::endl;
		return nullptr;
	}
	std::vector<Vertex> vertices;
	std::unordered_map<Vertex, uint32> uniqueVertices;
	std::vector<uint32> indices;
	for (const auto& index : shapes[0].mesh.indices)
	{
		Vertex vertex{};

		vertex.pos = {
			attrib.vertices[3 * index.vertex_index + 0],
			attrib.vertices[3 * index.vertex_index + 1],
			attrib.vertices[3 * index.vertex_index + 2]
		};

		vertex.texCoords = {
			attrib.texcoords[2 * index.texcoord_index + 0],
			attrib.texcoords[2 * index.texcoord_index + 1]
		};

		vertex.normal = {
			attrib.normals[3 * index.normal_index + 0],
			attrib.normals[3 * index.normal_index + 1],
			attrib.normals[3 * index.normal_index + 2]
		};

		uint32 curIndex = 0;
		if (auto it = uniqueVertices.find(vertex); it != uniqueVertices.end())
		{
			if (it->first != vertex)
			{
				std::cout << "aaaaaaaa" << std::endl;
			}
			curIndex = it->second;
		}
		else
		{
			curIndex = uniqueVertices.try_emplace(vertex, (uint32)vertices.size()).first->second;
			vertices.push_back(vertex);
		}
		indices.push_back(curIndex);
	}

	auto mesh = std::make_unique<CompleteMesh<Vertex, IndexType::UNSIGNED_INT>>();

	mesh->vbo.init(vertices);
	mesh->inputState.vertexBindingDescriptions = Vertex::bindingDescription;
	mesh->ebo.init(indices);

	return mesh;
}
}
