#include "Leaf.h"
#include "Branch.h"
#include "AnimatedBranch.h"
#include "util/util.h"
namespace tgen::gen
{
float Leaf::pertubateAngle = PI / 2.0f;
Leaf::Leaf(rb<Branch> branch, float branchT, float size, float randomAngle)
	: attachedTo(branch), branchT(branchT), randomAngle(randomAngle), size(size)
{
	vec3 dir = branch->bez.evaluateDir(branchT);
	vec3 norm = branch->bez.evaluateNormal(branchT);

	norm = glm::normalize(glm::rotate(glm::angleAxis(randomAngle, dir), norm));

	vec3 pos = calculatePos() + norm * branch->bez.evaluateWidth(branchT);

	norm = util::randomPerturbateVector(norm, pertubateAngle, util::hash(randomAngle / PI * 180.0f));

	dir = glm::normalize(dir - glm::dot(norm, dir) * norm);

	mat4 rotMat = glm::inverse(glm::lookAt(vec3(0.0f), dir, norm));


	model = glm::scale(glm::translate(mat4(1.0f), pos) * rotMat,
		vec3(size));
}

mat4 Leaf::animatedLeaf(rb<AnimatedBranch> animated, float animationT) const
{
	auto bez = animated->curBezier(animationT);
	vec3 dir = bez.evaluateDir(branchT);
	vec3 norm = bez.evaluateNormal(branchT);

	norm = glm::normalize(glm::rotate(glm::angleAxis(randomAngle, dir), norm));

	vec3 pos = bez.evaluatePos(branchT);// + norm * bez.evaluateWidth(branchT);

	norm = util::randomPerturbateVector(norm, pertubateAngle, util::hash(randomAngle / PI * 180.0f));

	dir = glm::normalize(dir - glm::dot(norm, dir) * norm);

	mat4 rotMat = glm::inverse(glm::lookAt(vec3(0.0f), dir, norm));

	float s = size * bez.endT;

	return glm::scale(glm::translate(mat4(1.0f), pos) * rotMat, vec3(s));
}

vec3 Leaf::calculatePos() const
{
	return attachedTo->bez.evaluatePos(branchT);
}
}