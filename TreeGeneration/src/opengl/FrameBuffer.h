#pragma once
#include "../Definitions.h"
#include "./Texture.h"
#include <unordered_map>
#include <memory>
using FrameBufferHandle = uint32;



class FrameBuffer {
public:
	FrameBuffer();
	~FrameBuffer();
	FrameBufferHandle getHandle();

	void attachColor(GLenum attachment, const sp<Texture>& tex, uint32 mipLevel = 0);
private:
	std::unordered_map<GLint, sp<Texture>> attachments;
	FrameBufferHandle handle;
};