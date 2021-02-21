#include "map.hpp"
#include "rendering/render.hpp"
#include <iostream>
//#include "stb_image.h"

void getPixel(stbi_uc* image, size_t imageWidth, size_t x, size_t y, stbi_uc* r, stbi_uc* g, stbi_uc* b, stbi_uc* a) {
	const stbi_uc* p = image + (4 * (y * imageWidth + x));
	*r = p[0];
	*g = p[1];
	*b = p[2];
	*a = p[3];
}

ECS::Entity MapComponent::createMap(const std::string& name, vec2 screenSize)
{
	auto entity = ECS::Entity();

	std::string navmeshAlphaPath = mapsPath(name + "-navmesh-alpha" + ".png");
	std::string debugPath = mapsPath(name + "-debug" + ".png");

	// Create rendering primitives
	ShadedMesh& resource = cacheResource(name);
	if (resource.effect.program.resource == 0)
		RenderSystem::createSprite(resource, debugPath, "textured");
	// TODO: change to normal map after done with debug map
	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	entity.emplace<ShadedMeshRef>(resource);

	// Initialize the position and scale
	auto& motion = entity.emplace<Motion>();
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = screenSize / 2.f;
	motion.scale = screenSize / static_cast<vec2>(resource.texture.size);

	auto& mapComponent = entity.emplace<MapComponent>();
	mapComponent.name = name;

	// Temporarily hardcoded until we implement a function to load this from the data file
	mapComponent.tileSize = 32;

	std::string navmeshPath = mapsPath(name  + "-navmesh-alpha" + ".png");

	int width, height, numChannelsRead;
	unsigned char* data = stbi_load(navmeshPath.c_str(), &width, &height, &numChannelsRead, 0);
	assert(numChannelsRead == 4);

	if (data == nullptr)
	{
		throw std::runtime_error("Failed to load map from nav mesh");
	}

	int num_tiles_x = width / mapComponent.tileSize;
	int num_tiles_y = height / mapComponent.tileSize;

	std::vector< std::vector <int> > matrix(num_tiles_y, std::vector <int>(num_tiles_x));
	stbi_uc r, g, b, a;
	int pixelX = 0;
	int pixelY = 0;
	for (int y = 0; y < num_tiles_y; y++) {
		pixelX = 0;
		for (int x = 0; x < num_tiles_x; x++) {

			getPixel(data, width, pixelX, pixelY, &r, &g, &b, &a);

			if (a < 100) {
				matrix[y][x] = 0; // for all darkness values above 100, consider filled
			}
			else {
				matrix[y][x] = 3;
			}
			//std::cout << matrix[y][x]; // comment these two lines out if you would like to see the map that is being generated in terminal
			pixelX += mapComponent.tileSize;
		}
		//std::cout << std::endl;
		pixelY += mapComponent.tileSize;
	}

	mapComponent.grid = matrix;
	return entity;
}

