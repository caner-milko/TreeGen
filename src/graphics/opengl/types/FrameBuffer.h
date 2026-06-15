#pragma once
#include "Common.h"
#include "Texture.h"
#include <unordered_map>
#include <memory>
namespace tgen::graphics::gl {
using FrameBufferHandle = uint32;

class FrameBuffer {
public:
	FrameBuffer() = default;
	bool init(bool reinit = true) {
		if (handle != 0 && reinit) {
			this->~FrameBuffer();
		}
		assert(handle == 0);
		glCreateFramebuffers(1, &handle);
		return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
	}
	FrameBuffer(FrameBuffer&& other) noexcept : handle(std::exchange(other.handle, 0)) {}
	~FrameBuffer() {
		destroy();
	}
	FrameBufferHandle getHandle() {
		return handle;
	}
	MOVE_OPERATOR(FrameBuffer);
	DELETE_COPY_CONSTRUCTORS(FrameBuffer);
	GL_OPERATOR_BOOL(handle);

	void attach(GLenum attachment, const Texture& tex, uint32 mipLevel = 0);
private:
	void destroy() { glDeleteFramebuffers(1, &handle); }
	FrameBufferHandle handle = 0;
};
};