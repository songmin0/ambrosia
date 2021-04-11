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
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;

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

	auto active_anim = AnimationData(
		playerName + "button_active", uiPath("player_buttons/" + playerName + "/active/" + playerName + "_active"), 40);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::ACTIVE, std::make_shared<AnimationData>(active_anim));

	auto inactive_anim = AnimationData(
		playerName + "button_inactive", uiPath("player_buttons/" + playerName + "/" + playerName + "_inactive"), 1);
	anims.addAnimation(AnimationType::INACTIVE, std::make_shared<AnimationData>(inactive_anim));

	auto disabled_anim = AnimationData(
		playerName + "button_disabled", uiPath("player_buttons/" + playerName + "/" + playerName + "_disabled"), 1);
	anims.addAnimation(AnimationType::DISABLED, std::make_shared<AnimationData>(disabled_anim));

	return entity;
}

ECS::Entity SkillButton::createSkillButton(vec2 position, PlayerType player, SkillType skillType, const std::string& texture, void(*callback)())
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource(texture);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createPlayerSpecificMesh(resource, uiPath("skill_buttons/" + texture), "skill_button");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;

	entity.emplace<ClickableCircleComponent>(position, resource.texture.size.x / 2, callback);
	entity.emplace<ButtonStateComponent>();
	entity.emplace<SkillInfoComponent>(player, skillType);
	entity.emplace<VisibilityComponent>();

	entity.emplace<SkillButton>();

	return entity;
}

ECS::Entity SkillButton::createMoveButton(vec2 position, const std::string& texture, void(*callback)())
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("move_button");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath(texture + ".png"), "dynamic_texture");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;

	entity.emplace<ClickableCircleComponent>(position, resource.texture.size.x / 2, callback);
	entity.emplace<ButtonStateComponent>();
	entity.emplace<VisibilityComponent>();
	entity.emplace<MoveButtonComponent>();
	entity.emplace<SkillInfoComponent>(PlayerType::RAOUL, SkillType::MOVE);

	entity.emplace<SkillButton>();

	return entity;
}

ECS::Entity UpgradeButton::createUpgradeButton(vec2 position, PlayerType player, SkillType skillType, const std::string& texture, void(*callback)())
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource(texture);
	if (resource.effect.program.resource == 0)
	{
		if (skillType == SkillType::NONE) {
			RenderSystem::createSprite(resource, uiPath("shop/" + texture + ".png"), "skill_button");
		}
		else {
			RenderSystem::createPlayerSpecificMesh(resource, uiPath("skill_buttons/" + texture), "skill_button");
		}
		
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;

	entity.emplace<ClickableCircleComponent>(position, resource.texture.size.x / 2, callback);
	entity.emplace<SkillInfoComponent>(player, skillType);
	entity.emplace<VisibilityComponent>().isVisible = true;

	entity.emplace<UpgradeButton>();

	return entity;
}

ECS::Entity Button::createPlayerUpgradeButton(ButtonShape shape, vec2 position, const std::string& texture, void(*callback)())
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource(texture);
	if (resource.effect.program.resource == 0)
	{
		resource = ShadedMesh();
		RenderSystem::createSprite(resource, uiPath("shop/" + texture + ".png"), "textured");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;

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