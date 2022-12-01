#pragma once
#include "Texture.h"
#include <array>
class CubemapTexture {
public:
	CubemapTexture(std::array<std::string_view, 6> paths, TextureWrapping wrapping = TextureWrapping::CLAMP_TO_EDGE,
		TextureFiltering minFiltering = TextureFiltering::LINEAR, TextureFiltering maxFiltering = TextureFiltering::LINEAR);
	void bind();
	void destroy();
private:
	std::array<std::string, 6> paths;
	TextureHandle handle;
};