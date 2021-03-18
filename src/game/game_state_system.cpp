#include "game_state_system.hpp"
#include <level_loader/level_loader.hpp>
#include "ui/menus.hpp"
#include "camera.hpp"
#include "level_loader/level_loader.hpp"



GameStateSystem::GameStateSystem() {
	isInTutorial = false;
	hasDoneTutorial = false;
	isInMainScreen = false;
	isInHelpScreen = false;
	currentLevelIndex = -1;
	currentTutorialIndex = 0;

	LevelLoader lc;
	recipe = lc.readLevel("recipe-1");
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
	return !isInHelpScreen && !isInMainScreen;
}

void GameStateSystem::newGame()
{
	isInTutorial = true;
	hasDoneTutorial = false;
	currentTutorialIndex = 0;
	currentLevelIndex = 0;
	currentLevel = recipe["maps"][currentLevelIndex];
	EventSystem<LoadLevelEvent>::instance().sendEvent(LoadLevelEvent{});
	EventSystem<StartTutorialEvent>::instance().sendEvent(StartTutorialEvent{});
}

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
		//TODO handle finishing the recipe here but for now just restart the recipe.
		currentLevelIndex = 0;
		currentLevel = recipe["maps"][currentLevelIndex];
		EventSystem<LoadLevelEvent>::instance().sendEvent(LoadLevelEvent{});
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
	//TODO fill in if necessary
}

void GameStateSystem::save()
{
	//TODO fill in if necessary
}

void GameStateSystem::launchVictoryScreen()
{
	//TODO fill in once the victory screen has been hooked up
}

void GameStateSystem::launchDefeatScreen()
{
	//TODO fill in once the victory screen has been hooked up
}

void GameStateSystem::launchMainMenu()
{
	isInMainScreen = true;
	Camera::createCamera(vec2{ 0.0f,0.0f });
	vec2 screenBufferSize = getScreenBufferSize();
	StartMenu::createStartMenu(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::setWindow(GLFWwindow* window)
{
	this->window = window;
}
