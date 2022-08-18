#include "TreeWorld.h"

TreeWorld::TreeWorld(int xSize, int ySize, int zSize, vec3 leftBottomCorner, float cellSize) : xSize(xSize), ySize(ySize), zSize(zSize), leftBottomCorner(leftBottomCorner), cellSize(cellSize), shadowGrid(xSize*ySize*zSize, ShadowCell())
{
}

void TreeWorld::calculateShadows()
{
	for (auto& tree : trees) {
		tree.calculateShadows();
	}
}

Tree & TreeWorld::createTree(vec3 position)
{
	return trees.emplace_back(*this, position);
}

float TreeWorld::getLightAt(const vec3 & position, float a, float fullExposure)
{
	ivec3 relScaledPos = coordinateToCell(position);
	if (relScaledPos.x < 0 || relScaledPos.x >= xSize || relScaledPos.y < 0 || relScaledPos.y >= ySize || relScaledPos.z < 0 || relScaledPos.z >= zSize)
		return 0.0f;
	return glm::max(fullExposure - getCellAt(relScaledPos).shadow + a, 0.0f);
}

vec3 TreeWorld::getOptimalDirection(const vec3 & position, const vec3 & direction, float perceptionRadius, float perceptionAngle)
{
	float cos = glm::cos(perceptionAngle);
	float lowest = std::numeric_limits<float>().max();
	vec3 selDir = direction;
	int sameLowest = 0;
	ivec3 relScaledPos = coordinateToCell(position);
	float scaledPerception = perceptionRadius / cellSize;
	int perceptionFloor = static_cast<int>(glm::floor(scaledPerception));
	int perceptionCeil = static_cast<int>(glm::ceil(scaledPerception));
	for (int i = glm::max(relScaledPos.x - perceptionFloor, 0); i <= glm::min(relScaledPos.x + perceptionCeil, xSize-1); i++) {
		for (int j = glm::max(relScaledPos.y - perceptionFloor, 0); j <= glm::min(relScaledPos.y + perceptionCeil, ySize-1); j++) {
			for (int k = glm::max(relScaledPos.z - perceptionFloor, 0); k <= glm::min(relScaledPos.z + perceptionCeil, zSize-1); k++) {

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
	return selDir / (float)sameLowest;

}

void TreeWorld::castShadows(const vec3& pos, int pyramidHeight, float a, float b)
{
	ivec3 relScaledPos = coordinateToCell(pos);
	for (int q = 0; q <= pyramidHeight; q++) {
		int j = relScaledPos.y - q;
		if (j < 0)
			break;
		if (j >= ySize)
			continue;
		for (int i = glm::max(relScaledPos.x - q, 0); i <= glm::min(relScaledPos.x + q, xSize-1); i++) {
			for (int k = glm::max(relScaledPos.z - q, 0); k <= glm::min(relScaledPos.z + q, zSize-1); k++) {
				float s = a * glm::pow(b, -q);
				addShadowTo(ivec3(i, j, k), s);
			}
		}
	}
}

ivec3 TreeWorld::coordinateToCell(const vec3 & pos)
{
	vec3 relPos = (pos - leftBottomCorner) / cellSize;
	int x = static_cast<int>(glm::round(relPos.x));
	int y = static_cast<int>(glm::round(relPos.y));
	int z = static_cast<int>(glm::round(relPos.z));
	return vec3(x, y, z);
}

vec3 TreeWorld::cellToCoordinate(const ivec3 & cell)
{
	return vec3(cell) * cellSize + leftBottomCorner;
}

int TreeWorld::cellToIndex(const ivec3& cell) const
{
	return xSize * ySize * cell.z + xSize * cell.y + cell.x;
}


ShadowCell & TreeWorld::getCellAt(const ivec3& cell)
{
	return shadowGrid[cellToIndex(cell)];
}

void TreeWorld::addShadowTo(const ivec3& cell, float amount)
{
	getCellAt(cell).shadow += amount;
}
