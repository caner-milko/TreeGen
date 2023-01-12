#include "Terrain.h"

Terrain::Terrain(const TerrainData& data) : data(data)
{
}

void Terrain::generateMesh(std::vector<TerrainVertex>* vertices, std::vector<int32>* indices) const
{
	int verticesSize = (data.gridSize.x + 1) * (data.gridSize.y + 1);
	*vertices = std::vector<TerrainVertex>();
	vertices->reserve(verticesSize);
	int indicesSize = data.gridSize.x * data.gridSize.y * 2 * 3;
	*indices = std::vector<int32>();
	indices->reserve(indicesSize);


	for (int32 y = 0; y <= data.gridSize.y; y++) {
		for (int32 x = 0; x <= data.gridSize.x; x++) {
			TerrainVertex vertex;
			vec2 uv = cellSize() * vec2(x, y);
			vertex.pos = vec3(uv.x, heightAt(uv), uv.y);
			vertex.normal = normalAt(vec2(x, y));
			vertices->emplace_back(vertex);
		}
	}
	for (int32 x = 0; x < data.gridSize.x; x++) {
		for (int32 y = 0; y < data.gridSize.y; y++) {
			//first triangle
			indices->emplace_back(vertexIndex(ivec2(x, y)));
			indices->emplace_back(vertexIndex(ivec2(x + 1, y)));
			indices->emplace_back(vertexIndex(ivec2(x, y + 1)));
			//second triangle
			indices->emplace_back(vertexIndex(ivec2(x + 1, y)));
			indices->emplace_back(vertexIndex(ivec2(x + 1, y + 1)));
			indices->emplace_back(vertexIndex(ivec2(x, y + 1)));
		}
	}
	int indexSize = indices->size();

}

float Terrain::heightAt(vec2 uv) const
{
	return data.heightMap->getRGB<1>(uv).x / 256.0;
}

vec3 Terrain::normalAt(ivec2 vertex) const
{
	ivec2 nX = vertex + ivec2(-1 * (vertex.x > 0), 0);
	ivec2 nY = vertex + ivec2(0, -1 * (vertex.y > 0));
	ivec2 pX = vertex + ivec2(1 * (vertex.x <= data.gridSize.y), 0);
	ivec2 pY = vertex + ivec2(0, 1 * (vertex.y <= data.gridSize.y));

	vec2 cells = cellSize();

	vec2 nXuv = cells * vec2(nX);
	vec2 nYuv = cells * vec2(nY);
	vec2 pXuv = cells * vec2(pX);
	vec2 pYuv = cells * vec2(pY);

	float nXh = heightAt(nXuv);
	float nYh = heightAt(nYuv);
	float pXh = heightAt(pXuv);
	float pYh = heightAt(pYuv);

	vec3 xDif = glm::normalize(vec3(pXuv.x, pXh, pXuv.y) - vec3(nXuv.x, nXh, nXuv.y));
	vec3 yDif = glm::normalize(vec3(pYuv.x, pYh, pYuv.y) - vec3(nYuv.x, nYh, nYuv.y));

	return glm::normalize(glm::cross(yDif, xDif));

}
