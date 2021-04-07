#include "game_state_system.hpp"
#include "ui/menus.hpp"
#include "camera.hpp"
#include "level_loader/level_loader.hpp"
#include "rendering/text.hpp"
#include "achievement_system.hpp"

#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>



GameStateSystem::GameStateSystem() {
	isInStory = false;
	isInTutorial = false;
	hasDoneTutorial = false; 
	isInAchievementsScreen = false;
	isInCreditsScreen = false;
	isInMainScreen = false;
	isInHelpScreen = false;
	currentLevelIndex = -1;
	currentTutorialIndex = 0;
	currentStoryIndex = 0;

	LevelLoader lc;
	recipe = lc.readLevel("tutorial");
	currentLevel = recipe["maps"][0];
	json save_obj = lc.load();
	if (save_obj.contains("recipe"))
	{
		AchievementSystem::instance().clearAchievements();
		for (Achievement achievement : save_obj["achievements"])
		{
			AchievementSystem::instance().addAchievement(achievement);
		}
	}
}

const vec2 GameStateSystem::getScreenBufferSize()
{
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	return vec2(frameBufferWidth, frameBufferHeight);
}

bool GameStateSystem::inGameState() {
	//TODO if we add more states that the game can be in add them here if they are relevant.
	return !isInMainScreen && !isInVictoryScreen && !isInDefeatScreen && !isInStory && !isInAchievementsScreen && !isInCreditsScreen;
}

bool GameStateSystem::hasLights() {
	bool isVeggieForest = (currentLevel["map"] == "veggie-forest");
	bool isSaladCanyon = (currentLevel["map"] == "salad-canyon");
	return inGameState() && (isVeggieForest || isSaladCanyon);
}

void GameStateSystem::newGame()
{
	LevelLoader lc;
	recipe = lc.readLevel("tutorial");
	isInTutorial = true;
	hasDoneTutorial = false;
	isInDefeatScreen = false;
	isInVictoryScreen = false;
	isInMainScreen = false;
	isInStory = false;
	currentTutorialIndex = 0;
	currentLevelIndex = 0;
	currentLevel = recipe["maps"][currentLevelIndex];
	EventSystem<LoadLevelEvent>::instance().sendEvent(LoadLevelEvent{});
	EventSystem<StartTutorialEvent>::instance().sendEvent(StartTutorialEvent{});
};

void GameStateSystem::beginStory()
{
	removeAllMotionEntities();
	isInTutorial = false;
	isInDefeatScreen = false;
	isInVictoryScreen = false;
	isInMainScreen = false;
	isInStory = true;
	currentStoryIndex = 0;
	EventSystem<AdvanceStoryEvent>::instance().sendEvent(AdvanceStoryEvent{});
	createText("Press space to skip.", { 50.0, 50.0 }, 0.3f);
};

void GameStateSystem::nextMap()
{
	//Save the game
	LevelLoader lc;
	std::list<Achievement> achievements = AchievementSystem::instance().getAchievements();
	lc.save(recipe["name"], currentLevelIndex, achievements);

	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});

	currentLevelIndex++;
	if (currentLevelIndex == recipe["maps"].size() - 1)
	{
		EventSystem<ReachedBossEvent>::instance().sendEvent(ReachedBossEvent{});
		std::cout << "ReachedBossEvent\n";
	}

	//auto& currentRecipe = currentRecipeEntity.get<Recipe>();
	if (currentLevelIndex < recipe["maps"].size()) {
		//Read the next level from the recipe
		currentLevel = recipe["maps"][currentLevelIndex];
		EventSystem<LoadLevelEvent>::instance().sendEvent(LoadLevelEvent{});
	}
	else {
		//TODO handle finishing the recipe here but for now return to main menu
		currentLevelIndex = 0;
		currentLevel = recipe["maps"][currentLevelIndex];
		launchMainMenu();
	}
}

void GameStateSystem::restartMap()
{
	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});
	currentLevel = recipe["maps"][currentLevelIndex];
	EventSystem<LoadLevelEvent>::instance().sendEvent(LoadLevelEvent{});

	//TODO fill in
}

void GameStateSystem::loadSave()
{
	LevelLoader lc;
	json save_obj = lc.load();
	if (save_obj.contains("recipe"))
	{
		GameStateSystem::instance().recipe = lc.readLevel(save_obj["recipe"]);
		GameStateSystem::instance().currentLevelIndex = save_obj["level"];
		GameStateSystem::instance().restartMap();
	}
	else // load a new game because there's no save
	{
		beginStory();
	}
}

void GameStateSystem::loadRecipe(const std::string& recipe)
{
	LevelLoader lc;
	GameStateSystem::instance().recipe = lc.readLevel(recipe);
	GameStateSystem::instance().currentLevelIndex = 0;
	GameStateSystem::instance().restartMap();
}

void GameStateSystem::save()
{
	//TODO fill in if necessary
}

void GameStateSystem::launchVictoryScreen()
{
	Camera::createCamera(vec2(0.f));
	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});
	isInVictoryScreen = true;
	if (recipe["name"] != "tutorial")
	{
		EventSystem<BeatLevelEvent>::instance().sendEvent(BeatLevelEvent{});
		// Defeated the boss
		if (currentLevelIndex == recipe["maps"].size() - 1)
			EventSystem<DefeatedBossEvent>::instance().sendEvent(DefeatedBossEvent{});
	}
	// Must call removeAllMotionEntities() *after* sending achievement events
	removeAllMotionEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createVictoryScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchDefeatScreen()
{
	Camera::createCamera(vec2(0.f));
	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});
	isInDefeatScreen = true;
	removeAllMotionEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createDefeatScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchMainMenu()
{
	Camera::createCamera(vec2(0.f));
	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});
	isInMainScreen = true;
	removeAllMotionEntities();
	MouseClickFX::createMouseClickFX();
	vec2 screenBufferSize = getScreenBufferSize();
	StartMenu::createStartMenu(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchAchievementsScreen()
{
	removeAllMotionEntities();
	isInAchievementsScreen = true;
	Camera::createCamera(vec2(0.f));
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createAchievementsScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchCreditsScreen()
{
	removeAllMotionEntities();
	Camera::createCamera(vec2(0.f));
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createCreditsScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchRecipeSelectMenu()
{
	isInMainScreen = true;
	Camera::createCamera(vec2(0.f));
	removeAllMotionEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createRecipeSelectScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::removeAllMotionEntities()
{
	//while (!ECS::registry<Motion>.entities.empty())
	//	ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<Motion>.entities.back());
	for (auto& motion : ECS::registry<Motion>.entities)
	{
		if (!(motion.has<AchievementPopup>() && isInVictoryScreen))
			ECS::ContainerInterface::removeAllComponentsOf(motion);
	}

	for (auto& text : ECS::registry<Text>.entities)
	{
		if (!(text.has<AchievementMessage>() && isInVictoryScreen))
			ECS::ContainerInterface::removeAllComponentsOf(text);
	}

	std::cout << "Entity removal complete. \n";
	ECS::ContainerInterface::listAllComponents();
}

void GameStateSystem::setWindow(GLFWwindow* window)
{
	this->window = window;
}
