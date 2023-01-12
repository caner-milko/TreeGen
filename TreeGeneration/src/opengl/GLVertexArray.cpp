#include "GLVertexArray.h"

GLVertexArray::GLVertexArray(VertexArrayHandle handle) : m_Handle(handle)
{
}

void GLVertexArray::init() {
	glGenVertexArrays(1, &m_Handle);
}

void GLVertexArray::attachBuffer(const BufferType bufferType, const uint32 size, const DrawMode mode, const void* data) {
	GLuint buffer;
	glGenBuffers(1, &buffer);

	glBindBuffer(bufferType, buffer);
	glBufferData(bufferType, size, data, mode);
}

void GLVertexArray::bind() const {
	glBindVertexArray(m_Handle);
}

void GLVertexArray::destroy() {
	glDeleteVertexArrays(1, &m_Handle);
}

VertexArrayHandle GLVertexArray::getHandle() const
{
	return m_Handle;
}

void GLVertexArray::enableAttribute(const uint32 index, const uint32 size, const uint32 offset, const void* data) {
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, offset, data);
	glEnableVertexAttribArray(index);
}