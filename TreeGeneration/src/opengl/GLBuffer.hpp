#pragma once
#include "../Definitions.h"
#include <glad/glad.h>
using BufferHandle = uint32;

enum BufferType : uint32 {
	SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER,
};
template<BufferType T>
class GLBuffer {
public:

	GLBuffer() {
	}
	GLBuffer(BufferHandle handle) : handle(handle) {

	}

	void init() {
		glCreateBuffers(1, &handle);
	}
	BufferHandle getHandle() const {
		return handle;
	}
	void bufferStorage(size_t size, const void* data, uint32 flags) {
		glNamedBufferStorage(handle, size, data, flags);
	}
	void bufferData(size_t size, const void* data, uint32 usage) {
		glNamedBufferData(handle, size, data, usage);
	}
	void bindBase(uint32 index, BufferType type = T) {
		glBindBufferBase(type, index, handle);
	}
	void destroy() {
		glDeleteBuffers(1, &handle);
	}
private:
	BufferHandle handle = 0;
};

using SSBO = GLBuffer<SHADER_STORAGE_BUFFER>;