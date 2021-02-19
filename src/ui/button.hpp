#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"

enum class ButtonShape { CIRCLE, RECTANGLE };

struct Button
{
	static ECS::Entity createButton(ButtonShape shape, vec2 position, const std::string& texture, void(*callback)());
	static ECS::Entity createPlayerButton(PlayerType player, vec2 position, void(*callback)());
};