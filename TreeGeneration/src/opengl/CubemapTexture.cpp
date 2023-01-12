#include "CubemapTexture.h"
#include "ResourceManager.h"
CubemapTexture::CubemapTexture(std::array<std::string_view, 6> paths, const CubemapTextureData& data) : data(data)
{
	glGenTextures(1, &handle);
	glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

	for (int i = 0; i < paths.size(); i++) {
		ImagePtr imageData = ResourceManager::getInstance().readImageFile(paths[i]);
		int space = Texture::selectInternalFormat(data.colorSpace, imageData->nrChannels == 4);
		if (imageData->nrChannels == 4) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, space, imageData->width, imageData->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData->data);
		}
		else if (imageData->nrChannels == 3) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, space, imageData->width, imageData->height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData->data);
		}
		this->paths[i] = paths[i];
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, (int32_t)data.wrapping);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, (int32_t)data.wrapping);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, (int32_t)data.wrapping);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, (int32_t)data.minFiltering);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, (int32_t)data.maxFiltering);

}

void CubemapTexture::bind()
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
}

void CubemapTexture::destroy()
{
	glDeleteTextures(1, &handle);
}
