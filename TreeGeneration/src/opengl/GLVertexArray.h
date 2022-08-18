#pragma once
#include <glad/glad.h>
#include "../Definitions.h"
#include <vector>
using VertexArrayHandle = uint32;
struct GLVertexArray {
public:
	enum BufferType : int {
		ARRAY = GL_ARRAY_BUFFER,
		ELEMENT = GL_ELEMENT_ARRAY_BUFFER
	};

	enum DrawMode : int {
		STATIC = GL_STATIC_DRAW,
		DYNAMIC = GL_DYNAMIC_DRAW,
		STREAM = GL_STREAM_DRAW
	};

	GLVertexArray() = default;
	GLVertexArray(VertexArrayHandle handle);

	void init();
	void attachBuffer(const BufferType bufferType, const uint32 size, const DrawMode mode, const void* data);
	void bind() const;
	void enableAttribute(const uint32 index, const uint32 size, const uint32 offset, const void* data);
	void destroy();
	VertexArrayHandle getHandle() const;


private:
	VertexArrayHandle m_Handle = 0;
};
