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