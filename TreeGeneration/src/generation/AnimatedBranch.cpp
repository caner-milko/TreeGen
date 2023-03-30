#include "AnimatedBranch.h"
namespace tgen::gen
{
AnimatedBranch::AnimatedBranch(vec2 animationBounds, const Branch& endBranch,
	const std::optional<Branch>& startBranch) :
	boundingBox(glm::min(endBranch.boundingBox.min, startBranch ? startBranch->boundingBox.min : endBranch.boundingBox.min),
		glm::max(endBranch.boundingBox.max, startBranch ? startBranch->boundingBox.max : endBranch.boundingBox.max)),
	from(endBranch.from), bez(endBranch.bez)
{
	float startLowRadius = 0.0f;
	float endHighRadius = 0.0f;
	if (startBranch)
	{
		BranchShaderData sbsd = startBranch->asShaderData(vec3(0.0));
		startLowRadius = sbsd.lowRadius;
		endHighRadius = sbsd.highRadius;
	}
	BranchShaderData ebsd = endBranch.asShaderData(vec3(0.0));
	shaderData.TBounds = startBranch ? vec2(1.0f) : vec2(0.0f, 1.0f);
	shaderData.animationBounds = animationBounds;
	shaderData.model = ebsd.model;
	shaderData.A = ebsd.A;
	shaderData.B = ebsd.B;
	shaderData.C = ebsd.C;
	shaderData.lowRadiusBounds = vec2(startLowRadius, ebsd.lowRadius);
	shaderData.highRadiusBounds = vec2(endHighRadius, ebsd.highRadius);
	shaderData.startLength = ebsd.startLength;
	shaderData.length = ebsd.length;
	shaderData.offset = ebsd.offset;
	shaderData.order = ebsd.order;
}

AnimatedBranchShaderData AnimatedBranch::asShaderData(const vec3& color) const
{
	AnimatedBranchShaderData absd = shaderData;
	absd.color = vec4(color, 0.0f);
	return absd;
}

float MapBoundsT(vec2 TBounds, vec2 animationTBounds, float animationT)
{
	return glm::mix(TBounds.x, TBounds.y,
		glm::clamp((animationT - animationTBounds.x) /
			glm::max(0.001f, animationTBounds.y - animationTBounds.x), 0.0f, 1.0f));
}

float MapLowRadius(vec2 lowRadiusBounds, vec2 animationTBounds, float animationT)
{
	return glm::mix(lowRadiusBounds.x, lowRadiusBounds.y,
		glm::clamp((animationT - animationTBounds.x) /
			(1.0f - animationTBounds.x), 0.0f, 1.0f));
}

float MapHighRadius(vec2 highRadiusBounds, vec2 animationTBounds, float animationT)
{
	return glm::mix(highRadiusBounds.x, highRadiusBounds.y, glm::clamp((animationT - animationTBounds.y) /
		glm::max(0.001f, 1.0f - animationTBounds.y), 0.0f, 1.0f));
}

Bezier AnimatedBranch::curBezier(float animationT)
{
	Bezier bezier = bez;
	bezier.endT = MapBoundsT(shaderData.TBounds, shaderData.animationBounds, animationT);
	bezier.lowRadius = MapLowRadius(shaderData.lowRadiusBounds, shaderData.animationBounds, animationT);
	bezier.highRadius = MapHighRadius(shaderData.highRadiusBounds, shaderData.animationBounds, animationT);

	return bezier;
}
}