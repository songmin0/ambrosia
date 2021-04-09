#include "game_state_system.hpp"
#include "turn_system.hpp"
#include "camera.hpp"
#include "achievement_system.hpp"

#include "ui/menus.hpp"
#include "maps/map_objects.hpp"
#include "level_loader/level_loader.hpp"
#include "rendering/text.hpp"
#include "entities/players.hpp"
#include "entities/enemies.hpp"

#include <sstream>
#include <iostream>

GameStateSystem::GameStateSystem()
{
	resetFlags();
	isTransitioning = false;

	currentLevelIndex = 0;
	currentTutorialIndex = 0;
	currentStoryIndex = 0;

	LevelLoader lc;
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

void GameStateSystem::resetFlags()
{
	isInMainScreen = false;
	isInAchievementsScreen = false;
	isInCreditsScreen = false;
	isInStory = false;
	isInTutorial = false;
	isInHelpScreen = false;
	isInVictoryScreen = false;
	isInDefeatScreen = false;
}

bool GameStateSystem::inGameState()
{
	return !isInMainScreen && !isInVictoryScreen && !isInDefeatScreen &&
				 !isInStory && !isInAchievementsScreen && !isInCreditsScreen;
}

bool GameStateSystem::hasLights()
{
	bool isVeggieForest = (currentLevel["map"] == "veggie-forest");
	bool isSaladCanyon = (currentLevel["map"] == "salad-canyon");
	return inGameState() && (isVeggieForest || isSaladCanyon);
}

void GameStateSystem::beginStory()
{
	std::cout << "GameStateSystem::beginStory: attempting to start the story..." << std::endl;

	resetFlags();
	isInStory = true;

	removeNonPlayerEntities();
	removePlayerEntities();

	currentStoryIndex = 0;
	EventSystem<AdvanceStoryEvent>::instance().sendEvent(AdvanceStoryEvent{});
	createText("Press space to skip.", { 50.0, 50.0 }, 0.3f);
}

void GameStateSystem::beginTutorial()
{
	std::cout << "GameStateSystem::beginTutorial: attempting to load/start tutorial..." << std::endl;

	resetFlags();
	isInTutorial = true;

	currentTutorialIndex = 0;
	loadRecipe("tutorial", 0, isInTutorial);
	EventSystem<StartTutorialEvent>::instance().sendEvent(StartTutorialEvent{});
}

void GameStateSystem::nextMap()
{
	std::cout << "GameStateSystem::nextMap: attempting to start next map in current recipe" << std::endl;

	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});

	currentLevelIndex++;
	if (currentLevelIndex == recipe["maps"].size() - 1)
	{
		EventSystem<ReachedBossEvent>::instance().sendEvent(ReachedBossEvent{});
		std::cout << "ReachedBossEvent\n";
	}

	if (currentLevelIndex < recipe["maps"].size())
	{
		std::cout << "GameStateSystem::nextMap: found next map in recipe"<< std::endl;

		// Advance to the next level in the recipe
		save();
		restartMap();
	}
	else
	{
		std::cout << "GameStateSystem::nextMap: no maps left in current recipe; going back to main menu"<< std::endl;

		//TODO handle finishing the recipe here but for now return to main menu
		currentLevelIndex = 0;
		currentLevel = recipe["maps"][currentLevelIndex];
		launchMainMenu();
	}
}

void GameStateSystem::restartMap()
{
	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});

	resetFlags();
	currentLevel = recipe["maps"][currentLevelIndex];

	std::cout << "GameStateSystem::restartMap: starting " << currentLevel.at("map") << std::endl;

	// Get screen/buffer size
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	// Create all entities except for the players
	removeNonPlayerEntities();
	createNonPlayerEntities();

	// Get the players ready for the new map
	preparePlayersForNextMap();

	// Make sure the mouse cursor is just the normal one
	EventSystem<ResetMouseCursorEvent>::instance().sendEvent({});
}

void GameStateSystem::save()
{
	std::cout << "GameStateSystem::save: saving " << recipe["name"]
						<< ", level " << currentLevelIndex << std::endl;

	LevelLoader lc;
	std::list<Achievement> achievements = AchievementSystem::instance().getAchievements();
	lc.save(recipe["name"], currentLevelIndex, achievements);
}

void GameStateSystem::loadSave()
{
	std::cout << "GameStateSystem::loadSave: attempting to load a saved game..." << std::endl;

	LevelLoader lc;
	json save_obj = lc.load();
	if (save_obj.contains("recipe"))
	{
		std::cout << "GameStateSystem::loadSave: a saved game was found" << std::endl;
		loadRecipe(save_obj["recipe"], save_obj["level"]);
	}
	else
	{
		std::cout << "GameStateSystem::loadSave: no saved game found" << std::endl;
		beginStory();
	}
}

void GameStateSystem::loadRecipe(const std::string& recipeName, int level,
																 bool isInTutorial)
{
	std::cout << "GameStateSystem::loadRecipe: loading " << recipeName
						<< ", level " << level << std::endl;

	resetFlags();
	this->isInTutorial = isInTutorial;
	currentLevelIndex = level;

	LevelLoader lc;
	recipe = lc.readLevel(recipeName);
	currentLevel = recipe["maps"][currentLevelIndex];

	// Get rid of all entities and create new ones
	removeNonPlayerEntities();
	removePlayerEntities();
	createPlayerEntities();
	createNonPlayerEntities();

	save();
}

void GameStateSystem::launchMainMenu()
{
	std::cout << "GameStateSystem::launchMainMenu: creating main menu" << std::endl;

	resetFlags();
	isInMainScreen = true;

	Camera::createCamera(vec2(0.f));
	MouseClickFX::createMouseClickFX();
	removeNonPlayerEntities();
	removePlayerEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	StartMenu::createStartMenu(screenBufferSize.x, screenBufferSize.y);

	EventSystem<ResetMouseCursorEvent>::instance().sendEvent({});
}

void GameStateSystem::launchAchievementsScreen()
{
	std::cout << "GameStateSystem::launchAchievementsScreen: creating achievements screen" << std::endl;

	resetFlags();
	isInAchievementsScreen = true;

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	removePlayerEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createAchievementsScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchCreditsScreen()
{
	std::cout << "GameStateSystem::launchCreditsScreen: creating credits screen" << std::endl;

	resetFlags();
	isInCreditsScreen = true;

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	removePlayerEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createCreditsScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchRecipeSelectMenu()
{
	std::cout << "GameStateSystem::launchRecipeSelectMenu: creating recipe selection menu" << std::endl;

	resetFlags();
	isInMainScreen = true;

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	removePlayerEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createRecipeSelectScreen(screenBufferSize.x, screenBufferSize.y);

	EventSystem<ResetMouseCursorEvent>::instance().sendEvent({});
}

void GameStateSystem::launchVictoryScreen()
{
	std::cout << "GameStateSystem::launchVictoryScreen: creating victory screen" << std::endl;

	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});

	resetFlags();
	isInVictoryScreen = true;

	if (recipe["name"] != "tutorial")
	{
		EventSystem<BeatLevelEvent>::instance().sendEvent(BeatLevelEvent{});
		// Defeated the boss
		if (currentLevelIndex == recipe["maps"].size() - 1)
			EventSystem<DefeatedBossEvent>::instance().sendEvent(DefeatedBossEvent{});
	}

	save();

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	hidePlayers();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createVictoryScreen(screenBufferSize.x, screenBufferSize.y);

	EventSystem<ResetMouseCursorEvent>::instance().sendEvent({});
}

void GameStateSystem::launchDefeatScreen()
{
	std::cout << "GameStateSystem::launchDefeatScreen: creating defeat screen" << std::endl;

	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});

	resetFlags();
	isInDefeatScreen = true;

	save();

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	hidePlayers();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createDefeatScreen(screenBufferSize.x, screenBufferSize.y);

	EventSystem<ResetMouseCursorEvent>::instance().sendEvent({});
}

const vec2 GameStateSystem::getScreenBufferSize()
{
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	return vec2(frameBufferWidth, frameBufferHeight);
}

void GameStateSystem::setWindow(GLFWwindow* window)
{
	this->window = window;
}

void GameStateSystem::preloadResources()
{
	ECS::ContainerInterface::listAllComponents();
	std::cout << "Preloading...\n";

	LevelLoader lc;
	recipe = lc.readLevel("tutorial");
	currentLevel = recipe["maps"][0];

	createPlayerEntities();
	createNonPlayerEntities();

	std::cout << "Preload complete. Unloading...\n";
	removePlayerEntities();
	removeNonPlayerEntities();

	ECS::ContainerInterface::listAllComponents();
	std::cout << "Unload complete.\n";
}

void GameStateSystem::createPlayerEntities()
{
	std::cout << "GameStateSystem::createPlayerEntities: creating new player entities" << std::endl;

	playerRaoul = Player::create(PlayerType::RAOUL, currentLevel.at("raoul"));
	playerTaji = Player::create(PlayerType::TAJI, currentLevel.at("taji"));
	playerEmber = Player::create(PlayerType::EMBER, currentLevel.at("ember"));
	playerChia = Player::create(PlayerType::CHIA, currentLevel.at("chia"));
}

void GameStateSystem::removePlayerEntities()
{
	std::cout << "GameStateSystem::removePlayerEntities: completely removing player entities" << std::endl;

	while (!ECS::registry<PlayerComponent>.entities.empty())
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<PlayerComponent>.entities.back());
}

void GameStateSystem::hidePlayers()
{
	std::cout << "GameStateSystem::hidePlayers: disabling rendering for player entities" << std::endl;

	Player::disableRendering(playerRaoul);
	Player::disableRendering(playerTaji);
	Player::disableRendering(playerEmber);
	Player::disableRendering(playerChia);
}

void GameStateSystem::preparePlayersForNextMap()
{
	std::cout << "GameStateSystem::preparePlayersForNextMap: refilling players' HP, removing buffs/debuffs, etc." << std::endl;

	Player::prepareForNextMap(playerRaoul, currentLevel.at("raoul"));
	Player::prepareForNextMap(playerTaji, currentLevel.at("taji"));
	Player::prepareForNextMap(playerEmber, currentLevel.at("ember"));
	Player::prepareForNextMap(playerChia, currentLevel.at("chia"));
}

void GameStateSystem::createNonPlayerEntities()
{
	std::cout << "GameStateSystem::createNonPlayerEntities: creating the non-player entities for current map" << std::endl;

	// Get screen/buffer size
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	Camera::createCamera(currentLevel.at("camera"));
	createMap(frameBufferWidth, frameBufferHeight);
	createMobs();
	createButtons(frameBufferWidth, frameBufferHeight);
	createEffects();
}

void GameStateSystem::removeNonPlayerEntities()
{
	std::cout << "GameStateSystem::removeNonPlayerEntities: completely removing all non-player entities" << std::endl;

	auto removeEntities = [=](std::vector<ECS::Entity>& entities)
	{
		for (int i = entities.size() - 1; i >= 0; i--)
		{
			auto entity = entities[i];
			if (!entity.has<PlayerComponent>() && !(entity.has<AchievementPopup>() && isInVictoryScreen))
			{
				ECS::ContainerInterface::removeAllComponentsOf(entity);
			}
		}
	};

	removeEntities(ECS::registry<Motion>.entities);
	removeEntities(ECS::registry<Text>.entities);
}

void GameStateSystem::createMap(int frameBufferWidth, int frameBufferHeight)
{
	std::string mapName = currentLevel.at("map");

	std::cout << "GameStateSystem::createMap: creating the " << mapName << " map" << std::endl;

	// Create the map
	MapComponent::createMap(mapName, { frameBufferWidth, frameBufferHeight });

	// Create a deforming blob for pizza arena
	// maybe add own section in level file we have more of these
	if (mapName == "pizza-arena")
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::PIZZA_ARENA});

		CheeseBlob::createCheeseBlob({ 700, 950 });
	}
	else if (mapName == "dessert-arena")
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::DESSERT_ARENA});

		DessertForeground::createDessertForeground({ 1920, 672 });
		EventSystem<AddEmitterEvent>::instance().sendEvent(
				AddEmitterEvent{ "pinkCottonCandy",std::make_shared<BasicEmitter>(BasicEmitter(5)) });
		EventSystem<AddEmitterEvent>::instance().sendEvent(
				AddEmitterEvent{ "blueCottonCandy", std::make_shared<BlueCottonCandyEmitter>(BlueCottonCandyEmitter(5)) });
	}
	else if (mapName == "veggie-forest")
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::PLACEHOLDER3});
	}
	else
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::PLACEHOLDER1});
	}
}

void GameStateSystem::createMobs()
{
	createEnemies(currentLevel.at("mobs"));
}

void GameStateSystem::createButtons(int frameBufferWidth, int frameBufferHeight)
{
	// Create UI buttons
	Button::createPlayerButton(PlayerType::RAOUL, { frameBufferWidth / 2.f - 300, 60 },
														 []() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::RAOUL }); });

	Button::createPlayerButton(PlayerType::TAJI, { frameBufferWidth / 2.f - 100, 60 },
														 []() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::TAJI }); });

	Button::createPlayerButton(PlayerType::EMBER, { frameBufferWidth / 2.f + 100, 60 },
														 []() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::EMBER }); });

	Button::createPlayerButton(PlayerType::CHIA, { frameBufferWidth / 2.f + 300, 60 },
														 []() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::CHIA }); });

	SkillButton::createMoveButton({ 100, frameBufferHeight - 80 }, "skill_buttons/skill_generic_move",
																[]() {
																	std::cout << "Move button clicked!" << std::endl;
																	if (!ECS::registry<TurnSystem::TurnComponentIsActive>.entities.empty())
																	{
																		auto activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities.front();
																		if (!activeEntity.has<PlayerComponent>())
																		{
																			return;
																		}

																		auto& turnComponent = activeEntity.get<TurnSystem::TurnComponent>();
																		if (turnComponent.canStartMoving())
																		{
																			turnComponent.activeAction = SkillType::MOVE;
																			SetActiveSkillEvent event;
																			event.entity = activeEntity;
																			event.type = SkillType::MOVE;
																			EventSystem<SetActiveSkillEvent>::instance().sendEvent(event);


																			if (GameStateSystem::instance().isInTutorial && GameStateSystem::instance().currentTutorialIndex == 3)
																			{
																				EventSystem<AdvanceTutorialEvent>::instance().sendEvent(AdvanceTutorialEvent{});
																			}
																		}
																	}
																});

	SkillButton::createSkillButton({ 250, frameBufferHeight - 80 }, PlayerType::RAOUL, SkillType::SKILL1, "skill1",
																 []() {
																	 std::cout << "Skill one button clicked!" << std::endl;

																	 if (!ECS::registry<TurnSystem::TurnComponentIsActive>.entities.empty())
																	 {
																		 auto activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities.front();
																		 if (!activeEntity.has<PlayerComponent>())
																		 {
																			 return;
																		 }

																		 auto& turnComponent = activeEntity.get<TurnSystem::TurnComponent>();
																		 if (turnComponent.canStartSkill())
																		 {
																			 turnComponent.activeAction = SkillType::SKILL1;
																			 SetActiveSkillEvent event;
																			 event.entity = activeEntity;
																			 event.type = SkillType::SKILL1;

																			 EventSystem<SetActiveSkillEvent>::instance().sendEvent(event);

																			 if (GameStateSystem::instance().isInTutorial && GameStateSystem::instance().currentTutorialIndex == 5)
																			 {
																				 EventSystem<AdvanceTutorialEvent>::instance().sendEvent(AdvanceTutorialEvent{});
																			 }
																		 }
																	 }
																 });

	SkillButton::createSkillButton({ 400, frameBufferHeight - 80 }, PlayerType::RAOUL, SkillType::SKILL2, "skill2",
																 []() {
																	 std::cout << "Skill two button clicked!" << std::endl;

																	 if (!ECS::registry<TurnSystem::TurnComponentIsActive>.entities.empty())
																	 {
																		 auto activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities.front();
																		 if (!activeEntity.has<PlayerComponent>())
																		 {
																			 return;
																		 }

																		 auto& turnComponent = activeEntity.get<TurnSystem::TurnComponent>();
																		 if (turnComponent.canStartSkill())
																		 {
																			 turnComponent.activeAction = SkillType::SKILL2;
																			 SetActiveSkillEvent event;
																			 event.entity = activeEntity;
																			 event.type = SkillType::SKILL2;

																			 EventSystem<SetActiveSkillEvent>::instance().sendEvent(event);
																		 }
																	 }
																 });

	SkillButton::createSkillButton({ 550, frameBufferHeight - 80 }, PlayerType::RAOUL, SkillType::SKILL3, "skill3",
																 []() {
																	 std::cout << "Skill three button clicked!" << std::endl;

																	 if (!ECS::registry<TurnSystem::TurnComponentIsActive>.entities.empty())
																	 {
																		 auto activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities.front();
																		 if (!activeEntity.has<PlayerComponent>())
																		 {
																			 return;
																		 }

																		 auto& turnComponent = activeEntity.get<TurnSystem::TurnComponent>();
																		 if (turnComponent.canStartSkill())
																		 {
																			 turnComponent.activeAction = SkillType::SKILL3;
																			 SetActiveSkillEvent event;
																			 event.entity = activeEntity;
																			 event.type = SkillType::SKILL3;

																			 EventSystem<SetActiveSkillEvent>::instance().sendEvent(event);

																			 if (GameStateSystem::instance().isInTutorial && GameStateSystem::instance().currentTutorialIndex == 8)
																			 {
																				 EventSystem<AdvanceTutorialEvent>::instance().sendEvent(AdvanceTutorialEvent{});
																			 }
																		 }
																	 }
																 });

	ToolTip::createMoveToolTip({ 100, frameBufferHeight - 120 });
	ToolTip::createToolTip(PlayerType::RAOUL, SkillType::SKILL1, { 250, frameBufferHeight - 120 });
	ToolTip::createToolTip(PlayerType::RAOUL, SkillType::SKILL2, { 400, frameBufferHeight - 120 });
	ToolTip::createToolTip(PlayerType::RAOUL, SkillType::SKILL3, { 550, frameBufferHeight - 120 });

	HelpButton::createHelpButton(vec2(frameBufferWidth - 60.f, 30.f));
}

void GameStateSystem::createEffects()
{
	MouseClickFX::createMouseClickFX();
	ActiveSkillFX::createActiveSkillFX();
}
