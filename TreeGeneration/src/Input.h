#pragma once
#include "./Definitions.h"
class Input {
private:
	Input() {

	}
	~Input() = default;

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

public:

	static auto& GetInstance() {
		static Input instance;
		return instance;
	}

	void Reset() {
		mouseMoved = false;
		scrolled = false;
		scrollOffset = vec2(0.0f);
		mouseOffset = vec2(0.0f);
	}

	void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
		mouseMoved = true;
		mouseOffset = vec2(xpos - mousePos.x, mousePos.y - ypos);
		mousePos = { xpos, ypos };
	}
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		scrolled = true;
		scrollOffset = vec2{ xoffset, yoffset };
	}

	bool didMouseMove() const {
		return mouseMoved;
	}

	vec2 getMousePos() const {
		return mousePos;
	}

	vec2 getMouseOffset() const {
		return mouseOffset;
	}

	bool didScroll() const {
		return mouseMoved;
	}

	vec2 getScrollOffset() const {
		return scrollOffset;
	}



private:
	bool mouseMoved = false;
	vec2 mousePos = vec2(0.0f);
	vec2 mouseOffset = vec2(0.0f);

	bool scrolled = false;
	vec2 scrollOffset = vec2(0.0f);

};