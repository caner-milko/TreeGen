#pragma once
#include "GrowthData.h"
#include "TreeNode.h"
#include "Branch.h"
#include "Leaf.h"
#include <unordered_map>
#include <glm/gtc/constants.hpp>

#include "util/Event.h"

#include <string>
#include <functional>
namespace tgen::gen
{
class TreeWorld;
struct Tree
{
public:
	TreeWorld* world;
	GrowthDataId growthDataId;
	uint32 age = 0;
	uint32 id;
	uint32 seed = 0;
	uint32 maxOrder = 0;
	uint32 lastNodeId = 1;
	TreeNode* root;

	uint32 metamerCount = 0;
	uint32 budCount = 0;

	Tree(rb<TreeWorld> world, uint32 id, vec3 position, GrowthDataId growthDataId, uint32 seed);
	Tree(const Tree& from);

	inline TreeGrowthData& getGrowthData();

	void budToMetamer(TreeNode& bud);

	void startGrow();

	float accumulateLight();

	void distributeVigor();

	float lightAtBud(TreeNode& bud);

	void addNewShoots();

	void shedBranchs();
	void removeNode(TreeNode& node);

	std::vector<vec2> spreadSeeds();

	void calculateChildCount();

	void endGrow();

	void printTreeRecursive(TreeNode& node, const std::string& prefix) const;

	//void calculateShadows() const;

	std::vector<TreeNode> AsNodeVector(bool includeBuds) const;

	const std::vector<rb<Branch>>& getBranchs();

	const std::vector<rb<Branch>>& recalculateBranchs(bool clearCache);

	const Branch& getBranchAt(TreeNodeId id) const
	{
		return cachedBranchs.at(id);
	}

	void generateLeaves();

	~Tree();
	struct TreeEventData : public EventData {};
	Event<TreeEventData> OnBeforeGrow{};
	Event<TreeEventData> OnAfterGrow{};
	Event<TreeEventData> OnDestroy{};
	bool operator==(const Tree& other) const;

private:
	std::vector<rb<Branch>> branchs;
	std::unordered_map<TreeNodeId, Branch> cachedBranchs;
	float accumulateLightRecursive(TreeNode& node);
	void distributeVigorRecursive(TreeNode& node);
	void addShootsRecursive(TreeNode& node);
	void shedBranchsRecursive(TreeNode& node);
	uint32 calculateChildCountRecursive(TreeNode& node);
	//void calculateShadowsRecursive(TreeNode& node) const;

	void removeShadows(const TreeNode& node) const;
	void addShadows(TreeNode& node);
};
}