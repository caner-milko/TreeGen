#include "./Texture.h"
#include "./ResourceManager.h"
int Texture::selectInternalFormat(TextureColorSpace colorSpace, bool alpha)
{
	if (alpha) {
		switch (colorSpace) {
		case SRGB:
			return GL_SRGB_ALPHA;
		case RAW:
			return GL_RGBA;
		default:
			return GL_RGBA;
		}
	}
	else {
		int space = GL_RGB;
		switch (colorSpace) {
		case SRGB:
			return GL_SRGB;
		case RAW:
			return GL_RGB;
		default:
			return GL_RGB;
		}
	}
	return GL_RGB;
}
Texture::Texture(std::string_view path, const TextureData& data) : path(path), data(data)
{
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int32_t)data.wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int32_t)data.wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int32_t)data.minFiltering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int32_t)data.maxFiltering);

	ImagePtr imageData = ResourceManager::getInstance().readImageFile(path);

	if (imageData != nullptr) {
		if (imageData->nrChannels == 4) {

			glTexImage2D(GL_TEXTURE_2D, 0, selectInternalFormat(data.colorSpace, true), imageData->width, imageData->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData->data);
		}
		else if (imageData->nrChannels == 3) {

			glTexImage2D(GL_TEXTURE_2D, 0, selectInternalFormat(data.colorSpace, false), imageData->width, imageData->height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->data);
		}
		if (data.generateMipMap)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, handle);
}

TextureHandle Texture::getHandle() const
{
	return handle;
}

void Texture::destroy()
{
	glDeleteTextures(1, &handle);
}
