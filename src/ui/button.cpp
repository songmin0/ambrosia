#include "button.hpp"
#include "ui_components.hpp"
#include "rendering/render.hpp"
#include "animation/animation_components.hpp"

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

ECS::Entity Button::createPlayerButton(PlayerType player, vec2 position, void(*callback)())
{
	auto entity = Button::createButton(ButtonShape::RECTANGLE, position, "placeholder_char_button", callback);
	std::string playerName = "";

	switch (player) {
	case PlayerType::RAOUL:
		playerName = "raoul";
		entity.emplace<PlayerButtonComponent>(PlayerType::RAOUL);
		break;
	case PlayerType::TAJI:
		playerName = "taji";
		entity.emplace<PlayerButtonComponent>(PlayerType::TAJI);
		break;
	case PlayerType::CHIA:
		playerName = "chia";
		entity.emplace<PlayerButtonComponent>(PlayerType::CHIA);
		break;
	case PlayerType::SPICY:
		playerName = "spicy";
		entity.emplace<PlayerButtonComponent>(PlayerType::SPICY);
		break;
	default:
		break;
	}

	auto active_anim = new AnimationData(
		playerName + "button_active", uiPath("player_buttons/" + playerName + "/active/" + playerName + "_active"), 40);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::ACTIVE, *active_anim);

	auto inactive_anim = new AnimationData(
		playerName + "button_inactive", uiPath("player_buttons/" + playerName + "/" + playerName + "_inactive"), 1);
	anims.addAnimation(AnimationType::INACTIVE, *inactive_anim);

	auto disabled_anim = new AnimationData(
		playerName + "button_disabled", uiPath("player_buttons/" + playerName + "/" + playerName + "_disabled"), 1);
	anims.addAnimation(AnimationType::DISABLED, *disabled_anim);

	return entity;
}