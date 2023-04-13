#pragma once
#include "Common.h"
namespace tgen::graphics
{
struct Image
{
	uint8* data;
	int32 width, height, nrChannels;
	Image(uint8* data, int32 width, int32 height, int32 nrChannels);
	DELETE_COPY_CONSTRUCTORS(Image);
	~Image();
	bool isEmpty() const;
	inline ivec2 uvToPixel(vec2 uv) const
	{
		return glm::max(ivec2(0, 0), glm::min(ivec2(uv * vec2(width, height)), ivec2(width - 1, height - 1)));
	}
	template<int N>
	glm::vec<N, uint8> getRGB(vec2 uv) const
	{
		return getRGB<N>(uvToPixel(uv));
	}
	template<int N>
	glm::vec<N, uint8> getRGB(ivec2 pos) const
	{
		size_t address = (pos.x + pos.y * width) * nrChannels;
		glm::vec<N, uint8> vals;
		for (int i = 0; i < N; i++)
		{
			vals[i] = data[address + i];
		}
		return vals;
	}
};
}