#pragma once
#include "Common.h"
namespace tgen::gen {
struct Branch;

struct Leaf {
	mat4 model;
	Branch& attachedTo;
	float branchT;
	static float pertubateAngle;

	Leaf(Branch& branch, float branchT, float size, float randomAngle);


	vec3 calculatePos() inline const;

};
}