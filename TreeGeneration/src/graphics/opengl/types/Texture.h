#pragma once
#include <glad/glad.h>
#include <string>
#include "Common.h"
#include "graphics/Image.h"
#include "graphics/opengl/BasicTypes.h"
namespace tgen::graphics::gl
{
using TextureHandle = uint32;

class Texture
{
public:
	struct TextureCreateData
	{
		ImageType imageType = ImageType::TEX_2D;
		ivec3 size = { 1920, 1080, 1 };
		int32 mipLevels = -1;
		AddressMode wrapping = AddressMode::REPEAT;
		Filter minFiltering = Filter::LINEAR_MIPMAP_LINEAR;
		Filter maxFiltering = Filter::LINEAR;
		Format textureFormat = Format::R8G8B8_SRGB;
		SampleCount sampleCount = SampleCount::SAMPLES_1;
		bool compareEnable = false;
		CompareOp compareOp = CompareOp::NEVER;
	};
	struct TextureUploadData
	{
		UploadDimension dimensions = UploadDimension::TWO;
		UploadFormat inputFormat = UploadFormat::RGBA;
		uint32 level = 0;
		ivec3 offset = ivec3(0);
		ivec3 size = ivec3(1920, 1080, 1);
		UploadType type = UploadType::UBYTE;
		const void* data = nullptr;
	};
	Texture() = default;
	bool init(TextureCreateData data = {}, bool reinit = true);
	virtual ~Texture() { destroy(); }
	Texture(Texture&& other) noexcept : handle(std::exchange(other.handle, 0)), data(std::exchange(other.data, {})) {}
	MOVE_OPERATOR(Texture);
	DELETE_COPY_CONSTRUCTORS(Texture)
		GL_OPERATOR_BOOL(handle);

	virtual void subImage(TextureUploadData uploadData = {});
	void genMipMaps();

	TextureHandle getHandle() const
	{
		return handle;
	}
	TextureCreateData const& getCreateData() const
	{
		return data;
	}
private:
	void destroy() { glDeleteTextures(1, &handle); data = {}; }
	TextureCreateData data;
	TextureHandle handle = 0;
};
}