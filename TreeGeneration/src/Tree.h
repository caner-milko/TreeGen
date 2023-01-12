#pragma once
#include "TreeNode.h"
#include "Branch.h"
#include "Leaf.h"
#include <unordered_map>
#include <glm/gtc/constants.hpp>

#include <string>
struct TreeWorld;

struct TreeGrowthData
{
	float apicalControl = 0.5f;
	float vigorMultiplier = 2.0f;
	float baseLength = .02f;

	float baseRadius = 0.0005f;
	float radiusN = 2.5f;
	float branchCurviness = 0.5f;
	//lateral
	float lateralAngle = glm::radians(45.0f);
	//
	vec3 tropism = vec3(0.0f, 1.0f, 0.0f);
	//default, optimal, tropism
	vec2 directionWeights = vec2(0.1f, 0.1f);
	//shadows
	float fullExposure = 2.5f;

	int32 pyramidHeight = 6;
	float a = 0.8f;
	//b > 1
	float b = 1.5f;

	//shedding
	bool shouldShed = true;
	float shedMultiplier = 0.3f;
	float shedExp = 1.5f;

	//leaf params
	int32 leafMaxChildCount = 5;
	int32 leafMinOrder = 4;
	float leafDensity = 20.0f;
	float leafSizeMultiplier = 0.2f;

};

struct Tree
{
public:
	TreeWorld& world;
	TreeGrowthData growthData{};
	uint32 age = 0;
	uint32 id;
	uint32 seed = 0;
	uint32 maxOrder = 0;
	uint32 lastNodeId = 1;
	TreeNode* root;

	uint32 metamerCount = 0;
	uint32 budCount = 0;

	Tree(TreeWorld& world, uint32 id, vec3 position, TreeGrowthData growthData, uint32 seed);
	Tree(const Tree& from);

	void budToMetamer(TreeNode& bud);

	void startGrow();

	float accumulateLight();

	void distributeVigor();

	float lightAtBud(TreeNode& bud);

	void addNewShoots();

	void shedBranchs();

	void calculateChildCount();

	void endGrow();

	void printTreeRecursive(TreeNode& node, const std::string& prefix) const;

	//void calculateShadows() const;

	std::vector<TreeNode> AsNodeVector(bool includeBuds) const;



	const std::vector<Branch>& getBranchs();

	const std::vector<Branch>& recalculateBranchs();

	void generateLeaves();

	~Tree();

	bool operator==(const Tree& other) const;

private:

	std::vector<Branch> branchs;
	float accumulateLightRecursive(TreeNode& node);
	void distributeVigorRecursive(TreeNode& node);
	void addShootsRecursive(TreeNode& node);
	void shedBranchsRecursive(TreeNode& node);
	float calculateChildCountRecursive(TreeNode& node);
	//void calculateShadowsRecursive(TreeNode& node) const;

	void removeShadows(const TreeNode& node) const;
	void addShadows(TreeNode& node);
};
