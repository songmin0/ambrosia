#include "button.hpp"
#include "ui_components.hpp"
#include "rendering/render.hpp"
#include "animation/animation_components.hpp"
#include <iostream>
#include <game/turn_system.hpp>

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

	entity.emplace<Button>();

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
	case PlayerType::EMBER:
		playerName = "spicy";
		entity.emplace<PlayerButtonComponent>(PlayerType::EMBER);
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

ECS::Entity SkillButton::createSkillButton(vec2 position, PlayerType player, SkillType skillType, const std::string& texture, void(*callback)())
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource(texture);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath(texture + ".png"), "dynamic_texture");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	motion.boundingBox = motion.scale * vec2({ resource.texture.size.x, resource.texture.size.y });

	entity.emplace<ClickableCircleComponent>(position, resource.texture.size.x / 2, callback);
	entity.emplace<ButtonStateComponent>();
	entity.emplace<SkillInfoComponent>(player, skillType);
	entity.emplace<VisibilityComponent>();

	if (skillType == SkillType::MOVE)
	{
		entity.emplace<MoveButtonComponent>();
	}

	entity.emplace<SkillButton>();

	return entity;
}

// This will be moved to ui_entities.cpp once the branch that has it is merged...
ECS::Entity ToolTip::createToolTip(PlayerType player, SkillType skillType, vec2 position)
{
	auto entity = ECS::Entity();
	std::string texture = "move_tooltip";

	switch (skillType) {
	case SkillType::SKILL1:
		texture = "raoul_skill1";
		break;
	case SkillType::SKILL2:
		texture = "raoul_skill2";
		break;
	case SkillType::SKILL3:
		texture = "raoul_skill3";
		break;
	default:
		entity.emplace<MoveToolTipComponent>();
		break;
	}

	ShadedMesh& resource = cacheResource(texture);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tooltips/" + texture + ".png"), "textured");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position + vec2(resource.texture.size.x / 2.f, -resource.texture.size.y) / 2.f;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	motion.boundingBox = motion.scale * vec2({ resource.texture.size.x, resource.texture.size.y });

	entity.emplace<SkillInfoComponent>(player, skillType);
	entity.emplace<VisibilityComponent>().isVisible = false;

	entity.emplace<ToolTip>();

	return entity;
}
