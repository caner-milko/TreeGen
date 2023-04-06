#include "Leaf.h"
#include "Branch.h"
#include "AnimatedBranch.h"
#include "util/util.h"
#include "TreeNode.h"

namespace tgen::gen
{
float Leaf::pertubateAngle = PI / 2.0f;

inline mat4 calcLeafModel(const Bezier& bez, float randomAngle,
	float branchT, float pertubateAngle, float size)
{
	vec3 dir = bez.evaluateDir(branchT);
	vec3 norm = bez.evaluateNormal(branchT);

	norm = glm::normalize(glm::rotate(glm::angleAxis(randomAngle, dir), norm));

	vec3 pos = bez.evaluatePos(branchT) + norm * bez.evaluateWidth(branchT);

	norm = util::randomPerturbateVector(norm, pertubateAngle, util::hash(randomAngle / PI * 180.0f));

	dir = glm::normalize(dir - glm::dot(norm, dir) * norm);

	mat4 rotMat = glm::inverse(glm::lookAt(vec3(0.0f), dir, norm));

	return glm::scale(glm::translate(mat4(1.0f), pos) * rotMat, vec3(size));
}

Leaf::Leaf(rb<Branch> branch, float branchT, float size, float randomAngle)
	: attachedTo(branch), branchT(branchT), randomAngle(randomAngle), size(size)
{
	model = calcLeafModel(branch->bez, randomAngle, branchT, pertubateAngle, size);
}

constexpr uint32 leafAgeAnimTime = 1;
constexpr float leafAnimOffset = 0.75f;

mat4 Leaf::animatedLeaf(rb<AnimatedBranch> animated, uint32 treeAge, float animationT)
{
	auto bez = animated->curBezier(animationT);
	float treeAgeInterpolated = treeAge - 1 + animationT;
	int createdAt = attachedTo->from->createdAt - 1;
	float dif = treeAgeInterpolated - createdAt - leafAnimOffset;
	if (dif <= leafAgeAnimTime)
	{
		dif = dif * 1.0f / leafAgeAnimTime;
		float s = glm::clamp(size * dif, 0.0f, size);
		model = calcLeafModel(bez, randomAngle, branchT, pertubateAngle, s);
		return model;
	}
	return model;
}
}