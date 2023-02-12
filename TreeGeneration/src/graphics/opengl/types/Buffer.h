#pragma once
#include "Common.h"
#include "BasicTypes.h"
#include "ApiToEnum.h"
#include <span>
#include <glad/glad.h>
namespace tgen::graphics::gl {
using BufferHandle = uint32;

enum class BufferType {
	VBO, EBO, SSBO, UBO
};
template<enum BufferType E>
class Buffer {
public:
	Buffer() = default;
	bool init(std::span<uint8> data,
				BufferStorageFlags storageFlags = BufferStorageFlag::NONE,
				BufferMapFlags mapFlags = BufferMapFlag::NONE, bool reinit = true) {
		if (handle != 0 && reinit) {
			this->~Buffer();
		}
		assert(handle == 0);
		this->size = data.size();
		glCreateBuffers(1, &handle);
		GLbitfield field = detail::BufferStorageFlagsToGL(storageFlags) | detail::BufferMapFlagsToGL(mapFlags);
		glNamedBufferStorage(handle, data.size(), data.data(), field);
		return true;
	}

	virtual ~Buffer() {
		destroy();
	}
	BufferHandle getHandle() const {
		return handle;
	}
	Buffer(Buffer<E>&& other) noexcept 
		: handle(std::exchange(other.handle, 0)), size(std::exchange(other.size, 0)) {
	}

	DELETE_COPY_CONSTRUCTORS(Buffer);
	MOVE_OPERATOR(Buffer);
	GL_OPERATOR_BOOL(handle);

	size_t getRawSize() const { return size; }

	void bufferData(std::span<uint8> data, size_t offset) {
		assert(handle);
		assert(size == 0 || size >= (data.size() + offset));
		glNamedBufferSubData(handle, offset, data.size(), data.data());
	}
	
private:
	void destroy() { glDeleteBuffers(1, &handle); handle = 0; size = 0; }
	BufferHandle handle = 0;
	size_t size = 0;
};

template<enum BufferType E, typename T>
class TypedBuffer : public Buffer<E> {
public:
	TypedBuffer() = default;
	TypedBuffer(Buffer<E>&& other) : Buffer<E>(std::move(other)) {}
	TypedBuffer(TypedBuffer<E, T>&& other) : Buffer<E>(std::move(other)) {}
	MOVE_OPERATOR(TypedBuffer);
	DELETE_COPY_CONSTRUCTORS(TypedBuffer);

	bool init(BufferStorageFlags storageFlags = BufferStorageFlag::NONE,
		BufferMapFlags mapFlags = BufferMapFlag::NONE, bool reinit = true) {
		return init(std::span<T>((T*)nullptr, 1), storageFlags, mapFlags, reinit);
	}
	
	bool init(size_t size, BufferStorageFlags storageFlags = BufferStorageFlag::NONE,
		BufferMapFlags mapFlags = BufferMapFlag::NONE, bool reinit = true) {
		return init(std::span<T>((T*)nullptr, size), storageFlags, mapFlags, reinit);
	}

	bool init(std::span<T> data,
		BufferStorageFlags storageFlags = BufferStorageFlag::NONE,
		BufferMapFlags mapFlags = BufferMapFlag::NONE, bool reinit = true) {
		return Buffer<E>::init(std::span<uint8>((uint8*)data.data(), data.size() * sizeof(T)),
			storageFlags, mapFlags, reinit);
	}

	void bufferData(std::span<T> data, size_t offset) {
		Buffer<E>::bufferData(std::span<uint8>((uint8*)data.data(), data.size() * sizeof(T)), offset);
	}

	size_t getSize() const { return Buffer<E>::getRawSize() / sizeof(T); }
};


template<typename T>
using VBO = TypedBuffer<BufferType::VBO, T>;

using EBOInt = TypedBuffer<BufferType::EBO, uint32>;
using EBOShort = TypedBuffer<BufferType::EBO, uint16>;
using EBOByte = TypedBuffer<BufferType::EBO, uint8>;

template<typename T>
using SSBO = TypedBuffer<BufferType::SSBO, T>;
template<typename T>
using UBO = TypedBuffer<BufferType::UBO, T>;
}