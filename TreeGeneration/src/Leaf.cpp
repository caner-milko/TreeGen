#include "Leaf.h"
#include "Branch.h"
#include "Util.hpp"
float Leaf::pertubateAngle = PI / 4.0f;
Leaf::Leaf(Branch& branch, float branchT, float size, float randomAngle) : attachedTo(branch), branchT(branchT)
{
	vec3 dir = branch.evaluateDir(branchT);
	vec3 norm = branch.evaluateNormal(branchT);

	norm = glm::normalize(glm::rotate(glm::angleAxis(randomAngle, dir), norm));

	vec3 pos = calculatePos() + norm * branch.evaluateWidth(branchT);

	norm = util::randomPerturbateVector(norm, pertubateAngle, util::hash(randomAngle / PI * 180.0f));

	dir = glm::normalize(dir - glm::dot(norm, dir) * norm);

	mat4 rotMat = glm::inverse(glm::lookAt(vec3(0.0f), dir, norm));


	model = glm::scale(glm::translate(mat4(1.0f), pos) * rotMat,
		vec3(size));
}

vec3 Leaf::calculatePos() const {
	return attachedTo.evaluatePos(branchT);
}