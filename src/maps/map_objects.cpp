#include "map_objects.hpp"
#include "rendering/render.hpp"

ECS::Entity CheeseBlob::createCheeseBlob(vec2 position)
{
	auto entity = ECS::Entity();

	std::string key = "cheeseblob_static";
	ShadedMesh& resource = cacheResource(key);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, objectsPath("cheese-texture.png"), "bloblike");
	}

	// uncomment this if you want to try loading a textured mesh... right now it loads but its a tiny af square
	//if (resource.mesh.vertices.size() == 0)
	//{
	//	// this mesh loader doesn't support textures, if you load "blob-convert-mesh.obj" it works but then there's no texture coords
	//	// and then we can't see the image
	//	// so we'd have to create our own obj loader...
	//	resource.mesh.loadFromOBJFile(objectsPath("blob-mesh2.obj"));
	//	RenderSystem::CreateTexturedMesh(resource, objectsPath("cheese-texture.png"), "bloblike");
	//}

	entity.emplace<ShadedMeshRef>(resource);

	// Setting initial motion values
	Motion& motion = entity.emplace<Motion>();
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });

	// use this scale if you're doing a mesh to see it cause their current loader normalizes the mesh size to tiny
	//motion.scale = vec2({ 100f, -100f });
	//motion.boundingBox = motion.scale * vec2({ resource.texture.size.x, resource.texture.size.y });
	
	entity.emplace<CheeseBlob>();

	return entity;
};
