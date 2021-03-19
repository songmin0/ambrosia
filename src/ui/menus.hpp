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

struct Screens
{
	static void createVictoryScreen(int frameBufferWidth, int frameBufferHeight, int type = 1);
	static void createDefeatScreen(int frameBufferWidth, int frameBufferHeight, int type = 1);
};
