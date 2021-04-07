#include "menus.hpp"
#include "map.hpp"
#include "game/game_state_system.hpp"
#include "rendering/text.hpp"
#include "game/achievement_system.hpp"
#include <iostream>
#include <SDL.h>
#include <SDL_mixer.h>
#define SDL_MAIN_HANDLED

const float TITLE_LINE_Y = 80.f;

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
				GameStateSystem::instance().launchRecipeSelectMenu();
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
		{ frameBufferWidth - 250, frameBufferHeight - 100 }, "menus/start/achievements-button",
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
	const float leftAlignX = 100;
	const float lineSpaceY = 80.f;
	createText("Achievements", { leftAlignX, TITLE_LINE_Y });

	// Getting all achievements
	std::list<Achievement> achievements = AchievementSystem::instance().getAchievements();
	int lineIndex = 1;
	for (Achievement curr : achievements)
	{
		auto text = AchievementText[curr];
		createText(std::string(text), { leftAlignX, TITLE_LINE_Y + lineSpaceY * lineIndex }, 0.75);
		lineIndex++;
	}
	//createText("Beat tutorial", { leftAlignX, TITLE_LINE_Y + lineSpaceY }, 0.5);
	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth - 200, 60 }, "menus/back-button",
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
	const float leftAlignX = 100;
	const float middleAlignX = frameBufferWidth / 2.f;
	const float paragraphSpaceY = 80;
	const float newlineSpaceY = 50;
	createText("Credits", { leftAlignX, TITLE_LINE_Y });

	float firstSectionY = TITLE_LINE_Y + paragraphSpaceY;
	createText("Developers", { leftAlignX, firstSectionY }, 0.75);
	createText("Louise Hsu", { leftAlignX, firstSectionY + newlineSpaceY }, 0.5);
	createText("Emma Liu", { leftAlignX, firstSectionY + newlineSpaceY * 2 }, 0.5);
	createText("Jacques Marais", { leftAlignX, firstSectionY + newlineSpaceY * 3 }, 0.5);
	createText("Alexander Neumann", { middleAlignX, firstSectionY + newlineSpaceY }, 0.5);
	createText("Matthew Ng", { middleAlignX, firstSectionY + newlineSpaceY * 2 }, 0.5);
	createText("Christine Song", { middleAlignX, firstSectionY + newlineSpaceY * 3 }, 0.5);

	float secondSectionY = firstSectionY + newlineSpaceY * 3 + paragraphSpaceY;
	createText("Music", { leftAlignX, secondSectionY }, 0.75);
	createText("Ambrosia Theme - Emma Liu", { leftAlignX, secondSectionY + newlineSpaceY }, 0.5);
	createText("Overworld Theme - David Vitas", { leftAlignX, secondSectionY + newlineSpaceY * 2 }, 0.5);
	createText("Victory Fanfare Loop - David Vitas", { leftAlignX, secondSectionY + newlineSpaceY * 3 }, 0.5);
	createText("Boss Battle Loop - David Vitas", { middleAlignX, secondSectionY + newlineSpaceY }, 0.5);
	createText("Lullaby Loop - David Vitas", { middleAlignX, secondSectionY + newlineSpaceY * 2 }, 0.5);

	float thirdSectionY = secondSectionY + newlineSpaceY * 3 + paragraphSpaceY;
	createText("Sound Effects", { leftAlignX, thirdSectionY }, 0.75);
	createText("Player hit, mob hit, defeat", { leftAlignX, thirdSectionY + newlineSpaceY }, 0.5);
	createText("Sound effects obtained from https ://www.zapsplat.com", { leftAlignX, thirdSectionY + newlineSpaceY * 2 }, 0.3);
	
	createText("Mouse click, melee, projectile throw, buff, debuff", { leftAlignX, thirdSectionY + newlineSpaceY * 3 }, 0.5);
	createText("Credit: https://www.FesliyanStudios.com Background Music", { leftAlignX, thirdSectionY + newlineSpaceY * 4 }, 0.3);


	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth - 200, 60 }, "menus/back-button",
		[]() {
			std::cout << "Back button clicked!" << std::endl;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInCreditsScreen = false;
				GameStateSystem::instance().launchMainMenu();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
};

void Screens::createRecipeSelectScreen(int frameBufferWidth, int frameBufferHeight)
{
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

	std::string selectText("Select a recipe to play:");
	vec2 position(70.f, 80.f);
	float scale = 0.75f;
	createText(selectText, position, scale);

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

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 2, frameBufferHeight / 2 - 200 }, "recipe_select/tutorial-select",
		[]() {
			std::cout << "Tutorial selected!" << std::endl;
			GameStateSystem::instance().isTransitioning = true;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = false;
				GameStateSystem::instance().beginStory();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
	createText("tutorial", vec2(frameBufferWidth / 2 - 85, frameBufferHeight / 2 - 70), 0.6f);

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 4, frameBufferHeight / 2 + 100 }, "recipe_select/recipe1-select",
		[]() {
			std::cout << "Recipe 1 selected!" << std::endl;
			GameStateSystem::instance().isTransitioning = true;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = false;
				GameStateSystem::instance().isInStory = false;
				GameStateSystem::instance().isInTutorial = false;
				GameStateSystem::instance().loadRecipe("recipe-1");
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
	createText("Recipe 1", vec2(frameBufferWidth / 4 - 85, frameBufferHeight / 2 + 230), 0.6f);

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 4 * 2, frameBufferHeight / 2 + 100 }, "recipe_select/recipe2-select",
		[]() {
			std::cout << "Recipe 2 selected!" << std::endl;
			GameStateSystem::instance().isTransitioning = true;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = false;
				GameStateSystem::instance().isInStory = false;
				GameStateSystem::instance().isInTutorial = false;
				GameStateSystem::instance().loadRecipe("recipe-2");
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
	createText("Recipe 2", vec2(frameBufferWidth / 4 * 2 - 85, frameBufferHeight / 2 + 230), 0.6f);

	Button::createButton(ButtonShape::RECTANGLE,
		{ frameBufferWidth / 4 * 3, frameBufferHeight / 2 + 100 }, "recipe_select/recipe2-select",
		[]() {
			std::cout << "There will be a recipe 3. This is a placeholder for now." << std::endl;
		});
	createText("Recipe 3", vec2(frameBufferWidth / 4 * 3 - 85, frameBufferHeight / 2 + 230), 0.6f);

	Button::createButton(ButtonShape::RECTANGLE,
		{ 200, frameBufferHeight - 100 }, "menus/back-button",
		[]() {
			std::cout << "Returning to start screen" << std::endl;
			GameStateSystem::instance().isTransitioning = true;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().isInMainScreen = true;
				GameStateSystem::instance().launchMainMenu();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		});
}