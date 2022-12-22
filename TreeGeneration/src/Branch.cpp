#include "Branch.h"
#include "Util.hpp"
Branch::Branch(const TreeNode& node, float baseRadius, float radiusPow, float curviness, float startLength, const vec3& lastPlaneNormal, float lastOffset) : from(node), startLength(startLength), boundingBox(vec3(0.0f), vec3(0.0f)) {
	lowRadius = glm::pow(node.childCount, 1.0f / radiusPow) * baseRadius;

	TreeNode* dominantChild = node.dominantChild();
	length = node.length;
	if (false && node.order > 0 && node.parent->dominantChild()->id != node.id) {

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
	A = node.startPos;
	C = node.endPos();
	B = glm::mix((A + C) / 2.0f, B, curviness);

	float dots = glm::dot(glm::normalize(B - A), glm::normalize(C - A));

	if (glm::abs(dots) >= 0.9999f) {
		vec3 randomDir(1.0f, 0.0f, 0.0f);
		if (glm::abs(glm::dot(randomDir, glm::normalize(C - A)) >= 0.99))
			randomDir = vec3(0.0f, 1.0f, 0.0f);
		B += glm::normalize(glm::cross(randomDir, C - A)) * 0.00005f;
	}

	bezierPlaneNormal = normalize(glm::cross(B - A, C - A));

	vec3 bezDir = evaluateDir(0.0f);

	vec3 bezierNormalOnPlane = glm::normalize(glm::cross(bezierPlaneNormal, bezDir));

	float angle = glm::atan(glm::dot(bezierNormalOnPlane, lastPlaneNormal), glm::dot(bezierPlaneNormal, lastPlaneNormal));

	offset = lastOffset + angle;

	highRadius = dominantChild == nullptr ? 0.0f : (glm::pow(dominantChild->childCount, 1.0f / radiusPow) * baseRadius);
	lowRadius = glm::max(lowRadius, 0.0001f);
	highRadius = glm::max(highRadius, 0.0001f);


	// extremes
	vec3 mi = glm::min(A, C);
	vec3 ma = glm::max(A, C);


	mi -= lowRadius;
	ma += lowRadius;

	// maxima/minima point, if p1 is outside the current bbox/hull
	if (B.x<mi.x || B.x>ma.x || B.y<mi.y || B.y>ma.y || B.z < mi.x || B.z > mi.y)
	{

		vec3 t = glm::clamp((A - B) / (A - 2.0f * B + C), 0.0f, 1.0f);
		vec3 s = 1.0f - t;
		vec3 q = s * s * A + 2.0f * s * t * B + t * t * C;

		mi = min(mi, q - lowRadius);
		ma = max(ma, q + lowRadius);
	}



	boundingBox = { mi, ma };

	model = boundingBox.asModel();

}

void Branch::generateLeaves(uint32 maxChildCount, uint32 minOrder, float leafDensity, float sizeMultiplier)
{
	leaves.clear();

	if (minOrder > from.order)
		return;
	TreeNode* dominantChild = from.dominantChild();
	uint32 domChildCount = dominantChild == nullptr ? 0 : dominantChild->childCount;
	if (dominantChild == nullptr) {
		dominantChild = nullptr;
	}
	if (domChildCount >= maxChildCount)
		return;
	float startT = 0.0f;

	if (from.childCount > maxChildCount) {
		startT = float(from.childCount - maxChildCount) / float(from.childCount - domChildCount);
	}

	float chance = length * leafDensity;

	int id = from.id;
	uint32 hashed = util::hash(id);
	for (int i = 0; i < glm::max(leafDensity, 5.0f); i++) {
		uint32 hashedI = util::hash(i);
		if ((util::IntNoise2D(hashed, hashedI) * 0.5f + 0.5f) < chance) {
			chance--;
		}
		else {
			break;
		}
		float t = util::IntNoise2D(id, i) * 0.5f + 0.5f;

		float rndAngle = PI * util::IntNoise2D(hashed, i);

		float size = sizeMultiplier * (util::IntNoise2D(id, hashedI) * 0.3f + 0.7f);

		if (t > 0.9) {
			size *= 1.0f;
		}

		leaves.emplace_back(*this, t, size, rndAngle);



	}

}

BranchShaderData Branch::asShaderData(const vec3& color) const
{
	return BranchShaderData{
		.model = model,
		.A = vec4(A, 0.0f),
		.B = vec4(B, 0.0f),
		.C = vec4(C, 0.0f),
		.color = vec4(color, 0.0f),
		.lowRadius = lowRadius,
		.highRadius = highRadius,
		.startLength = startLength,
		.length = length,
		.offset = vec4(offset)
	};
}

vec3 Branch::evaluatePos(float t) const
{
	float mint = 1.0f - t;
	return mint * mint * A + 2.0f * mint * t * B + t * t * C;
}

vec3 Branch::evaluateDir(float t) const
{
	return glm::normalize(2.0f * (t - 1.0f) * A + 2.0f * (1.0f - 2.0f * t) * B + 2.0f * t * C);
}

vec3 Branch::evaluateNormal(float t) const
{
	vec3 dir = evaluateDir(t);
	return glm::normalize(glm::cross(bezierPlaneNormal, dir));
}

float Branch::evaluateWidth(float t) const
{
	return (1.0f - t) * lowRadius + t * highRadius;
}
