#include "Texture.h"
#include "ApiToEnum.h"

#include <glm/gtc/integer.hpp>
namespace tgen::graphics::gl {
using namespace tgen::graphics::gl::detail;
bool Texture::init(TextureCreateData data, bool reinit)
{
	if (handle != 0 && reinit) {
		this->~Texture();
	}
	assert(handle == 0);
	this->data = data;
	glCreateTextures(ImageTypeToGL(data.imageType), 1, &handle);
	auto a = AddressModeToGL(data.wrapping);
	auto ab = AddressModeToGL(data.wrapping);

	glTextureParameteri(handle, GL_TEXTURE_WRAP_S, AddressModeToGL(data.wrapping));
	glTextureParameteri(handle, GL_TEXTURE_WRAP_T, AddressModeToGL(data.wrapping));
	glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, FilterToGL(data.minFiltering));
	glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, FilterToGL(data.maxFiltering));

	switch (data.imageType)
	{
	case ImageType::TEX_1D:
		data.mipLevels = data.mipLevels == -1 ? 1 + glm::log2(data.size.x) : data.mipLevels;
		glTextureStorage1D(handle, data.mipLevels, FormatToGL(data.textureFormat), data.size.x);
		break;
	case ImageType::TEX_2D:
		data.mipLevels = data.mipLevels == -1 ? 1 + glm::log2(glm::max(data.size.x, data.size.y)) : data.mipLevels;
		glTextureStorage2D(handle,
			data.mipLevels,
			FormatToGL(data.textureFormat),
			data.size.x,
			data.size.y);
		break;
	case ImageType::TEX_3D:
		data.mipLevels = data.mipLevels == -1 ? 1 + glm::log2(glm::max(glm::max(data.size.x, data.size.y), data.size.z))
			: data.mipLevels;
		glTextureStorage3D(handle,
			data.mipLevels,
			FormatToGL(data.textureFormat),
			data.size.x,
			data.size.y,
			data.size.z);
		break;
	case ImageType::TEX_CUBEMAP:
		data.mipLevels = data.mipLevels == -1 ? 1 + glm::log2(glm::max(data.size.x, data.size.y)) : data.mipLevels;
		glTextureStorage2D(handle,
			data.mipLevels,
			FormatToGL(data.textureFormat),
			data.size.x,
			data.size.y);
		break;
		// case ImageType::TEX_CUBEMAP_ARRAY:
		//   ASSERT(false);
		//   break;
	case ImageType::TEX_2D_MULTISAMPLE:
		data.mipLevels = data.mipLevels == -1 ? 1 + glm::log2(glm::max(data.size.x, data.size.y)) : data.mipLevels;
		glTextureStorage2DMultisample(handle,
			SampleCountToGL(data.sampleCount),
			FormatToGL(data.textureFormat),
			data.size.x,
			data.size.y,
			GL_FALSE);
		break;
	default: break;
	}
	return handle;
}
void Texture::subImage(TextureUploadData uploadData)
{
	assert(handle);
	switch (uploadData.dimensions) {
	case UploadDimension::ONE:
		glTextureSubImage1D(handle, uploadData.level, uploadData.offset.x, uploadData.size.x,
			UploadFormatToGL(uploadData.inputFormat), UploadTypeToGL(uploadData.type), uploadData.data);
		return;
	case UploadDimension::TWO:
		glTextureSubImage2D(handle, uploadData.level, uploadData.offset.x, uploadData.offset.y,
			uploadData.size.x, uploadData.size.y, UploadFormatToGL(uploadData.inputFormat),
			UploadTypeToGL(uploadData.type), uploadData.data);
		return;
	case UploadDimension::THREE:
		glTextureSubImage3D(handle, uploadData.level, uploadData.offset.x, uploadData.offset.y, uploadData.offset.z,
			uploadData.size.x, uploadData.size.y, uploadData.size.z,
			UploadFormatToGL(uploadData.inputFormat), UploadTypeToGL(uploadData.type), uploadData.data);
		return;
	}
	assert(false);
}

void Texture::genMipMaps()
{
	assert(handle);
	glGenerateTextureMipmap(handle);
}
}