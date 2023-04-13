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

	}

	rc<graphics::Image> getImage()
	{
		if (edited)
			return image;
	}

	void Edited()
	{
		edited = true;
	}

	void UpdateTexture();

private:
	bool edited;
	rc<graphics::Image> image;
	graphics::gl::Texture texture;
};
}