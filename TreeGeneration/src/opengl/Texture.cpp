#include "./Texture.h"
#include "./ResourceManager.h"
Texture::Texture(std::string_view path, TextureWrapping wrapping, TextureFiltering minFiltering, TextureFiltering maxFiltering) : path(path)
{
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_2D, handle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int32_t)wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int32_t)wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int32_t)minFiltering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int32_t)maxFiltering);

	ImageDataPtr imageData = ResourceManager::getInstance().readImageFile(path);

	if (imageData != nullptr) {
		if (imageData->nrChannels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageData->width, imageData->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData->data);
		}
		else if (imageData->nrChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageData->width, imageData->height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
}

void Texture::bind()
{
	glBindTexture(GL_TEXTURE_2D, handle);
}

void Texture::destroy()
{
	glDeleteTextures(1, &handle);
}
