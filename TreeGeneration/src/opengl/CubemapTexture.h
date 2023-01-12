#pragma once
#include "Texture.h"
#include <array>
class CubemapTexture {
public:
	struct CubemapTextureData {
		Texture::TextureWrapping wrapping = Texture::CLAMP_TO_EDGE;
		Texture::TextureFiltering minFiltering = Texture::LINEAR;
		Texture::TextureFiltering maxFiltering = Texture::LINEAR;
		Texture::TextureColorSpace colorSpace = Texture::SRGB;
	};
	CubemapTexture(std::array<std::string_view, 6> paths, const CubemapTextureData& data);
	void bind();
	void destroy();
private:
	std::array<std::string, 6> paths;
	CubemapTextureData data;
	TextureHandle handle;
};