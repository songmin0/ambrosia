#include "menus.hpp"
#include "map.hpp"
#include "game/game_state_system.hpp"
#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#define SDL_MAIN_HANDLED

void StartMenu::createStartMenu(int frameBufferWidth, int frameBufferHeight)
{
	//Play menu music
	Mix_PlayMusic(Mix_LoadMUS(audioPath("music/Ambrosia_Theme.wav").c_str()), -1);

	// Background
	auto background = ECS::Entity();
	ShadedMesh& splashResource = cacheResource("start_splash");
	if (splashResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(splashResource, uiPath("menus/start/start-splash.png"), "textured");
	}
	background.emplace<ShadedMeshRef>(splashResource);
	background.emplace<RenderableComponent>(RenderLayer::MAP);
	background.emplace<Motion>();
	auto& mapComponent = background.emplace<MapComponent>();
	mapComponent.name = "startScreen";
	mapComponent.mapSize = static_cast<vec2>(splashResource.texture.size);

	// Glow
	auto glow = ECS::Entity();
	ShadedMesh& glowResource = cacheResource("start_glow");
	if (glowResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(glowResource, uiPath("menus/start/start-glow.png"), "fading");
	}

	glow.emplace<ShadedMeshRef>(glowResource);
	glow.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);

	Motion& motion = glow.emplace<Motion>();
	motion.position = vec2(683.f, 450.f);

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 2 - 30, frameBufferHeight / 3 - 50}, "menus/start/title-button",
		[]() {
			std::cout << "Title clicked! ... though it shouldn't do anything?" << std::endl;
		});

	Button::createButton(ButtonShape::RECTANGLE, 
		{ frameBufferWidth - 250, frameBufferHeight / 2 + 50 }, "menus/start/start-button",
		[]() {
			std::cout << "Start button clicked!" << std::endl;
			GameStateSystem::instance().isInMainScreen = false;
			GameStateSystem::instance().newGame();
		});

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth - 250, frameBufferHeight / 2 + 150 }, "menus/start/load-button",
		[]() {
			std::cout << "Load button clicked!" << std::endl;
		});

	Button::createButton(ButtonShape::RECTANGLE,
		{ 100, frameBufferHeight - 100 }, "menus/start/credits-button",
		[]() {
			std::cout << "Credits button clicked!" << std::endl;
		});
}
