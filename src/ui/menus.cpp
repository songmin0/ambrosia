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
			GameStateSystem::instance().isTransitioning = true;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = false;
				GameStateSystem::instance().beginStory();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth - 250, frameBufferHeight / 2 + 150 }, "menus/start/load-button",
		[]() {
			std::cout << "Load button clicked!" << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = false;
				GameStateSystem::instance().loadSave();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth - 250, frameBufferHeight / 2 + 250 }, "menus/start/achievements-button",
		[]() {
			std::cout << "Achievement button clicked!" << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = false;
				GameStateSystem::instance().isInAchievementsScreen = true;
				GameStateSystem::instance().launchAchievementsScreen();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});

	Button::createButton(ButtonShape::RECTANGLE,
		{ 100, frameBufferHeight - 100 }, "menus/start/credits-button",
		[]() {
			std::cout << "Credits button clicked!" << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = false;
				GameStateSystem::instance().isInCreditsScreen = true;
				GameStateSystem::instance().launchCreditsScreen();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
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
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInVictoryScreen = false;
				GameStateSystem::instance().nextMap();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
};

void Screens::createDefeatScreen(int frameBufferWidth, int frameBufferHeight, int type)
{
	auto background = ECS::Entity();
	const std::string key = "defeat-" + std::to_string(type);
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

	auto logo = ECS::Entity();
	ShadedMesh& logoResource = cacheResource("defeat_logo");
	if (logoResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(logoResource, uiPath("menus/defeat-logo.png"), "distendable");
	}
	logo.emplace<DistendableComponent>(0.05f, 0.1f, 0.1f, 0.25f);
	logo.emplace<ShadedMeshRef>(logoResource);
	logo.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);
	logo.emplace<Motion>().position = vec2(frameBufferWidth / 2, frameBufferHeight / 2 - 180);

	auto tryAgain = ECS::Entity();
	ShadedMesh& tryagainResource = cacheResource("tryagain_logo");
	if (tryagainResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(tryagainResource, uiPath("menus/try-again.png"), "textured");
	}
	tryAgain.emplace<ShadedMeshRef>(tryagainResource);
	tryAgain.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);
	tryAgain.emplace<Motion>().position = vec2(frameBufferWidth / 2, frameBufferHeight / 2 + 20);

	// TODO: Hook up 
	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 2 - 120, frameBufferHeight / 2 + 180 }, "menus/yes-button",
		[]() {
			//Load save
			std::cout << "Yes button clicked!" << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInDefeatScreen = false;
				GameStateSystem::instance().restartMap();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 2 + 120, frameBufferHeight / 2 + 180 }, "menus/no-button",
		[]() {
			//Go to main menu
			std::cout << "No button clicked..." << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInDefeatScreen = false;
				GameStateSystem::instance().launchMainMenu();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
}
void Screens::createAchievementsScreen(int frameBufferWidth, int frameBufferHeight, int type)
{
	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 2.f - 500, 60 }, "menus/back-button",
		[]() {
			std::cout << "Back button clicked!" << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInAchievementsScreen = false;
				GameStateSystem::instance().launchMainMenu();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
}
void Screens::createCreditsScreen(int frameBufferWidth, int frameBufferHeight, int type)
{
	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 2.f - 500, 60 }, "menus/back-button",
		[]() {
			std::cout << "Back button clicked!" << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInCreditsScreen = false;
				GameStateSystem::instance().launchMainMenu();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
}
;
