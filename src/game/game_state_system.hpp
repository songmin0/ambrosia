#include "common.hpp"
#include "event_system.hpp"
#include "events.hpp"
#include "entities/tiny_ecs.hpp"

#include "../ext/nlohmann/json.hpp"

using json = nlohmann::json;

class GameStateSystem {
private:
	GameStateSystem();
	~GameStateSystem();
	void resetState();

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

	json getSkillsForAllPlayers();

	void loadSave();
	void loadRecipe(const std::string& recipeName, json skill_levels = "", int level = 0,
									int ambrosia = 0, bool isInTutorial = false);
	void launchMainMenu();
	void launchAchievementsScreen();
	void launchCreditsScreen();
	void launchRecipeSelectMenu();
	void launchVictoryScreen();
	void launchDefeatScreen();
	void launchShopScreen();

	const vec2 getScreenBufferSize();
	void setWindow(GLFWwindow* window);
	void preloadResources();

	inline int getAmbrosia() const {return ambrosia;}
	void setAmbrosia(int amt);
	void createAmbrosiaUI();

private:
	////////////////////////
	// Player entities
	////////////////////////
	// Creates new player entities at beginning of recipe
	void createPlayerEntities(json levels = "");
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
	void createMap();
	void createMobs();
	void createButtons(int frameBufferWidth, int frameBufferHeight);
	void createEffects();
	

	void onDepositAmbrosiaEvent(const DepositAmbrosiaEvent& event);

public:
	bool isInMainScreen;
	bool isInAchievementsScreen;
	bool isInCreditsScreen;
	bool isInStory;
	bool isInTutorial;
	bool isInHelpScreen;
	bool isInVictoryScreen;
	bool isInDefeatScreen;
	bool isInShopScreen;
	bool isTransitioning;
	bool isInspecting;
	json currentLevel;
	json recipe;
	int currentLevelIndex;
	int currentTutorialIndex;
	int currentStoryIndex;

private:
	int ambrosia;
	EventListenerInfo depositAmbrosiaListener;

	ECS::Entity playerRaoul;
	ECS::Entity playerTaji;
	ECS::Entity playerEmber;
	ECS::Entity playerChia;

	GLFWwindow* window;
};