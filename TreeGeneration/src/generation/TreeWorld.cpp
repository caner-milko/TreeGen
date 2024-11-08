#include "TreeWorld.h"
#include "util/Util.h"
#include <iostream>
namespace tgen::gen
{
TreeWorld::TreeWorld(TreeWorldGrowthData worldGrowthData, const util::BBox& worldBoundingBox, float cellSize, uint32 seed)
	: TreeWorld(
		TreeWorldInfo{
	.seed = seed,
	.worldSize = ivec3(glm::ceil((worldBoundingBox.max - worldBoundingBox.min) / cellSize)),
	.leftBottomCorner = worldBoundingBox.min,
	.cellSize = cellSize
		}, worldGrowthData)
{
	recalculateLUT();
}

TreeWorld::TreeWorld(TreeWorldInfo info, TreeWorldGrowthData worldGrowthData) : info(info)
{
	SetWorldGrowthData(worldGrowthData);
	resizeShadowGrid();
}

void TreeWorld::SetWorldGrowthData(TreeWorldGrowthData data)
{
	this->worldGrowthData = data;
	recalculateLUT();
}

void TreeWorld::resizeShadowGrid()
{
	this->shadowGrid = std::vector(info.worldSize.x * info.worldSize.y * info.worldSize.z, ShadowCell());
	std::fill(shadowGrid.begin(), shadowGrid.end(), ShadowCell{ 0.0f });
	std::cout << "World Size: " << shadowGrid.size() << " Bytes: " << shadowGrid.size() * sizeof(ShadowCell) << std::endl;
}

void TreeWorld::recalculateLUT()
{
	pyramidShadowLUT = std::vector<float>(worldGrowthData.pyramidHeight + 1, 0);
	for (int q = 0; q <= worldGrowthData.pyramidHeight; q++)
	{
		pyramidShadowLUT[q] = worldGrowthData.a * glm::pow(worldGrowthData.b, -(q / 2.0f));
	}
}

Tree& TreeWorld::createTree(vec2 position, GrowthDataId growthDataId)
{
	treeCount++;
	float height = terrain->heightAtWorldPos(vec3(position.x, 0, position.y));
	vec3 terPos = vec3(position.x, height, position.y);
	auto& tree = *trees.emplace_back(std::make_unique<Tree>(this, trees.size(), terPos, growthDataId, trees.size()));

	onTreeCreated.dispatch({ .newTree = tree });
	return tree;
}

void TreeWorld::removeTree(uint32 treeId)
{
	for (int i = 0; i < trees.size(); i++)
	{
		if (trees[i]->id == treeId)
		{
			onTreeDestroyed.dispatch({ .newTree = *trees[i] });
			trees.erase(trees.begin() + i);
			return;
		}
	}
}

float TreeWorld::getLightAt(const vec3& position)
{
	ivec3 relScaledPos = coordinateToCell(position);
	if (relScaledPos.x < 0 || relScaledPos.x >= info.worldSize.x || relScaledPos.y < 0 || relScaledPos.y >= info.worldSize.y || relScaledPos.z < 0 || relScaledPos.z >= info.worldSize.z)
		return 0.0f;
	return glm::max(worldGrowthData.fullExposure - getCellAt(relScaledPos).shadow + worldGrowthData.a, 0.0f);
}

vec3 TreeWorld::getOptimalDirection(const vec3& position)
{
	if (isOutOfBounds(position))
	{
		return vec3(0.0f, 1.0f, 0.0f);
	}
	ivec3 relScaledPos = coordinateToCell(position);

	ivec3 negX = relScaledPos - ivec3(1, 0, 0);
	ivec3 posX = relScaledPos + ivec3(1, 0, 0);

	if (negX.x < 0)
		negX = relScaledPos;

	if (posX.x >= info.worldSize.x)
		posX = relScaledPos;

	float xGrad = (getCellAt(posX).shadow - getCellAt(negX).shadow) / static_cast<float>(posX.x - negX.x);


	ivec3 negY = relScaledPos - ivec3(0, 1, 0);
	ivec3 posY = relScaledPos + ivec3(0, 1, 0);


	if (negY.y < 0)
		negY = relScaledPos;

	if (posY.y >= info.worldSize.y)
		posY = relScaledPos;

	float yGrad = (getCellAt(posY).shadow - getCellAt(negY).shadow) / static_cast<float>(posY.y - negY.y);

	ivec3 negZ = relScaledPos - ivec3(0, 0, 1);
	ivec3 posZ = relScaledPos + ivec3(0, 0, 1);


	if (negZ.z < 0)
		negZ = relScaledPos;

	if (posZ.z >= info.worldSize.z)
		posZ = relScaledPos;

	float zGrad = (getCellAt(posZ).shadow - getCellAt(negZ).shadow) / static_cast<float>(posZ.z - negZ.z);

	vec3 gradient = vec3(xGrad, yGrad, zGrad);
	if (glm::length(gradient) == 0.0f)
		return vec3(0.0f, 1.0f, 0.0f);

	return -glm::normalize(vec3(xGrad, yGrad, zGrad));

	/*float cos = glm::cos(perceptionAngle);
	float lowest = std::numeric_limits<float>().max();
	vec3 selDir = direction;
	int sameLowest = 0;
	ivec3 relScaledPos = coordinateToCell(position);
	float scaledPerception = perceptionRadius / cellSize;
	int perceptionFloor = static_cast<int>(glm::floor(scaledPerception));
	int perceptionCeil = static_cast<int>(glm::ceil(scaledPerception));
	for (int i = glm::max(relScaledPos.x - perceptionFloor, 0); i <= glm::min(relScaledPos.x + perceptionCeil, worldSize.x - 1); i++) {
		for (int j = glm::max(relScaledPos.y - perceptionFloor, 0); j <= glm::min(relScaledPos.y + perceptionCeil, worldSize.y - 1); j++) {
			for (int k = glm::max(relScaledPos.z - perceptionFloor, 0); k <= glm::min(relScaledPos.z + perceptionCeil, worldSize.z - 1); k++) {

				ivec3 cellPos = ivec3(i, j, k);
				vec3 cellCoord = cellToCoordinate(cellPos);
				vec3 dif = cellCoord - position;
				vec3 normalizedDif = glm::normalize(dif);
				float dot = glm::dot(direction, normalizedDif);
				if (dot < cos)
					continue;
				if (glm::length(dif) > perceptionRadius)
					continue;
				ShadowCell& cell = getCellAt(cellPos);
				float shadowAt = cell.shadow;
				if (shadowAt < lowest) {
					lowest = shadowAt;
					selDir = normalizedDif;
					sameLowest = 1;
				}
				if (shadowAt == lowest) {
					sameLowest++;
					selDir += normalizedDif;
				}
			}
		}
	}
	return selDir / (float)sameLowest;*/

}

void TreeWorld::castShadows(const vec3& pos, bool addShadows)
{
	ivec3 relScaledPos = coordinateToCell(pos);
	for (int q = 0; q <= worldGrowthData.pyramidHeight; q++)
	{
		int j = relScaledPos.y - q;
		if (j < 0)
			break;
		if (j >= info.worldSize.y)
			continue;
		for (int i = glm::max(relScaledPos.x - q, 0); i <= glm::min(relScaledPos.x + q, info.worldSize.x - 1); i++)
		{
			for (int k = glm::max(relScaledPos.z - q, 0); k <= glm::min(relScaledPos.z + q, info.worldSize.z - 1); k++)
			{
				//LUT for pow
				float s = pyramidShadowLUT[q];
				addShadowTo(ivec3(i, j, k), s * (float(addShadows) * 2.0f - 1.0f));
			}
		}
	}
}

ivec3 TreeWorld::coordinateToCell(const vec3& pos) const
{
	vec3 relPos = (pos - info.leftBottomCorner) / info.cellSize;
	int x = static_cast<int>(glm::round(relPos.x));
	int y = static_cast<int>(glm::round(relPos.y));
	int z = static_cast<int>(glm::round(relPos.z));
	return ivec3(x, y, z);
}

vec3 TreeWorld::cellToCoordinate(const ivec3& cell) const
{
	return vec3(cell) * info.cellSize + info.leftBottomCorner;
}

int TreeWorld::cellToIndex(const ivec3& cell) const
{
	return  info.worldSize.x * info.worldSize.y * cell.z + info.worldSize.x * cell.y + cell.x;
}

std::vector<vec4> TreeWorld::renderShadowCells(const vec3& camPos, const vec3& viewDir, float fov, float visibilityRadius) const
{
	ivec3 camToCell = coordinateToCell(camPos);

	float visibilityScaled = visibilityRadius / info.cellSize;

	int visibilityScaledFloored = static_cast<int>(glm::floor(visibilityScaled));
	int visibilityScaledCeiled = static_cast<int>(glm::ceil(visibilityScaled));

	float fovCos = glm::cos(fov);
	std::vector<vec4> cells;
	for (int i = glm::max(camToCell.x - visibilityScaledFloored, 0); i <= glm::min(camToCell.x + visibilityScaledCeiled, info.worldSize.x - 1); i++)
	{
		for (int j = glm::max(camToCell.y - visibilityScaledFloored, 0); j <= glm::min(camToCell.y + visibilityScaledCeiled, info.worldSize.y - 1); j++)
		{
			for (int k = glm::max(camToCell.z - visibilityScaledFloored, 0); k <= glm::min(camToCell.z + visibilityScaledCeiled, info.worldSize.z - 1); k++)
			{

				ivec3 cellPos = ivec3(i, j, k);
				vec3 cellCoord = cellToCoordinate(cellPos);
				vec3 dif = cellCoord - camPos;
				vec3 normalizedDif = glm::normalize(dif);
				float dot = glm::dot(viewDir, normalizedDif);
				if (dot < fovCos)
					continue;
				if (glm::length(dif) > visibilityRadius)
					continue;
				const ShadowCell& cell = getCellAt(cellPos);
				float shadowAt = cell.shadow;
				if (shadowAt == 0.0f)
				{
					continue;
				}
				cells.emplace_back(cellCoord, shadowAt);
			}
		}
	}
	return cells;
}


ShadowCell& TreeWorld::getCellAt(const ivec3& cell)
{
	return shadowGrid[cellToIndex(cell)];
}

const ShadowCell& TreeWorld::getCellAt(const ivec3& cell) const
{
	return shadowGrid[cellToIndex(cell)];
}

void TreeWorld::addShadowTo(const ivec3& cell, float amount)
{
	getCellAt(cell).shadow += amount;
}

GrowthDataId TreeWorld::getGrowthDataFromMap(vec2 worldPos)
{
	vec2 uv = (worldPos - vec2(info.leftBottomCorner.x, info.leftBottomCorner.z))
		/ (info.cellSize * vec2(info.worldSize.x, info.worldSize.z));
	auto pix = presetMap->getImage()->uvToPixel(uv);
	auto col = presetMap->getImage()->getRGB<3>(pix);
	auto it = worldGrowthData.colorToPresetMap.find(col);
	assert(it != worldGrowthData.colorToPresetMap.end());
	return it->second;
}

void TreeWorld::setPresetMap(rc<graphics::Image> image)
{
	presetMap = std::make_shared<EditableMap>(image);
}

std::pair<GrowthDataId, glm::vec<3, uint8>> TreeWorld::newGrowthData(TreeGrowthData data, std::optional<glm::vec<3, uint8>> col)
{
	auto id = worldGrowthData.presets.emplace((GrowthDataId)worldGrowthData.presets.size(), data).first->first;
	auto size = worldGrowthData.presets.size();
	if (!col)
	{
		uint8 colR = util::IntNoise2D(size, 0, 0, 1) * 255;
		uint8 colG = util::IntNoise2D(size, util::hash(size), 0, 1) * 255;
		uint8 colB = util::IntNoise2D(size, size * 2, 0, 1) * 255;
		col = glm::vec<3, uint8>(colR, colG, colB);
	}
	worldGrowthData.colorToPresetMap[*col] = id;
	return { id, *col };
}
}
