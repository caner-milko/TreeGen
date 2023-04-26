#pragma once
#include "Image.h"
#include "types/Texture.h"
namespace tgen::gen
{
class EditableMap
{
public:
	EditableMap(rc<graphics::Image> image) : image(image)
	{
		initTexture();
		updateImage();
	}

	void initTexture();

	rc<graphics::Image> getImage()
	{
		if (textureEdited)
			updateImage();
		return image;
	}

	void edited()
	{
		textureEdited = true;
	}

	void updateImage();

	rc<graphics::gl::Texture> getTexture() { return texture; }

private:
	bool textureEdited;
	rc<graphics::Image> image;
	rc<graphics::gl::Texture> texture;
};
}