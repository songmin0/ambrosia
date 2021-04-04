#include "world.hpp"
#include "camera.hpp"
#include "event_system.hpp"
#include "events.hpp"
#include "turn_system.hpp"

#include "physics/physics.hpp"
#include "physics/projectile.hpp"
#include "physics/debug.hpp"
#include "entities/raoul.hpp"
#include "entities/taji.hpp"
#include "entities/ember.hpp"
#include "entities/chia.hpp"
#include "entities/enemies.hpp"
#include "rendering/render_components.hpp"
#include "animation/animation_components.hpp"
#include "maps/map_objects.hpp"
#include "ui/button.hpp"
#include "ui/ui_system.hpp"
#include "ui/ui_entities.hpp"
#include "ui/menus.hpp"
#include "ai/ai.hpp"
#include "level_loader/level_loader.hpp"
#include "game_state_system.hpp"
#include "game/swarm_behaviour.hpp"
#include "rendering/text.hpp"


// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>
// Create the world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px) :
	shouldPlayAudioAtStartOfTurn(false)
{
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());

	///////////////////////////////////////
	// Initialize GLFW
	auto glfw_err_callback = [](int error, const char* desc) { std::cerr << "OpenGL:" << error << desc << std::endl; };
	glfwSetErrorCallback(glfw_err_callback);
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	//-------------------------------------------------------------------------
	// GLFW / OGL Initialization, needs to be set before glfwCreateWindow
	// Core Opengl 3.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_size_px.x, window_size_px.y, "Ambrosia", nullptr, nullptr);
	if (window == nullptr)
		throw std::runtime_error("Failed to glfwCreateWindow");

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto keyRedirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->onKey(_0, _1, _2, _3); };
	auto mouseClickRedirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->onMouseClick(_0, _1, _2); };
	glfwSetKeyCallback(window, keyRedirect);
	glfwSetMouseButtonCallback(window, mouseClickRedirect);

	auto mouseHoverRedirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->onMouseHover(_0, _1); };
	glfwSetCursorPosCallback(window, mouseHoverRedirect);

	initAudio();
	std::cout << "Loaded music\n";

	//Register the loadLevelEvent listener
	loadLevelListener = EventSystem<LoadLevelEvent>::instance().registerListener(
			std::bind(&WorldSystem::onLoadLevelEvent, this, std::placeholders::_1));

	transitionEventListener = EventSystem<TransitionEvent>::instance().registerListener(
		std::bind(&WorldSystem::onTransition, this, std::placeholders::_1));

	initCursors();
}

WorldSystem::~WorldSystem(){
	releaseAudio();
	releaseCursors();

	if (loadLevelListener.isValid())
	{
		EventSystem<LoadLevelEvent>::instance().unregisterListener(loadLevelListener);
	}

	if (transitionEventListener.isValid())
	{
		EventSystem<TransitionEvent>::instance().unregisterListener(transitionEventListener);
	} 

	// Destroy all created components
	ECS::ContainerInterface::clearAllComponents();

	// Close the window
	glfwDestroyWindow(window);
}

// Update our game world
void WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	processTimers(elapsed_ms);

	if (!GameStateSystem::instance().inGameState()) {
		return;
	}
	// Updating window title
	std::stringstream title_ss;
	title_ss << "Ambrosia";
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Check for player defeat
	assert(ECS::registry<ScreenState>.components.size() == 1);
	auto& screen = ECS::registry<ScreenState>.components[0];

	for (auto entity : ECS::registry<DeathTimer>.entities)
	{
		// Progress timer
		auto& counter = ECS::registry<DeathTimer>.get(entity);
		counter.counter_ms -= elapsed_ms;

		// Remove player/mob once death timer expires
		if (counter.counter_ms < 0)
		{

			// this has to go here, so the new chunks are added to mobs before the potato is removed
			if (ECS::registry<HasSwarmBehaviour>.has(entity))
			{
				SwarmBehaviour sb;
				sb.spawnExplodedChunks(entity);
			}

			//If the entity has a stats component get rid of the health bar too
			if (entity.has<StatsComponent>()) {
				ECS::ContainerInterface::removeAllComponentsOf(entity.get<StatsComponent>().healthBar);
			}
			ECS::ContainerInterface::removeAllComponentsOf(entity);
			// Check if there are no more players left, restart game
			if (ECS::registry<PlayerComponent>.entities.empty())
			{
				if (!GameStateSystem::instance().isTransitioning)
				{
					GameStateSystem::instance().isTransitioning = true;
					EventSystem<PlaySoundEffectEvent>::instance().sendEvent({ SoundEffect::GAME_OVER });
					TransitionEvent event;
					event.callback = []() {
						GameStateSystem::instance().launchDefeatScreen();
					};
					EventSystem<TransitionEvent>::instance().sendEvent(event);
					return;
				}
			}
		}
	}
	if (ECS::registry<AISystem::MobComponent>.entities.size() == 0) {
		if (!GameStateSystem::instance().isTransitioning)
		{
			GameStateSystem::instance().isTransitioning = true;
			TransitionEvent event;
			event.callback = []() {
				GameStateSystem::instance().launchVictoryScreen();
			};
			EventSystem<TransitionEvent>::instance().sendEvent(event);
		}
	}
}

void WorldSystem::processTimers(float elapsed_ms)
{
	for (auto& timer : ECS::registry<TimerComponent>.components)
	{
		if (timer.isCountingUp)
		{
			if (timer.counter_ms > timer.maxTime_ms)
			{
				timer.counter_ms = timer.maxTime_ms;
			}
			else if (timer.counter_ms < timer.maxTime_ms)
			{
				timer.counter_ms += elapsed_ms;
			}
		}
		else if (!timer.isCountingUp)
		{
			if (timer.counter_ms < 0.f)
			{
				timer.counter_ms = 0.f;
			}
			else if (timer.counter_ms > 0.f)
			{
				timer.counter_ms -= elapsed_ms;
			}
		}
	}

	// update screen state with its timer
	if (!ECS::registry<ScreenState>.entities.empty())
	{
		auto& screenEntity = ECS::registry<ScreenState>.entities.front();
		if (screenEntity.has<TimerComponent>())
		{
			auto& screenTimer = screenEntity.get<TimerComponent>();
			if (screenTimer.complete)
			{
				return;
			}

			assert(screenTimer.maxTime_ms > 0); // cannot divide by 0
			screenEntity.get<ScreenState>().darken_screen_factor = screenTimer.counter_ms / screenTimer.maxTime_ms;

			// start transition if timer is done
			if (screenTimer.isCountingUp && screenTimer.counter_ms >= screenTimer.maxTime_ms)
			{
				screenTimer.complete = true;
				transition();

				TransitionEvent event;
				event.callback = []() { return; };
				event.isFadingOut = false;
				EventSystem<TransitionEvent>::instance().sendEvent(event);
			}
			// finish transition
			else if (!screenTimer.isCountingUp && screenTimer.counter_ms <= 0.f)
			{
				screenTimer.complete = true;
				GameStateSystem::instance().isTransitioning = false;
				std::cout << "Transition complete" << std::endl;
			}
		}
	}
}

// Reset the world state to its initial state
void WorldSystem::restart()
{
	// Debugging for memory/component leaks
	ECS::ContainerInterface::listAllComponents();
	std::cout << "Restarting\n";

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (!ECS::registry<Motion>.entities.empty())
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<Motion>.entities.back());

	// Remove camera entity
	while (!ECS::registry<CameraComponent>.entities.empty()) {
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<CameraComponent>.entities.back());
	}

	while (!ECS::registry<Text>.entities.empty())
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<Text>.entities.back());

	// Debugging for memory/component leaks
	ECS::ContainerInterface::listAllComponents();

	// Get screen/buffer size
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
	
	Camera::createCamera(GameStateSystem::instance().currentLevel.at("camera"));
	createMap(frameBufferWidth, frameBufferHeight);
	createPlayers(frameBufferWidth, frameBufferHeight);
	createMobs(frameBufferWidth, frameBufferHeight);
	createButtons(frameBufferWidth, frameBufferHeight);
	createEffects(frameBufferWidth, frameBufferHeight);

	playAudio();
	shouldPlayAudioAtStartOfTurn = false;

	// On restart, go back to the normal mouse cursor
	glfwSetCursor(window, nullptr);
}

void WorldSystem::preloadResources()
{
	ECS::ContainerInterface::listAllComponents();
	std::cout << "Preloading... \n";

	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	createMap(frameBufferWidth, frameBufferHeight);
	createPlayers(frameBufferWidth, frameBufferHeight);
	createMobs(frameBufferWidth, frameBufferHeight);
	createButtons(frameBufferWidth, frameBufferHeight);
	createEffects(frameBufferWidth, frameBufferHeight);

	std::cout << "Preload complete. Unloading... \n";
	while (!ECS::registry<Motion>.entities.empty())
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<Motion>.entities.back());

	while (!ECS::registry<CameraComponent>.entities.empty()) {
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<CameraComponent>.entities.back());
	}

	ECS::ContainerInterface::listAllComponents();
	std::cout << "Unload complete. \n";
}

// Compute collisions between entities
void WorldSystem::handleCollisions()
{
	if (GameStateSystem::instance().inGameState()) 
	{
		// Loop over all collisions detected by the physics system
		auto& registry = ECS::registry<PhysicsSystem::Collision>;
		for (unsigned int i = 0; i < registry.components.size(); i++)
		{
			// The entity and its collider
			auto entity = registry.entities[i];
			auto entity_other = registry.components[i].other;

			// Check for projectiles colliding with the player or mobs
			if (ECS::registry<ProjectileComponent>.has(entity))
			{
				auto& projComponent = entity.get<ProjectileComponent>();
				projComponent.processCollision(entity_other);
			}
		}

		// Remove all collisions from this simulation step
		ECS::registry<PhysicsSystem::Collision>.clear();
	}
}

// Should the game be over ?
bool WorldSystem::isOver() const
{
	return glfwWindowShouldClose(window)>0;
}

void WorldSystem::createMap(int frameBufferWidth, int frameBufferHeight)
{
	// Create the map
	MapComponent::createMap(GameStateSystem::instance().currentLevel.at("map"), { frameBufferWidth, frameBufferHeight });

	// Create a deforming blob for pizza arena
	// maybe add own section in level file we have more of these
	if (GameStateSystem::instance().currentLevel.at("map") == "pizza-arena") {
		CheeseBlob::createCheeseBlob({ 700, 950 });
	}

	if (GameStateSystem::instance().currentLevel.at("map") == "dessert-arena") {
		DessertForeground::createDessertForeground({ 1920, 672 });
		EventSystem<AddEmitterEvent>::instance().sendEvent(AddEmitterEvent{ std::make_shared<BasicEmitter>(BasicEmitter(5)) });
		EventSystem<AddEmitterEvent>::instance().sendEvent(AddEmitterEvent{ std::make_shared<BlueCottonCandyEmitter>(BlueCottonCandyEmitter(5)) });
		
	}
}

void WorldSystem::createButtons(int frameBufferWidth, int frameBufferHeight)
{
	// Create UI buttons
	auto player_button_1 = Button::createPlayerButton(PlayerType::RAOUL, { frameBufferWidth / 2.f - 300, 60 },
		[]() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::RAOUL }); });

	auto player_button_2 = Button::createPlayerButton(PlayerType::TAJI, { frameBufferWidth / 2.f - 100, 60 },
		[]() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::TAJI }); });

	auto player_button_3 = Button::createPlayerButton(PlayerType::EMBER, { frameBufferWidth / 2.f + 100, 60 },
		[]() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::EMBER }); });

	auto player_button_4 = Button::createPlayerButton(PlayerType::CHIA, { frameBufferWidth / 2.f + 300, 60 },
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
		};
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

void WorldSystem::createEffects(int frameBufferWidth, int frameBufferHeight)
{
	MouseClickFX::createMouseClickFX();
	ActiveSkillFX::createActiveSkillFX();
}

void WorldSystem::createPlayers(int frameBufferWidth, int frameBufferHeight)
{ 
	// you can also createPlayers using the old method 
	// eg: playerRaoul = Raoul::createRaoul(vec2( 640, 512 ));
	// please specify vec2(x, y), as {x , y} is also valid json

	playerRaoul = Raoul::createRaoul(GameStateSystem::instance().currentLevel.at("raoul"));
	playerTaji = Taji::createTaji(GameStateSystem::instance().currentLevel.at("taji"));
	playerEmber = Ember::createEmber(GameStateSystem::instance().currentLevel.at("ember"));
	playerChia = Chia::createChia(GameStateSystem::instance().currentLevel.at("chia"));
}

void WorldSystem::createMobs(int frameBufferWidth, int frameBufferHeight)
{
	auto mobs = GameStateSystem::instance().currentLevel.at("mobs");
	createEnemies(mobs);
}

// On key callback
void WorldSystem::onKey(int key, int, int action, int mod)
{
	//Don't let debug buttons work unless in game
	if (!GameStateSystem::instance().inGameState()) {
		return;
	}
	// Handles inputs for camera movement
	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraComponent = camera.get<CameraComponent>();
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_UP) {
			cameraComponent.velocity.y = -cameraComponent.speed;
		} else if (key == GLFW_KEY_DOWN) {
			cameraComponent.velocity.y = cameraComponent.speed;
		}
		if (key == GLFW_KEY_LEFT) {
			cameraComponent.velocity.x = -cameraComponent.speed;
		} else if (key == GLFW_KEY_RIGHT) {
			cameraComponent.velocity.x = cameraComponent.speed;
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT && cameraComponent.velocity.x <= 0) {
			cameraComponent.velocity.x = 0;
		}
		else if (key == GLFW_KEY_RIGHT && cameraComponent.velocity.x >= 0) {
			cameraComponent.velocity.x = 0;
		}
		else if (key == GLFW_KEY_UP && cameraComponent.velocity.y <= 0) {
			cameraComponent.velocity.y = 0;
		}
		else if (key == GLFW_KEY_DOWN && cameraComponent.velocity.y >= 0) {
			cameraComponent.velocity.y = 0;
		}
	}

	// Animation Test
	if (action == GLFW_RELEASE && key == GLFW_KEY_3) {
		auto& anim = playerEmber.get<AnimationsComponent>();
		anim.changeAnimation(AnimationType::ATTACK3);
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_4) {
		for (auto entity : ECS::registry<Lettuce>.entities)
		{
			auto& anim = entity.get<AnimationsComponent>();
			anim.changeAnimation(AnimationType::ATTACK2);
		}
	}

	// Debug info for stats...press 'S' at any time to print out the stats for all entities. It's useful for checking
	// whether buffs and debuffs are working properly and to see how much damage was applied in an attack
	if (action == GLFW_RELEASE && key == GLFW_KEY_S)
	{
		for (auto& entity : ECS::registry<StatsComponent>.entities)
		{
			auto& statsComponent = entity.get<StatsComponent>();

			std::cout << "Entity " << entity.id << "-->"
								<< " HP: " << statsComponent.getStatValue(StatType::HP)
								<< " Ambrosia: " << statsComponent.getStatValue(StatType::AMBROSIA)
								<< " Strength: " << statsComponent.getStatValue(StatType::STRENGTH)
								<< std::endl;
		}
	}

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R)
	{
			int w, h;
			glfwGetWindowSize(window, &w, &h);

			restart();
	}

	if (key == GLFW_KEY_H && action == GLFW_RELEASE)
	{
		if (GameStateSystem::instance().isInHelpScreen)
		{
			EventSystem<HideHelpEvent>::instance().sendEvent(HideHelpEvent{});
		}
		else if (!GameStateSystem::instance().isInTutorial)
		{
			EventSystem<ShowHelpEvent>::instance().sendEvent(ShowHelpEvent{});
		}
	}

	// Debugging
	if (key == GLFW_KEY_D)
		DebugSystem::in_debug_mode = (action != GLFW_RELEASE);

	// swap maps for swapping between pizza and dessert maps for recipe 1
	if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
			GameStateSystem::instance().currentLevelIndex = 1;
			GameStateSystem::instance().restartMap();
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_N) {
		GameStateSystem::instance().currentLevelIndex = 0;
		GameStateSystem::instance().restartMap();
	}

	// load save debug
	if (action == GLFW_RELEASE && key == GLFW_KEY_L) {
		json save_obj = lc.load();
		if (save_obj.contains("recipe")) // only try to load if a save actually exists
		{
			GameStateSystem::instance().recipe = lc.readLevel(save_obj["recipe"]);
			GameStateSystem::instance().currentLevelIndex = save_obj["level"];
			GameStateSystem::instance().restartMap();
		}
	}

	// Play the next audio track (this is just so that we can give all of them a try)
	if (action == GLFW_RELEASE && key == GLFW_KEY_A) {
		playNextAudioTrack_DEBUG();
	}
}

void WorldSystem::onMouseClick(int button, int action, int mods) const
{
	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		double mousePosX, mousePosY;
		glfwGetCursorPos(window, &mousePosX, &mousePosY);

		std::cout << "Mouse click (release): {" << mousePosX << ", " << mousePosY << "}" << std::endl;

		if (GameStateSystem::instance().isTransitioning)
		{
			return;
		}

		if (GameStateSystem::instance().isInStory)
		{
			int storyIndex = ++GameStateSystem::instance().currentStoryIndex;
			std::cout << "Advancing story to index: " << storyIndex << std::endl;

			if (storyIndex > 9)
			{
				GameStateSystem::instance().isTransitioning = true;
				TransitionEvent event;
				event.callback = []() {
					GameStateSystem::instance().newGame();
				};
				EventSystem<TransitionEvent>::instance().sendEvent(event);
			}
			else
			{
				GameStateSystem::instance().isTransitioning = true;
				TransitionEvent event;
				event.callback = []() {
					EventSystem<AdvanceStoryEvent>::instance().sendEvent(AdvanceStoryEvent{});
				};
				EventSystem<TransitionEvent>::instance().sendEvent(event);
			}
		}

		RawMouseClickEvent event;
		event.mousePos = {mousePosX, mousePosY};
		EventSystem<RawMouseClickEvent>::instance().sendEvent(event);
		EventSystem<PlaySoundEffectEvent>::instance().sendEvent({SoundEffect::MOUSE_CLICK});
	}
}

void WorldSystem::onMouseHover(double xpos, double ypos) const
{
	RawMouseHoverEvent event;
	event.mousePos = { xpos, ypos };
	EventSystem<RawMouseHoverEvent>::instance().sendEvent(event);
}

void WorldSystem::initAudio()
{
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		throw std::runtime_error(Mix_GetError());

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
		throw std::runtime_error(Mix_GetError());

	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

	/*
	 * This music was made by David Vitas (https://www.davidvitas.com/portfolio/2016/5/12/rpg-music-pack). If we decide
	 * to keep some or all of it in the game, then we need to credit him somewhere in the game.
	 *
	 * I only included the looped tracks. Most of them have a separate intro track that can be played before playing the
	 * looped track, but that's tricky to implement with SDL_mixer, so I kept it simple, using only the looped ones.
	 *
	 * NOTE: Ambrosia_Theme.wav was composed by Emma! \o/
	 * */
	music[MusicType::START_SCREEN] = Mix_LoadMUS(audioPath("music/Ambrosia_Theme.wav").c_str());
	music[MusicType::SHOP] = Mix_LoadMUS(audioPath("music/Overworld_Theme.wav").c_str());
	music[MusicType::VICTORY] = Mix_LoadMUS(audioPath("music/Victory_Fanfare_Loop.wav").c_str());
	music[MusicType::BOSS] = Mix_LoadMUS(audioPath("music/Boss_Battle_Loop.wav").c_str());
	music[MusicType::PIZZA_ARENA] = Mix_LoadMUS(audioPath("music/Battle_Theme_Loop.wav").c_str());
	music[MusicType::DESSERT_ARENA] = Mix_LoadMUS(audioPath("music/Lullaby_Loop.wav").c_str());
	music[MusicType::PLACEHOLDER1] = Mix_LoadMUS(audioPath("music/Town_Theme.wav").c_str());
	music[MusicType::PLACEHOLDER2] = Mix_LoadMUS(audioPath("music/Evil_Gloating_Loop.wav").c_str());
	music[MusicType::PLACEHOLDER3] = Mix_LoadMUS(audioPath("music/Deep_Forest.wav").c_str());
	music[MusicType::PLACEHOLDER4] = Mix_LoadMUS(audioPath("music/Time_Cave.wav").c_str());
	music[MusicType::PLACEHOLDER5] = Mix_LoadMUS(audioPath("music/Title_screen.wav").c_str());

	// Check that all music was loaded
	for (auto& musicItem : music)
	{
		if (!musicItem.second)
		{
			throw std::runtime_error(Mix_GetError());
		}
	}

	/*
	 * The `turn_start` and `game_over` sound effects are from www.freesound.org. We can use those effects without needing
	 * to credit the creators because they are licensed under the Creative Commons 0 License.
	 *
	 * The `hit_player`, `hit_mob`, and `defeat` sound effects are from www.zapsplat.com. I consider them to be
	 * placeholders until we find some better sound effects, but if we decide to keep them in the game, then we need to
	 * add this somewhere in the game:
	 *
	 * 		“Sound effects obtained from https://www.zapsplat.com“
	 *
	 * The remaining sound effects are from www.fesliyanstudios.com. If we decide to keep them in the game, then we need
	 * to add this somewhere in the game:
	 *
	 * 		Credit: https://www.FesliyanStudios.com Background Music
	 *
	 * We can use them for free, as long as we don't make money off of this game. If that changes, then we need to pay for
	 * licensing (see https://www.fesliyanstudios.com/policy).
	 * */

	soundEffects[SoundEffect::MOUSE_CLICK] = Mix_LoadWAV(audioPath("effects/mouse_click.wav").c_str());
	soundEffects[SoundEffect::TURN_START] = Mix_LoadWAV(audioPath("effects/turn_start.wav").c_str());
	soundEffects[SoundEffect::GAME_OVER] = Mix_LoadWAV(audioPath("effects/game_over.wav").c_str());
	soundEffects[SoundEffect::HIT_PLAYER] = Mix_LoadWAV(audioPath("effects/hit_player.wav").c_str());
	soundEffects[SoundEffect::HIT_MOB] = Mix_LoadWAV(audioPath("effects/hit_mob.wav").c_str());
	soundEffects[SoundEffect::DEFEAT] = Mix_LoadWAV(audioPath("effects/defeat.wav").c_str());
	soundEffects[SoundEffect::MELEE] = Mix_LoadWAV(audioPath("effects/melee.wav").c_str());
	soundEffects[SoundEffect::PROJECTILE] = Mix_LoadWAV(audioPath("effects/projectile.wav").c_str());
	soundEffects[SoundEffect::BUFF] = Mix_LoadWAV(audioPath("effects/buff.wav").c_str());
	soundEffects[SoundEffect::DEBUFF] = Mix_LoadWAV(audioPath("effects/debuff.wav").c_str());

	// Check that all sound effects were loaded
	for (auto& soundEffectItem : soundEffects)
	{
		if (!soundEffectItem.second)
		{
			throw std::runtime_error(Mix_GetError());
		}
	}

	// Hook up the listener for sound effect events
	soundEffectListener = EventSystem<PlaySoundEffectEvent>::instance().registerListener(
			std::bind(&WorldSystem::onPlaySoundEffectEvent, this, std::placeholders::_1));

	// Listen for player change events - also for playing audio
	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
			std::bind(&WorldSystem::onPlayerChangeEvent, this, std::placeholders::_1));
}

void WorldSystem::releaseAudio()
{
	// Stop listening for sound effect events
	if (soundEffectListener.isValid())
	{
		EventSystem<PlaySoundEffectEvent>::instance().unregisterListener(soundEffectListener);
	}

	// Destroy music components
	for (auto& musicItem : music)
	{
		if (musicItem.second)
		{
			Mix_FreeMusic(musicItem.second);
		}
	}

	// Destroy sound effect
	for (auto& soundEffectItem : soundEffects)
	{
		if (soundEffectItem.second)
		{
			Mix_FreeChunk(soundEffectItem.second);
		}
	}

	Mix_CloseAudio();
}

void WorldSystem::playAudio()
{
	MusicType nextMusicType;

	if (GameStateSystem::instance().currentLevel.at("map") == "pizza-arena")
	{
		nextMusicType = MusicType::PIZZA_ARENA;
	}
	else if (GameStateSystem::instance().currentLevel.at("map") == "dessert-arena")
	{
		nextMusicType = MusicType::DESSERT_ARENA;
	}
	else if (GameStateSystem::instance().currentLevel.at("map") == "veggie-forest")
	{
		nextMusicType = MusicType::PLACEHOLDER3;
	}
	else
	{
		nextMusicType = MusicType::PLACEHOLDER1;
	}

	Mix_PlayMusic(music[nextMusicType], -1);
	currentMusic_DEBUG = nextMusicType;
}

void WorldSystem::playNextAudioTrack_DEBUG()
{
	int nextMusicType = (currentMusic_DEBUG + 1) % MusicType::LAST;
	currentMusic_DEBUG = static_cast<MusicType>(nextMusicType);

	Mix_PlayMusic(music[currentMusic_DEBUG], -1);
}

void WorldSystem::onPlaySoundEffectEvent(const PlaySoundEffectEvent& event)
{
	if (event.effect != SoundEffect::NONE)
	{
		Mix_PlayChannel(-1, soundEffects[event.effect], 0);
	}
}

void WorldSystem::onPlayerChangeEvent(const PlayerChangeEvent& event)
{
	if (shouldPlayAudioAtStartOfTurn)
	{
		Mix_PlayChannel(-1, soundEffects[SoundEffect::TURN_START], 0);
	}
	shouldPlayAudioAtStartOfTurn = true;

	// When the active player changes, go back to the normal mouse cursor
	glfwSetCursor(window, nullptr);
}

void WorldSystem::onLoadLevelEvent(const LoadLevelEvent& event)
{
	restart();
}

void WorldSystem::onTransition(TransitionEvent event)
{
	assert(!ECS::registry<ScreenState>.entities.empty());
	auto& screenEntity = ECS::registry<ScreenState>.entities.front();
	transition = event.callback;

	if (!screenEntity.has<TimerComponent>())
	{
		screenEntity.emplace<TimerComponent>();
	}
	
	auto& screenTimer = screenEntity.get<TimerComponent>();
	if (event.isFadingOut)
	{
		std::cout << "Beginning transition out" << std::endl;
		screenTimer.isCountingUp = true;
		screenTimer.maxTime_ms = event.duration;
		screenTimer.counter_ms = 0.f;
		screenTimer.complete = false;
	}
	else // fading in
	{
		screenTimer.isCountingUp = false;
		screenTimer.maxTime_ms = event.duration;
		screenTimer.counter_ms = event.duration;
		screenTimer.complete = false;
	}
}

void WorldSystem::initCursors()
{
	// A lambda to cut down on code duplication. Its job is to load a mouse cursor
	// image and use it to create the GLFWcursor object.
	auto createCursor = [&](std::string path, vec2 hotspotCoords) -> GLFWcursor*
	{
		int width, height, numChannelsRead;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &numChannelsRead, 0);

		if (!data || numChannelsRead != 4)
		{
			throw std::runtime_error("Failed to load cursor: " + path);
		}

		GLFWimage cursorImage;
		cursorImage.width = width;
		cursorImage.height = height;
		cursorImage.pixels = data;

		GLFWcursor* mouseCursor = glfwCreateCursor(&cursorImage, hotspotCoords.x, hotspotCoords.y);

		if (!mouseCursor)
		{
			throw std::runtime_error("Failed to set cursor: " + path);
		}

		stbi_image_free(data);
		return mouseCursor;
	};

	// Create two cursors
	moveCursor = createCursor(uiPath("cursor-shoe.png"), {40.f, 40.f});
	skillCursor = createCursor(uiPath("cursor-hand.png"), {21.f, 0.f});

	// Listen for a few events to change the mouse cursor
	setActiveSkillListener = EventSystem<SetActiveSkillEvent>::instance().registerListener(
			std::bind(&WorldSystem::onSetActiveSkillEvent, this, std::placeholders::_1));

	finishedMovementListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&WorldSystem::onFinishedMovementEvent, this, std::placeholders::_1));

	finishedSkillListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&WorldSystem::onFinishedSkillEvent, this, std::placeholders::_1));
}

void WorldSystem::releaseCursors()
{
	// Stop listening for events related to changing the mouse cursor
	if (setActiveSkillListener.isValid())
	{
		EventSystem<SetActiveSkillEvent>::instance().unregisterListener(setActiveSkillListener);
	}
	if (finishedMovementListener.isValid())
	{
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(finishedMovementListener);
	}
	if (finishedSkillListener.isValid())
	{
		EventSystem<FinishedSkillEvent>::instance().unregisterListener(finishedSkillListener);
	}

	// Release the cursors
	if (moveCursor)
	{
		glfwDestroyCursor(moveCursor);
	}
	if (skillCursor)
	{
		glfwDestroyCursor(skillCursor);
	}
}

void WorldSystem::onSetActiveSkillEvent(const SetActiveSkillEvent& event)
{
	auto entity = event.entity;
	if (!entity.has<PlayerComponent>())
	{
		return;
	}

	// Update the active cursor based on skill type
	if (event.type == SkillType::NONE)
	{
		glfwSetCursor(window, nullptr);
	}
	else if (event.type == SkillType::MOVE)
	{
		assert(moveCursor);
		glfwSetCursor(window, moveCursor);
	}
	else
	{
		assert(skillCursor);
		glfwSetCursor(window, skillCursor);
	}
}

void WorldSystem::onFinishedMovementEvent(const FinishedMovementEvent& event)
{
	auto entity = event.entity;
	if (!entity.has<PlayerComponent>())
	{
		return;
	}

	// When movement finishes, go back to the normal mouse cursor
	glfwSetCursor(window, nullptr);
}

void WorldSystem::onFinishedSkillEvent(const FinishedSkillEvent& event)
{
	auto entity = event.entity;
	if (!entity.has<PlayerComponent>())
	{
		return;
	}

	// When a skill finishes, go back to the normal mouse cursor
	glfwSetCursor(window, nullptr);
}
