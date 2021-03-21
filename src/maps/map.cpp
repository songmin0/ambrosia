#include "map.hpp"
#include "rendering/render.hpp"
#include <iostream>

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

	std::string navmeshPath = mapsPath(name + "/" + name + "-navmesh" + ".png");
	std::string debugPath = mapsPath(name + "/" + name + "-debug" + ".png");
	std::string mapPath = mapsPath(name + "/" + name + ".png");

	// Create rendering primitives
	ShadedMesh& resource = cacheResource(name);
	if (resource.effect.program.resource == 0)
		RenderSystem::createSprite(resource, mapPath, "textured");
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::MAP);

	// Initialize the position and scale
	auto& motion = entity.emplace<Motion>();
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = { 0.f, 0.f };
	motion.scale = { 1.f, 1.f };

	auto& mapComponent = entity.emplace<MapComponent>();
	mapComponent.name = name;
	mapComponent.mapSize = static_cast<vec2>(resource.texture.size);
	mapComponent.tileSize = 32.f;

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

			if (r < 100) {
				matrix[y][x] = 0; // for all darkness values above 100, consider filled
			}
			else {
				matrix[y][x] = 3;
			}
			//std::cout << matrix[y][x]; // uncomment these two lines out if you would like to see the map that is being generated in the debug console
			pixelX += mapComponent.tileSize;
		}
		//std::cout << std::endl;
		pixelY += mapComponent.tileSize;
	}

	mapComponent.grid = matrix;

	stbi_image_free(data);
	return entity;
}

