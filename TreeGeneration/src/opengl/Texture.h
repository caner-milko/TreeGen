#pragma once
#include <glad/glad.h>
#include <string>
#include "../Definitions.h"
using TextureHandle = uint32;

class Texture {
public:
	enum TextureWrapping
	{
		REPEAT = GL_REPEAT, MIRRORED_REPEAT = GL_MIRRORED_REPEAT, CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE, CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
	};
	enum TextureFiltering
	{
		NONE = -1, LINEAR = GL_LINEAR, NEAREST = GL_NEAREST, LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
	};
	enum TextureColorSpace
	{
		RAW, SRGB
	};
	struct TextureData {
		TextureWrapping wrapping = Texture::REPEAT;
		TextureFiltering minFiltering = Texture::LINEAR_MIPMAP_LINEAR;
		TextureFiltering maxFiltering = Texture::LINEAR;
		TextureColorSpace colorSpace = Texture::SRGB;
		bool generateMipMap = true;
	};
	static int selectInternalFormat(TextureColorSpace colorSpace, bool alpha);
	Texture(std::string_view path, const TextureData& data);
	void bind();
	TextureHandle getHandle() const;
	void destroy();
private:
	const std::string path;
	TextureData data;
	TextureHandle handle;
};