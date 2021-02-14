#include "Button.hpp"
#include "ui_components.hpp"
#include "render.hpp"

ECS::Entity Button::createButton(ButtonShape shape, vec2 position, const std::string& texture, void(*callback)())
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource(texture);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, uiPath(texture + ".png"), "textured");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	motion.boundingBox = motion.scale * vec2({ resource.texture.size.x, resource.texture.size.y });

	// Add clickable component to button depending on shape
	switch (shape) {
	case ButtonShape::CIRCLE:
		entity.emplace<ClickableCircleComponent>(position, resource.texture.size.x / 2, callback);
		break;
	case ButtonShape::RECTANGLE:
		entity.emplace<ClickableRectangleComponent>(position, resource.texture.size.x, resource.texture.size.y, callback);
		break;
	default:
		break;
	}

	ECS::registry<Button>.emplace(entity);

	return entity;
}