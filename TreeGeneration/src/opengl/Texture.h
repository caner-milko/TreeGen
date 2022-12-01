#pragma once
#include <glad/glad.h>
#include <string>
#include "../Definitions.h"
using TextureHandle = uint32;
enum class TextureWrapping
{
	REPEAT = GL_REPEAT, MIRRORED_REPEAT = GL_MIRRORED_REPEAT, CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE, CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER
};
enum class TextureFiltering
{
	NONE = -1, LINEAR = GL_LINEAR, NEAREST = GL_NEAREST, LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR
};
class Texture {
public:
	Texture(std::string_view path, TextureWrapping wrapping = TextureWrapping::REPEAT,
		TextureFiltering minFiltering = TextureFiltering::LINEAR_MIPMAP_LINEAR, TextureFiltering maxFiltering = TextureFiltering::LINEAR);
	void bind();
	void destroy();
private:
	const std::string path;
	TextureHandle handle;
};