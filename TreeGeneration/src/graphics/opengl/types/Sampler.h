#pragma once
#include "Common.h"
#include <glad/glad.h>
namespace tgen::graphics::gl {
using SamplerHandle = uint32;
class Sampler {
public:
	Sampler() = default;
	void init(bool reinit = true) {
		if (handle != 0 && reinit) {
			this->~Sampler();
		}
		assert(handle == 0);
		glCreateSamplers(1, &handle);
	}
	Sampler(Sampler&& other) noexcept : handle(std::exchange(handle, 0)) {}
	~Sampler() {
		destroy();
	}
	DELETE_COPY_CONSTRUCTORS(Sampler);
	MOVE_OPERATOR(Sampler);
	GL_OPERATOR_BOOL(handle);

	SamplerHandle getHandle() const { return handle; }
private:
	void destroy() { glDeleteSamplers(1, &handle); }
	SamplerHandle handle;
};
}