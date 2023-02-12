#pragma once
#include "Texture.h"
#include <array>
namespace tgen::graphics::gl {
class CubemapTexture : public Texture {
public:
	CubemapTexture() = default;
	bool init(TextureCreateData data = {}, bool reinit = true);
	CubemapTexture(CubemapTexture&& other) noexcept : Texture(std::move(other)) {}
	MOVE_OPERATOR(CubemapTexture)
	DELETE_COPY_CONSTRUCTORS(CubemapTexture);
	GL_OPERATOR_BOOL(getHandle());

	void uploadFaces(const std::array<const void*, 6> faces, TextureUploadData uploadData = {});
};
}