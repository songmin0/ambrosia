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
	: ambrosia(0)
{
	resetState();
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

	depositAmbrosiaListener = EventSystem<DepositAmbrosiaEvent>::instance().registerListener(
			std::bind(&GameStateSystem::onDepositAmbrosiaEvent, this, std::placeholders::_1));
}

GameStateSystem::~GameStateSystem()
{
	if (depositAmbrosiaListener.isValid())
	{
		EventSystem<DepositAmbrosiaEvent>::instance().unregisterListener(depositAmbrosiaListener);
	}
}

void GameStateSystem::resetState()
{
	isInMainScreen = false;
	isInAchievementsScreen = false;
	isInShopScreen = false;
	isInCreditsScreen = false;
	isInStory = false;
	isInTutorial = false;
	isInHelpScreen = false;
	isInVictoryScreen = false;
	isInDefeatScreen = false;
	isInspecting = false;

	EventSystem<DeleteAllEmittersEvent>::instance().sendEvent(DeleteAllEmittersEvent{});
	EventSystem<ResetMouseCursorEvent>::instance().sendEvent({});
}

bool GameStateSystem::inGameState()
{
	return !isInMainScreen && !isInVictoryScreen && !isInDefeatScreen &&
		  !isInStory && !isInAchievementsScreen && !isInCreditsScreen && !isInShopScreen;
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

	resetState();
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

	resetState();
	isInTutorial = true;

	currentTutorialIndex = 0;
	loadRecipe("tutorial", "", 0, 0, isInTutorial);
	EventSystem<StartTutorialEvent>::instance().sendEvent(StartTutorialEvent{});
}

void GameStateSystem::nextMap()
{
	std::cout << "GameStateSystem::nextMap: attempting to start next map in current recipe" << std::endl;

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
	resetState();
	currentLevel = recipe["maps"][currentLevelIndex];

	std::cout << "GameStateSystem::restartMap: starting " << currentLevel.at("map") << std::endl;

	// Get screen/buffer size
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	// Create all entities except for the players
	removeNonPlayerEntities();
	createNonPlayerEntities();
	createMap();

	// Get the players ready for the new map
	preparePlayersForNextMap();
}

void GameStateSystem::save()
{
	std::cout << "GameStateSystem::save: saving " << recipe["name"]
						<< ", level " << currentLevelIndex << std::endl;

	LevelLoader lc;
	std::list<Achievement> achievements = AchievementSystem::instance().getAchievements();

	json skill_levels = getSkillsForAllPlayers();
	std::cout << skill_levels << std::endl;

	lc.save(recipe["name"], currentLevelIndex, ambrosia, achievements, skill_levels);
}

json GameStateSystem::getSkillsForAllPlayers()
{
	json skill_levels;
	skill_levels["raoul"]["level"] = (int)playerRaoul.get<StatsComponent>().getStatValue(StatType::LEVEL);
	skill_levels["raoul"]["skills"] = playerRaoul.get<SkillComponent>().getAllSkillLevels();

	skill_levels["chia"]["level"] = (int)playerChia.get<StatsComponent>().getStatValue(StatType::LEVEL);
	skill_levels["chia"]["skills"] = playerChia.get<SkillComponent>().getAllSkillLevels();

	skill_levels["ember"]["level"] = (int)playerEmber.get<StatsComponent>().getStatValue(StatType::LEVEL);
	skill_levels["ember"]["skills"] = playerEmber.get<SkillComponent>().getAllSkillLevels();

	skill_levels["taji"]["level"] = (int)playerTaji.get<StatsComponent>().getStatValue(StatType::LEVEL);
	skill_levels["taji"]["skills"] = playerTaji.get<SkillComponent>().getAllSkillLevels();
	
	return skill_levels;
}

void GameStateSystem::loadSave()
{
	std::cout << "GameStateSystem::loadSave: attempting to load a saved game..." << std::endl;

	LevelLoader lc;
	json save_obj = lc.load();
	if (save_obj.contains("recipe"))
	{
		std::cout << "GameStateSystem::loadSave: a saved game was found" << std::endl;
		loadRecipe(save_obj["recipe"], save_obj["skill_levels"], save_obj["level"], save_obj["ambrosia"]);
	}
	else
	{
		std::cout << "GameStateSystem::loadSave: no saved game found" << std::endl;
		beginStory();
	}
}


void GameStateSystem::loadRecipe(const std::string& recipeName, json skill_levels, int level,
																 int ambrosia, bool isInTutorial)
{
	std::cout << "GameStateSystem::loadRecipe: loading " << recipeName
						<< ", level " << level << std::endl;

	resetState();
	this->ambrosia = ambrosia;
	this->isInTutorial = isInTutorial;
	currentLevelIndex = level;

	LevelLoader lc;
	recipe = lc.readLevel(recipeName);
	currentLevel = recipe["maps"][currentLevelIndex];

	// Get rid of all entities and create new ones
	std::cout << skill_levels << std::endl;
	removeNonPlayerEntities();
	removePlayerEntities();
	createPlayerEntities(skill_levels);
	createNonPlayerEntities();
	createMap();

	save();
}

void GameStateSystem::launchMainMenu()
{
	std::cout << "GameStateSystem::launchMainMenu: creating main menu" << std::endl;

	resetState();
	isInMainScreen = true;

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	removePlayerEntities();
	MouseClickFX::createMouseClickFX();
	vec2 screenBufferSize = getScreenBufferSize();
	StartMenu::createStartMenu(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchAchievementsScreen()
{
	std::cout << "GameStateSystem::launchAchievementsScreen: creating achievements screen" << std::endl;

	resetState();
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

	resetState();
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

	resetState();
	isInMainScreen = true;

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	removePlayerEntities();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createRecipeSelectScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchVictoryScreen()
{
	std::cout << "GameStateSystem::launchVictoryScreen: creating victory screen" << std::endl;

	resetState();
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
}



void GameStateSystem::launchDefeatScreen()
{
	std::cout << "GameStateSystem::launchDefeatScreen: creating defeat screen" << std::endl;

	resetState();
	isInDefeatScreen = true;

	save();

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	hidePlayers();
	vec2 screenBufferSize = getScreenBufferSize();
	Screens::createDefeatScreen(screenBufferSize.x, screenBufferSize.y);
}

void GameStateSystem::launchShopScreen()
{

	if (currentLevelIndex == recipe["maps"].size() - 1) {
		std::cout << "Last map, don't launch shop!" << std::endl;
		nextMap();
		return;
	}

	std::cout << "GameStateSystem::launchShopScreen: creating shop screen" << std::endl;

	resetState();
	isInShopScreen = true;

	save();

	Camera::createCamera(vec2(0.f));
	removeNonPlayerEntities();
	hidePlayers();
	vec2 screenBufferSize = getScreenBufferSize();
	MouseClickFX::createMouseClickFX();
	Screens::createShopScreen(screenBufferSize.x, screenBufferSize.y, playerRaoul, playerChia, playerEmber, playerTaji);
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

void GameStateSystem::setAmbrosia(int amt)
{
	ambrosia = std::max(0, amt);

	assert(ECS::registry<AmbrosiaDisplay>.entities.size() == 1);
	auto entity = ECS::registry<AmbrosiaDisplay>.entities.front();
	auto& ambDisplay = entity.get<AmbrosiaDisplay>();

	assert(entity.has<Motion>());
	vec2 position = entity.get<Motion>().position + ambDisplay.textOffset;

	std::string str = std::to_string(ambrosia);

	entity.remove<Text>();
	addText(entity, str, position, ambDisplay.textScale, AMBROSIA_COLOUR);
}

void GameStateSystem::createPlayerEntities(json player_levels)
{
	std::cout << "GameStateSystem::createPlayerEntities: creating new player entities" << std::endl;
	playerRaoul = Player::create(PlayerType::RAOUL, currentLevel.at("raoul"));
	playerTaji = Player::create(PlayerType::TAJI, currentLevel.at("taji"));
	playerEmber = Player::create(PlayerType::EMBER, currentLevel.at("ember"));
	playerChia = Player::create(PlayerType::CHIA, currentLevel.at("chia"));

	if (player_levels != "") {
		playerRaoul.get<SkillComponent>().setAllSkillLevels(player_levels["raoul"]["skills"]);
		playerRaoul.get<StatsComponent>().setLevel(player_levels["raoul"]["level"]);

		playerTaji.get<SkillComponent>().setAllSkillLevels(player_levels["taji"]["skills"]);
		playerTaji.get<StatsComponent>().setLevel(player_levels["taji"]["level"]);

		playerEmber.get<SkillComponent>().setAllSkillLevels(player_levels["ember"]["skills"]);
		playerEmber.get<StatsComponent>().setLevel(player_levels["ember"]["level"]);

		playerChia.get<SkillComponent>().setAllSkillLevels(player_levels["chia"]["skills"]);
		playerChia.get<StatsComponent>().setLevel(player_levels["chia"]["level"]);
	}
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
	std::cout << "GameStateSystem::preparePlayersForNextMap: refilling players' HP, removing buffs/debuffs, setting skill levels, etc." << std::endl;

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
	createMobs();
	createButtons(frameBufferWidth, frameBufferHeight);
	createEffects();
	createAmbrosiaUI();
	setAmbrosia(ambrosia);
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

void GameStateSystem::createMap()
{
	std::string mapName = currentLevel.at("map");

	std::cout << "GameStateSystem::createMap: creating the " << mapName << " map" << std::endl;

	// Create the map
	MapComponent::createMap(mapName, getScreenBufferSize());

	// Create a deforming blob for pizza arena
	// maybe add own section in level file we have more of these
	if (mapName == "pizza-arena")
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::PIZZA_ARENA});

		CheeseBlob::createCheeseBlob(vec2(700.f, 950.f));
	}
	else if (mapName == "dessert-arena")
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::DESSERT_ARENA});

		DessertForeground::createDessertForeground(vec2(2291.f, 772.f));
		DessertBackground::createDessertBackground(vec2(1920.f, 672.f));
		EventSystem<AddEmitterEvent>::instance().sendEvent(
				AddEmitterEvent{ "pinkCottonCandy",std::make_shared<BasicEmitter>(BasicEmitter(5)) });
		EventSystem<AddEmitterEvent>::instance().sendEvent(
				AddEmitterEvent{ "blueCottonCandy", std::make_shared<BlueCottonCandyEmitter>(BlueCottonCandyEmitter(5)) });
	}
	else if (mapName == "veggie-forest")
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::PLACEHOLDER3});
	}
	else if (mapName == "bbq")
	{
		EventSystem<PlayMusicEvent>::instance().sendEvent({MusicType::BOSS});

		BBQBackground::createBBQBackground(vec2(960.f, 720.f));
		BBQFire::createBBQFire(vec2(2153.f, 1015.f));
		BBQFire::createBBQFire(vec2(1378.f, 1403.f));
		BBQFire::createBBQFire(vec2(41.f, 1264.f));
		BBQFire::createBBQFire(vec2(2137.f, 222.f));
		BBQFire::createBBQFire(vec2(954.f, 716.f), RenderLayer::MAP2);
		BBQFire::createBBQFire(vec2(1621.f, 1228.f), RenderLayer::MAP2);
		BBQFire::createBBQFire(vec2(1406.f, 285.f), RenderLayer::MAP2);
		BBQFire::createBBQFire(vec2(243.f, 326.f), RenderLayer::MAP2);
		BBQFire::createBBQFire(vec2(943.f, 1227.f), RenderLayer::MAP2);
		BBQFire::createBBQFire(vec2(358, 1079.f), RenderLayer::MAP2);
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

	HelpButton::createHelpButton(vec2(frameBufferWidth - 90.f, 30.f));
	InspectButton::createInspectButton(vec2(frameBufferWidth - 80.f, 90.f));
}

void GameStateSystem::createEffects()
{
	MouseClickFX::createMouseClickFX();
	ActiveSkillFX::createActiveSkillFX();
}

void GameStateSystem::createAmbrosiaUI()
{
	AmbrosiaDisplay::createAmbrosiaDisplay();
}

void GameStateSystem::onDepositAmbrosiaEvent(const DepositAmbrosiaEvent& event)
{
	setAmbrosia(getAmbrosia() + event.amount);
}
