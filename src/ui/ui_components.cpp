#include "ui_components.hpp"
#include "animation/animation_components.hpp"

ClickableCircleComponent::ClickableCircleComponent(vec2 position, float radius, void (*callback)())
{
	this->position = position;
	this->radius = radius;
	this->callback = callback;
}

ClickableRectangleComponent::ClickableRectangleComponent(vec2 position, float width, float height, void(*callback)())
{
	this->position = position;
	this->width = width;
	this->height = height;
	this->callback = callback;
}

PlayerButtonComponent::PlayerButtonComponent(PlayerType player)
{
	this->player = player;
}

ECS::Entity MouseClickFX::createMouseClickFX()
{
	auto entity = ECS::Entity();

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = vec2(0.f);
	motion.angle = 0.f;
	motion.velocity = vec2(0.f);
	motion.scale = vec2(1.f);
	motion.boundingBox = vec2(0.f);

	auto idle_anim = new AnimationData("fx_mouseclick", uiPath("mouseclick_fx/mouseclick_fx"), 12, 1, false, false);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::EFFECT, *idle_anim);
	anims.currAnimData.currFrame = 11; // start with the animation finished

	entity.emplace<MouseClickFX>();

	return entity;
}