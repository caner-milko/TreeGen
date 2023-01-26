#pragma once
#include <glad/glad.h>
#include <string>
#include "../Definitions.h"
#include "./Image.h"
#include "./BasicTypes.h"
using TextureHandle = uint32;

class Texture {
public:
	struct TextureCreateData {
		ImageType imageType = ImageType::TEX_2D;
		ivec3 size = {1920, 1080, 1};
		int32 mipLevels = -1;
		AddressMode wrapping = AddressMode::REPEAT;
		Filter minFiltering = Filter::LINEAR_MIPMAP_LINEAR;
		Filter maxFiltering = Filter::LINEAR;
		Format textureFormat = Format::R8G8B8_SRGB;
		SampleCount sampleCount = SampleCount::SAMPLES_1;
	};
	struct TextureUploadData {
		UploadDimension dimensions = UploadDimension::TWO;
		UploadFormat inputFormat = UploadFormat::RGBA;
		uint32 level = 0;
		ivec3 offset = ivec3(0);
		ivec3 size = ivec3(1920, 1080, 1);
		UploadType type = UploadType::UBYTE;
		const void* data = nullptr;
	};
	Texture(TextureCreateData data = {});
	virtual ~Texture();
	void bindTo(int32 unit) const;
	DELETE_COPY_CONSTRUCTORS(Texture)

	virtual void subImage(TextureUploadData uploadData = {});
	void genMipMaps();
	TextureHandle getHandle() const {
		return handle;
	}
	TextureCreateData getCreateData() const {
		return data;
	}
private:
	TextureCreateData data;
	TextureHandle handle = 0;
};