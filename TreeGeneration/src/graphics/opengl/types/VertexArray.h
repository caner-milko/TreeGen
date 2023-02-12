#pragma once
#include <glad/glad.h>
#include "Common.h"
#include <vector>
#include "Buffer.h"
namespace tgen::graphics::gl {
using VertexArrayHandle = uint32;
struct VertexArray {
public:
	VertexArray() = default;

	bool init(bool reinit = true) {
		if (handle != 0 && reinit) {
			this->~VertexArray();
		}
		assert(handle == 0); glCreateVertexArrays(1, &handle); return handle; }
	~VertexArray() { destroy(); };
	VertexArrayHandle getHandle() const { return handle; }
	VertexArray(VertexArray&& other) noexcept : handle(std::exchange(other.handle, 0)) {}
	MOVE_OPERATOR(VertexArray);
	DELETE_COPY_CONSTRUCTORS(VertexArray);
	GL_OPERATOR_BOOL(handle);

	
	void enableAttribute(uint32 location, uint64 binding, Format format, uint64 offset);

	void bindVBO(uint32 index, const Buffer<BufferType::VBO>& buffer, uint64 offset, uint64 stride);
	void bindEBO(const Buffer<BufferType::EBO>& buffer);

private:
	void destroy() {
		glDeleteVertexArrays(1, &handle);
	}
	VertexArrayHandle handle = 0;
};
}