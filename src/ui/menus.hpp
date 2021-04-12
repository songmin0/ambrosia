#pragma once
#include "ui_components.hpp"
#include "button.hpp"
#include "effects.hpp"
#include "ui_entities.hpp"
#include "game/common.hpp"
#include "game/event_system.hpp"
#include "game/events.hpp"
#include "entities/tiny_ecs.hpp"
#include "animation/animation_components.hpp"

struct StartMenu
{
	static void createStartMenu(int frameBufferWidth, int frameBufferHeight);
};

namespace Screens
{
	void createVictoryScreen(int frameBufferWidth, int frameBufferHeight, int type = 1);
	void createDefeatScreen(int frameBufferWidth, int frameBufferHeight, int type = 1);
	void createAchievementsScreen(int frameBufferWidth, int frameBufferHeight, int type = 1);
	void createCreditsScreen(int frameBufferWidth, int frameBufferHeight, int type = 1);
	void createRecipeSelectScreen(int frameBufferWidth, int frameBufferHeight);
	void createShopScreen(int frameBufferWidth, int frameBufferHeight, ECS::Entity raoul, ECS::Entity chia, ECS::Entity ember, ECS::Entity taji);
};
