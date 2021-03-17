#include <vector>
#include <string>
#include "entities/tiny_ecs.hpp"
#include "../ext/nlohmann/json.hpp"
#include "event_system.hpp"
#include "events.hpp"

using json = nlohmann::json;


struct Recipe {
	//TODO what should the type be for these
	std::vector<std::string> levels;
	std::vector<std::string> victoryScreens;
	std::vector<std::string> defeatScreens;
};


class GameStateSystem {
	//TODO should I move all the music logic from the WorldSystem to here?
public:
	// Returns the singleton instance of this system
	static GameStateSystem& instance() {
		static GameStateSystem gameStateSystem;
		return gameStateSystem;
	}

	bool inGameState();
	bool isInTutorial;
	bool hasDoneTutorial;
	bool isInMainScreen;
	bool isInHelpScreen;
	json currentLevel;
	json recipe;
	int currentLevelIndex; //THIS SHOULD NOT BE PUBLIC but for now this easily lets me debug change maps
	int currentTutorialIndex;

	//Load the next map in the current recipe
	void nextMap();
	//If the players lose restart the current map
	void restartMap();
	//TODO should loading a save be called from here?
	void loadSave();
	//TODO should save be called from here
	void save();
	//This will call the victory screen for the current map
	void launchVictoryScreen();
	//This will call the defeat screen for the current map
	void launchDefeatScreen();
	//This will call the main menu
	void launchMainMenu();

	void setWindow(GLFWwindow* window);

private:	
	GameStateSystem();
		
	//ECS::Entity currentRecipeEntity;
	GLFWwindow* window;
};