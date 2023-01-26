#include "FrameBuffer.h"

FrameBuffer::FrameBuffer()
{
	glCreateFramebuffers(1, &handle);
}

FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &handle);
}

FrameBufferHandle FrameBuffer::getHandle()
{
	return handle;
}

void FrameBuffer::attachColor(GLenum attachment, const sp<Texture>& tex, uint32 mipLevel)
{
	glNamedFramebufferTexture(handle, attachment, tex->getHandle(), mipLevel);

}
