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
		C = node.endPos();
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



}

void Branch::generateLeaves(float maxWidth, float leafDensity)
{
	leaves.clear();
	if (lowRadius > maxWidth && highRadius > maxWidth)
		return;

	float startT = 0.0f;

	float startWidth = lowRadius;

	if (lowRadius > maxWidth && highRadius < maxWidth) {
		startT = (maxWidth - highRadius) / (lowRadius - highRadius);
		startWidth = maxWidth;
	}

	float len = (1.0f - startT) * length;

	float avgWidthRatio = (startWidth + highRadius) / 2.0f / maxWidth;

	avgWidthRatio = glm::pow(avgWidthRatio, 3.0f);

	//width distribution
	float widthMultiplier = 1.0f / glm::exp(3.0f * (avgWidthRatio - 0.4f) * (avgWidthRatio * 0.4f)) * (1.0f - avgWidthRatio) * 1.4f;

	int leafCount = len * leafDensity * widthMultiplier;

	int id = from.id;
	leaves.reserve(leafCount);
	for (int i = 0; i < leafCount; i++) {
		float t = glm::mix(startT, 1.0f, util::IntNoise2D(id, i) * 0.5f + 0.5f);

		float rndAngle = PI * util::IntNoise2D(util::hash(id), i);

		leaves.emplace_back(*this, t, glm::sqrt(evaluateWidth(t) / maxWidth) * 0.2f, rndAngle);

	}

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
