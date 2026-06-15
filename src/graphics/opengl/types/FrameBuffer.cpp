#include "FrameBuffer.h"
namespace tgen::graphics::gl {

void FrameBuffer::attach(GLenum attachment, const Texture& tex, uint32 mipLevel)
{
	assert(handle);
	glNamedFramebufferTexture(handle, attachment, tex.getHandle(), mipLevel);

}
}