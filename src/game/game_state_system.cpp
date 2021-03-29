#include "game_state_system.hpp"
#include <level_loader/level_loader.hpp>
#include "ui/menus.hpp"
#include "camera.hpp"
#include "level_loader/level_loader.hpp"

#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>



GameStateSystem::GameStateSystem() {
	isInStory = false;
	isInTutorial = false;
	hasDoneTutorial = false;
	isInMainScreen = false;
	isInHelpScreen = false;
	currentLevelIndex = -1;
	currentTutorialIndex = 0;
	currentStoryIndex = 0;

	LevelLoader lc;
	recipe = lc.readLevel("recipe-2");
	currentLevel = recipe["maps"][0];
	//Create all the recipes here
	//auto firstRecipe = ECS::Entity();
	//auto& recipe = firstRecipe.emplace<Recipe>();
	//recipe.levels.push_back("pizza-arena");
	//recipe.levels.push_back("dessert-arena");
	//TODO add the victory and defeat screens


	//For now start with the firstRecipe as the currentRecipeEntity but in the future it should start as nothing and we will start in the main menu which will select a recipe
	//currentRecipeEntity = firstRecipe;
}

const vec2 GameStateSystem::getScreenBufferSize()
{
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	return vec2(frameBufferWidth, frameBufferHeight);
}

bool GameStateSystem::inGameState() {
	//TODO if we add more states that the game can be in add them here if they are relevant.
	return !isInMainScreen && !isInVictoryScreen && !isInDefeatScreen && !isInStory;
}

bool GameStateSystem::hasLights() {
	bool isVeggieForest = (currentLevel["map"] == "veggie-forest");
	return inGameState() && isVeggieForest;
}

void GameStateSystem::newGame()
{
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
};

void GameStateSystem::nextMap()
{
	//Save the game
	LevelLoader lc;
	lc.save(recipe["name"], currentLevelIndex);

	currentLevelIndex++;

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

void GameStateSystem::save()
{
	//TODO fill in if necessary
}

void GameStateSystem::launchVictoryScreen()
{
	Camera::createCamera(vec2(0.f));
	isInVictoryScreen = true;
	removeAllMotionEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createVictoryScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchDefeatScreen()
{
	Camera::createCamera(vec2(0.f));
	isInDefeatScreen = true;
	removeAllMotionEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createDefeatScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchMainMenu()
{
	isInMainScreen = true;
	Camera::createCamera(vec2(0.f));
	MouseClickFX::createMouseClickFX();
	removeAllMotionEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	StartMenu::createStartMenu(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::removeAllMotionEntities()
{
	while (!ECS::registry<Motion>.entities.empty())
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<Motion>.entities.back());

	std::cout << "Entity removal complete. \n";
	ECS::ContainerInterface::listAllComponents();
}

void GameStateSystem::setWindow(GLFWwindow* window)
{
	this->window = window;
}
