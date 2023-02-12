#include "Image.h"

#include <stb_image.h>
namespace tgen::graphics {
Image::Image(uint8* data, int32 width, int32 height, int32 nrChannels) : data(data), width(width), height(height), nrChannels(nrChannels)
{
}

Image::~Image()
{
	stbi_image_free(data);
}

bool Image::isEmpty() const
{

	return !data;
}
}