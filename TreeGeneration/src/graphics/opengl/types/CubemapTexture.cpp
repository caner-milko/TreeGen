#include "CubemapTexture.h"
namespace tgen::graphics::gl {
bool CubemapTexture::init(TextureCreateData data, bool reinit)
{
	data.imageType = ImageType::TEX_CUBEMAP;
	data.mipLevels = 1;
	data.wrapping = AddressMode::CLAMP_TO_EDGE;
	data.minFiltering = Filter::LINEAR;
	data.maxFiltering = Filter::LINEAR;
	return Texture::init(data, reinit);
}

void CubemapTexture::uploadFaces(const std::array<const void*, 6> faces, TextureUploadData uploadData)
{
	assert(Texture::getHandle());
	for (int i = 0; i < 6; i++) {
		uploadData.data = faces[i];
		uploadData.offset.z = i;
		uploadData.size.z = 1;
		uploadData.level = 0;
		uploadData.dimensions = UploadDimension::THREE;
		subImage(uploadData);
	}
}
}