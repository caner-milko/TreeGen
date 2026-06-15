#include "EditableMap.h"
#include "ResourceManager.h"
namespace tgen::gen
{
using namespace graphics;
using namespace graphics::gl;
void EditableMap::initTexture()
{
	texture = std::make_shared<Texture>();
	Texture::TextureCreateData createData;
	createData.size = ivec3(image->width, image->height, 0);
	createData.textureFormat = image->nrChannels == 4 ? Format::R8G8B8A8_UNORM : Format::R8G8B8_UNORM;
	texture->init(createData);
	graphics::ResourceManager::getInstance().uploadImageToTexture(*texture, *image);
}
void EditableMap::updateImage()
{
	Texture::TextureDownloadData downloadData;
	downloadData.bufSize = image->width * image->height * image->nrChannels;
	downloadData.data = image->data;
	downloadData.downloadFormat = image->nrChannels == 4 ? UploadFormat::RGBA : UploadFormat::RGB;
	texture->getImage(downloadData);
	textureEdited = false;
}

}