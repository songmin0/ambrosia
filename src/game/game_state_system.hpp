#include "common.hpp"
#include "event_system.hpp"
#include "events.hpp"
#include "entities/tiny_ecs.hpp"

#include "../ext/nlohmann/json.hpp"

using json = nlohmann::json;

class GameStateSystem {
private:
	GameStateSystem();
	void resetFlags();

public:
	// Returns the singleton instance of this system
	static GameStateSystem& instance() {
		static GameStateSystem gameStateSystem;
		return gameStateSystem;
	}

	bool inGameState();
	bool hasLights();

	void beginStory();
	void beginTutorial();
	void nextMap();
	void restartMap();

	void save();
	void loadSave();
	void loadRecipe(const std::string& recipeName, int level = 0,
									bool isInTutorial = false);

	void launchMainMenu();
	void launchAchievementsScreen();
	void launchCreditsScreen();
	void launchRecipeSelectMenu();
	void launchVictoryScreen();
	void launchDefeatScreen();

	const vec2 getScreenBufferSize();
	void setWindow(GLFWwindow* window);
	void preloadResources();

private:
	////////////////////////
	// Player entities
	////////////////////////
	// Creates new player entities at beginning of recipe
	void createPlayerEntities();
	// Completely removes player entities (e.g., when going back to main menu)
	void removePlayerEntities();
	// Disables player rendering (e.g., when going to victory/defeat/shop screens)
	void hidePlayers();
	// Enables player rendering, fills HP to maximum, removes buffs/debuffs, etc.
	void preparePlayersForNextMap();

	////////////////////////
	// Non-player entities
	////////////////////////
	// These entities don't persist between levels, so the functions are straightforward
	void createNonPlayerEntities();
	void removeNonPlayerEntities();
	void createMap(int frameBufferWidth, int frameBufferHeight);
	void createMobs();
	void createButtons(int frameBufferWidth, int frameBufferHeight);
	void createEffects();

public:
	bool isInMainScreen;
	bool isInAchievementsScreen;
	bool isInCreditsScreen;
	bool isInStory;
	bool isInTutorial;
	bool isInHelpScreen;
	bool isInVictoryScreen;
	bool isInDefeatScreen;
	bool isTransitioning;
	json currentLevel;
	json recipe;
	int currentLevelIndex;
	int currentTutorialIndex;
	int currentStoryIndex;

private:
	ECS::Entity playerRaoul;
	ECS::Entity playerTaji;
	ECS::Entity playerEmber;
	ECS::Entity playerChia;

	GLFWwindow* window;
};