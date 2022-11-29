#include "Branch.h"
#include "Util.hpp"
Branch::Branch(const TreeNode& node, float startLength, const vec3& lastPlaneNormal, float lastOffset) : from(node), startLength(startLength) {


	lowRadius = node.radius;

	TreeNode* dominantChild = node.dominantChild();
	length = node.length;
	if (node.order > 0 && node.parent->dominantChild()->id != node.id) {

		length += node.parent->length;
		A = node.parent->startPos;
		B = node.startPos + node.direction * node.length * 0.5f;
	}
	else {

		A = node.startPos;
		vec3 parentPoint(0.0);

		if (node.order == 0) {
			parentPoint = node.startPos + node.direction * node.length * 0.5f;
		}
		else {
			parentPoint = node.startPos + node.parent->direction * node.length * 0.5f;
		}

		vec3 childPoint(0.0);
		if (dominantChild != nullptr) {
			childPoint = node.endPos() - dominantChild->direction * node.length * 0.5f;
		}
		else {
			childPoint = node.startPos + node.direction * node.length * 0.5f;
		}

		B = (parentPoint + childPoint) / 2.0f;
	}
	C = node.endPos();

	float dots = glm::dot(glm::normalize(B - A), glm::normalize(C - A));

	if (glm::abs(dots) >= 0.9999f) {
		vec3 randomDir(1.0f, 0.0f, 0.0f);
		if (glm::abs(glm::dot(randomDir, glm::normalize(C - A)) >= 0.99))
			randomDir = vec3(0.0f, 1.0f, 0.0f);
		B += glm::normalize(glm::cross(randomDir, C - A)) * 0.0005f;
	}

	bezierPlaneNormal = normalize(glm::cross(B - A, C - A));

	float dot = glm::dot(lastPlaneNormal, bezierPlaneNormal);

	float acos = 0.0f;

	if (abs(dot) < 1.0f) {
		acos = glm::acos(dot);
	}

	offset = lastOffset + acos;

	highRadius = dominantChild == nullptr ? 0.0f : dominantChild->radius;
	lowRadius = glm::max(lowRadius, 0.01f);
	highRadius = glm::max(highRadius, 0.01f);

}