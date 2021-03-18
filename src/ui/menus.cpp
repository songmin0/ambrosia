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
	glow.emplace<Motion>().position = vec2(frameBufferWidth / 2, frameBufferHeight / 2);

	auto logo = ECS::Entity();
	ShadedMesh& logoResource = cacheResource("ambrosia_logo");
	if (logoResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(logoResource, uiPath("menus/start/title-button.png"), "distendable");
	}
	logo.emplace<ShadedMeshRef>(logoResource);
	logo.emplace<DistendableComponent>(0.06f, 0.1f, 0.03f, 0.3f);
	logo.emplace<RenderableComponent>(RenderLayer::UI);
	logo.emplace<Motion>().position = vec2(frameBufferWidth / 2 - 30, frameBufferHeight / 3 - 50);

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

void Screens::createVictoryScreen(int frameBufferWidth, int frameBufferHeight, int type)
{
	auto background = ECS::Entity();
	const std::string key = "victory-" + std::to_string(type);
	ShadedMesh& splashResource = cacheResource(key);
	if (splashResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(splashResource, uiPath("menus/" + key + ".png"), "textured");
	}
	background.emplace<ShadedMeshRef>(splashResource);
	background.emplace<RenderableComponent>(RenderLayer::MAP);
	background.emplace<Motion>();
	auto& mapComponent = background.emplace<MapComponent>();
	mapComponent.name = key;
	mapComponent.mapSize = static_cast<vec2>(splashResource.texture.size);

	auto victoryLogo = ECS::Entity();
	ShadedMesh& logoResource = cacheResource("victory_logo");
	if (logoResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(logoResource, uiPath("menus/victory-logo.png"), "distendable");
	}
	victoryLogo.emplace<DistendableComponent>();
	victoryLogo.emplace<ShadedMeshRef>(logoResource);
	victoryLogo.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);
	victoryLogo.emplace<Motion>().position = vec2(frameBufferWidth / 2, frameBufferHeight / 2 - 150);

	// TODO: Hook up 
	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 2, frameBufferHeight / 2 + 180 }, "menus/next-button",
		[]() {
			std::cout << "Next button clicked!" << std::endl;
		});
};
