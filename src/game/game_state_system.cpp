#include "game_state_system.hpp"
#include <level_loader/level_loader.hpp>
#include "ui/menus.hpp"
#include "camera.hpp"



GameStateSystem::GameStateSystem() {
		isInTutorial = false;
		hasDoneTutorial = false;
		isInMainScreen = false;
		isInHelpScreen = false;
		currentLevelIndex = -1;

		//Create all the recipes here
		auto firstRecipe = ECS::Entity();
		auto& recipe = firstRecipe.emplace<Recipe>();
		recipe.levels.push_back("pizza-arena");
		recipe.levels.push_back("dessert-arena");
		//TODO add the victory and defeat screens


		//For now start with the firstRecipe as the currentRecipeEntity but in the future it should start as nothing and we will start in the main menu which will select a recipe
		currentRecipeEntity = firstRecipe;

}


bool GameStateSystem::inGameState() {
		//TODO if we add more states that the game can be in add them here if they are relevant.
		return !isInHelpScreen && !isInMainScreen;
}

void GameStateSystem::nextMap()
{
		currentLevelIndex++;
		assert(currentRecipeEntity.has<Recipe>());
		auto& currentRecipe = currentRecipeEntity.get<Recipe>();
		if (currentLevelIndex < currentRecipe.levels.size()) {
				//Read the next level from the recipe
				LevelLoader lc;
				currentLevel = lc.readLevel(currentRecipe.levels[currentLevelIndex]);
				EventSystem<LoadLevelEvent>::instance().sendEvent(LoadLevelEvent{});
		}
}

void GameStateSystem::restartMap()
{
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
		//TODO fill in
		isInMainScreen = true;
		Camera::createCamera(vec2{ 0.0f,0.0f });

		int frameBufferWidth, frameBufferHeight;
		glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

		StartMenu::createStartMenu(frameBufferWidth, frameBufferHeight);
}

void GameStateSystem::setWindow(GLFWwindow* window)
{
		this->window = window;
}
