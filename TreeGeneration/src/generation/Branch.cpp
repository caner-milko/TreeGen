#include "Branch.h"
#include "util/Util.h"
namespace tgen::gen
{
Branch::Branch(rb<const TreeNode> nodePtr, float baseRadius, float radiusPow, float curviness, float startLength, const vec3& lastPlaneNormal, float lastOffset) : from(nodePtr), startLength(startLength), boundingBox(vec3(0.0f), vec3(0.0f))
{
	const TreeNode& node = *nodePtr;
	bez.lowRadius = glm::pow(node.childCount, 1.0f / radiusPow) * baseRadius;

	TreeNode* dominantChild = node.dominantChild();
	length = node.length;
	if (node.order > 0 && node.parent->dominantChild()->id != node.id)
	{
		length += node.parent->length;
		bez.A = node.parent->startPos;
		bez.B = node.startPos + node.direction * node.length * 0.5f;
	}
	else
	{

		bez.A = node.startPos;
		vec3 parentPoint(0.0);

		if (node.order == 0)
		{
			parentPoint = node.startPos + node.direction * node.length * 0.5f;
		}
		else
		{
			parentPoint = node.startPos + node.parent->direction * node.length * 0.5f;
		}

		vec3 childPoint(0.0);
		if (dominantChild != nullptr)
		{
			childPoint = node.endPos() - dominantChild->direction * node.length * 0.5f;
		}
		else
		{
			childPoint = node.startPos + node.direction * node.length * 0.5f;
		}

		bez.B = (parentPoint + childPoint) / 2.0f;
	}
	bez.C = node.endPos();
	bez.B = glm::mix((bez.A + bez.C) / 2.0f, bez.B, curviness);

	float dots = glm::dot(glm::normalize(bez.B - bez.A), glm::normalize(bez.C - bez.A));

	if (glm::abs(dots) >= 0.9999f)
	{
		vec3 randomDir(1.0f, 0.0f, 0.0f);
		if (glm::abs(glm::dot(randomDir, glm::normalize(bez.C - bez.A)) >= 0.99))
			randomDir = vec3(0.0f, 1.0f, 0.0f);
		bez.B += glm::normalize(glm::cross(randomDir, bez.C - bez.A)) * 0.00005f;
	}

	bez.bezierPlaneNormal = normalize(glm::cross(bez.B - bez.A, bez.C - bez.A));

	vec3 bezDir = bez.evaluateDir(0.0f);

	vec3 bezierNormalOnPlane = glm::normalize(glm::cross(bez.bezierPlaneNormal, bezDir));

	float angle = glm::atan(glm::dot(bezierNormalOnPlane, lastPlaneNormal), glm::dot(bez.bezierPlaneNormal, lastPlaneNormal));

	offset = lastOffset + angle, PI * 2.0f;

	bez.highRadius = dominantChild == nullptr ? 0.0f : (glm::pow(dominantChild->childCount, 1.0f / radiusPow) * baseRadius);
	bez.lowRadius = glm::max(bez.lowRadius, 0.0001f);
	bez.highRadius = glm::max(bez.highRadius, 0.0001f);


	// extremes
	vec3 mi = glm::min(bez.A, bez.C);
	vec3 ma = glm::max(bez.A, bez.C);


	mi -= bez.lowRadius;
	ma += bez.lowRadius;

	// maxima/minima point, if p1 is outside the current bbox/hull
	if (bez.B.x<mi.x || bez.B.x>ma.x || bez.B.y<mi.y || bez.B.y>ma.y || bez.B.z < mi.x || bez.B.z > mi.y)
	{

		vec3 t = glm::clamp((bez.A - bez.B) / (bez.A - 2.0f * bez.B + bez.C), 0.0f, 1.0f);
		vec3 s = 1.0f - t;
		vec3 q = s * s * bez.A + 2.0f * s * t * bez.B + t * t * bez.C;

		mi = min(mi, q - bez.lowRadius);
		ma = max(ma, q + bez.lowRadius);
	}


	boundingBox = { mi, ma };

	model = boundingBox.asModel();

	order = node.order;
}

void Branch::updateBranch(float baseRadius, float radiusPow)
{
	TreeNode* dominantChild = from->dominantChild();
	bez.lowRadius = glm::pow(from->childCount, 1.0f / radiusPow) * baseRadius;

	bez.highRadius = dominantChild == nullptr ? 0.0f
		: (glm::pow(dominantChild->childCount, 1.0f / radiusPow) * baseRadius);

	bez.lowRadius = glm::max(bez.lowRadius, 0.0001f);
	bez.highRadius = glm::max(bez.highRadius, 0.0001f);

	// extremes
	vec3 mi = glm::min(bez.A, bez.C);
	vec3 ma = glm::max(bez.A, bez.C);


	mi -= bez.lowRadius;
	ma += bez.lowRadius;

	// maxima/minima point, if p1 is outside the current bbox/hull
	if (bez.B.x<mi.x || bez.B.x>ma.x || bez.B.y<mi.y ||
		bez.B.y>ma.y || bez.B.z < mi.x || bez.B.z > mi.y)
	{

		vec3 t = glm::clamp((bez.A - bez.B) / (bez.A - 2.0f * bez.B + bez.C), 0.0f, 1.0f);
		vec3 s = 1.0f - t;
		vec3 q = s * s * bez.A + 2.0f * s * t * bez.B + t * t * bez.C;

		mi = min(mi, q - bez.lowRadius);
		ma = max(ma, q + bez.lowRadius);
	}


	boundingBox = { mi, ma };

	model = boundingBox.asModel();
}

void Branch::generateLeaves(uint32 maxChildCount, uint32 minOrder, float leafDensity, float sizeMultiplier, bool forceRegen)
{
	if (minOrder > from->order)
	{
		leaves.clear();
		return;
	}
	TreeNode* dominantChild = from->dominantChild();
	uint32 domChildCount = dominantChild == nullptr ? 0 : dominantChild->childCount;
	if (dominantChild == nullptr)
	{
		dominantChild = nullptr;
	}
	if (domChildCount >= maxChildCount)
	{
		leaves.clear();
		return;
	}
	if (!forceRegen && !leaves.empty())
		return;
	leaves.clear();
	float chance = length * leafDensity;

	uint32 id = from->id;
	uint32 hashed = util::hash(id);
	for (int i = 0; i < glm::max(leafDensity, 5.0f); i++)
	{
		uint32 hashedI = util::hash(i);
		if ((util::IntNoise2D(hashed, hashedI) * 0.5f + 0.5f) < chance)
		{
			chance--;
		}
		else
		{
			break;
		}
		float t = util::IntNoise2D(id, i) * 0.5f + 0.5f;

		float rndAngle = PI * util::IntNoise2D(hashed, i);

		float size = sizeMultiplier * (util::IntNoise2D(id, hashedI) * 0.3f + 0.7f);

		if (t > 0.9)
		{
			size *= 1.0f;
		}

		leaves.emplace_back(this, t, size, rndAngle);
	}
}

BranchShaderData Branch::asShaderData(const vec3& color) const
{
	return BranchShaderData{
		.model = model,
		.A = vec4(bez.A, 0.0f),
		.B = vec4(bez.B, 0.0f),
		.C = vec4(bez.C, 0.0f),
		.color = vec4(color, 0.0f),
		.lowRadius = bez.lowRadius,
		.highRadius = bez.highRadius,
		.startLength = startLength,
		.length = length,
		.offset = offset,
		.order = order
	};
}

float Bezier::mapT(float t) const
{
	return t * endT;
}

vec3 Bezier::evaluatePos(float t) const
{
	float mappedT = mapT(t);
	float mint = 1.0f - mappedT;
	return mint * mint * A + 2.0f * mint * mappedT * B + mappedT * mappedT * C;
}

vec3 Bezier::evaluateDir(float t) const
{
	float mappedT = mapT(t);
	return glm::normalize(2.0f * (mappedT - 1.0f) * A + 2.0f * (1.0f - 2.0f * mappedT) * B + 2.0f * mappedT * C);
}

vec3 Bezier::evaluateNormal(float t) const
{
	vec3 dir = evaluateDir(t);
	return glm::normalize(glm::cross(bezierPlaneNormal, dir));
}

float Bezier::evaluateWidth(float t) const
{
	return (1.0f - t) * lowRadius + t * highRadius;
}
}