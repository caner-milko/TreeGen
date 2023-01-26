#include "CubemapTexture.h"
#include "ResourceManager.h"
CubemapTexture::CubemapTexture(TextureCreateData data) 
	: Texture((
		data.imageType = ImageType::TEX_CUBEMAP,
		data.mipLevels = 1,
		data.wrapping = AddressMode::CLAMP_TO_EDGE,
		data.minFiltering = Filter::LINEAR,
		data.maxFiltering = Filter::LINEAR, 
		data))
{
}

void CubemapTexture::uploadFaces(const std::array<const void*, 6> faces, TextureUploadData uploadData)
{
	for (int i = 0; i < 6; i++) {
		uploadData.data = faces[i];
		uploadData.offset.z = i;
		uploadData.size.z = 1;
		uploadData.level = 0;
		uploadData.dimensions = UploadDimension::THREE;
		subImage(uploadData);
	}
}