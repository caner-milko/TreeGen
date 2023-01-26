#pragma once
#include "Texture.h"
#include <array>
class CubemapTexture : public Texture {
public:
	CubemapTexture(TextureCreateData data);
	DELETE_COPY_CONSTRUCTORS(CubemapTexture);
	~CubemapTexture() override = default;
	void uploadFaces(const std::array<const void*, 6> faces, TextureUploadData uploadData = {});
};