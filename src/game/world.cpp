#include "world.hpp"
#include "camera.hpp"
#include "event_system.hpp"
#include "events.hpp"

#include "physics/physics.hpp"
#include "physics/projectile.hpp"
#include "physics/debug.hpp"
#include "entities/enemies.hpp"
#include "rendering/render_components.hpp"
#include "animation/animation_components.hpp"
#include "ui/button.hpp"
#include "ui/ui_system.hpp"
#include "ui/ui_entities.hpp"
#include "ui/menus.hpp"
#include "ai/ai.hpp"
#include "level_loader/level_loader.hpp"
#include "game_state_system.hpp"
#include "ai/swarm_behaviour.hpp"
#include "rendering/text.hpp"
#include "entities/players.hpp"
#include "ui/tutorials.hpp"


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

	transitionEventListener = EventSystem<TransitionEvent>::instance().registerListener(
		std::bind(&WorldSystem::onTransition, this, std::placeholders::_1));

	initCursors();
}

WorldSystem::~WorldSystem(){
	releaseAudio();
	releaseCursors();

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

			if (entity.has<PlayerComponent>())
			{
				// For player deaths, disable their rendering but keep everything else
				Player::disableRendering(entity);
			}
			else
			{
				// For mob deaths, get rid of all their components
				ECS::ContainerInterface::removeAllComponentsOf(entity);
			}

			// Check if there are no more players left, restart game
			auto& players = ECS::registry<PlayerComponent>.entities;
			int numAlive = std::count_if(players.begin(), players.end(), [](ECS::Entity e)
			{
				return !e.has<DeathTimer>();
			});

			if (numAlive == 0 && !GameStateSystem::instance().isTransitioning)
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
	// If all mobs are dead and there are no active projectiles (e.g., ambrosia
	// projectiles), then go to the victory screen
	if (ECS::registry<AISystem::MobComponent>.entities.empty() &&
			ECS::registry<ProjectileComponent>.components.empty())
	{
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

	for (int i = 0; i < ECS::registry<AchievementPopup>.entities.size(); i++)
	{
		auto& achievementPopup = ECS::registry<AchievementPopup>.entities[i];
		auto& achievementTimer = achievementPopup.get<TimerComponent>();
		if (achievementTimer.counter_ms == achievementTimer.maxTime_ms)
		{
			ECS::ContainerInterface::removeAllComponentsOf(achievementPopup);
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

// On key callback
void WorldSystem::onKey(int key, int, int action, int mod)
{
	// Mute the music (but leave sound effects on)
	if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
		Mix_VolumeMusic(0);
	}

	// Debug printout of the entity registries
	if (action == GLFW_RELEASE && key == GLFW_KEY_P) {
		ECS::ContainerInterface::listAllComponents();
	}

	//Don't let debug buttons work unless in game
	if (!GameStateSystem::instance().inGameState()) {
		// Skip story
		if (GameStateSystem::instance().isInStory && !GameStateSystem::instance().isTransitioning && action == GLFW_RELEASE && key == GLFW_KEY_SPACE)
		{
			GameStateSystem::instance().beginTutorial();
		}
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
	if (action == GLFW_RELEASE && key == GLFW_KEY_4) {
		for (auto entity : ECS::registry<Chicken>.entities)
		{
			auto& anim = entity.get<AnimationsComponent>();
			anim.changeAnimation(AnimationType::ATTACK1);
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

	if (key == GLFW_KEY_I && action == GLFW_RELEASE)
	{
		TutorialSystem::toggleInspectMode();
	}

	// Debugging
	if (key == GLFW_KEY_D)
		DebugSystem::in_debug_mode = (action != GLFW_RELEASE);

	// Advance to next level or go back to main menu if no levels left
	if (action == GLFW_RELEASE && key == GLFW_KEY_N) {
			GameStateSystem::instance().nextMap();
	}

	// load save debug
	if (action == GLFW_RELEASE && key == GLFW_KEY_L) {
		GameStateSystem::instance().loadSave();
	}

	// Play the next audio track (this is just so that we can give all of them a try)
	if (action == GLFW_RELEASE && key == GLFW_KEY_A) {
		playNextAudioTrack_DEBUG();
	}

	// Go to main menu (for checking achievements after saving)
	if (action == GLFW_RELEASE && key == GLFW_KEY_BACKSPACE) {
		GameStateSystem::instance().launchMainMenu();
	}

	// Print player and skill levels
	if (action == GLFW_RELEASE && key == GLFW_KEY_U) {
		std::cout << GameStateSystem::instance().getSkillsForAllPlayers() << std::endl;
	}
}

void WorldSystem::onMouseClick(int button, int action, int mods) const
{
	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
	{
		double mousePosX, mousePosY;
		glfwGetCursorPos(window, &mousePosX, &mousePosY);

		std::cout << "Mouse click (release): {" << mousePosX << ", " << mousePosY << "}" << std::endl;

		//auto camera = ECS::registry<CameraComponent>.entities[0];
		//auto& cameraPos = camera.get<CameraComponent>().position;
		// mouse click print without camera position
		//std::cout << "Mouse click (release): {" << mousePosX + cameraPos.x << ", " << mousePosY + cameraPos.y << "}" << std::endl;

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
					GameStateSystem::instance().beginTutorial();
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

	// Hook up the listeners for music and sound effect events
	musicListener = EventSystem<PlayMusicEvent>::instance().registerListener(
			std::bind(&WorldSystem::onPlayMusicEvent, this, std::placeholders::_1));

	soundEffectListener = EventSystem<PlaySoundEffectEvent>::instance().registerListener(
			std::bind(&WorldSystem::onPlaySoundEffectEvent, this, std::placeholders::_1));

	// Listen for player change events - also for playing audio
	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
			std::bind(&WorldSystem::onPlayerChangeEvent, this, std::placeholders::_1));
}

void WorldSystem::releaseAudio()
{
	// Stop listening for music and sound effect events
	if (musicListener.isValid())
	{
		EventSystem<PlayMusicEvent>::instance().unregisterListener(musicListener);
	}

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

void WorldSystem::playNextAudioTrack_DEBUG()
{
	int nextMusicType = (currentMusic_DEBUG + 1) % MusicType::LAST;
	currentMusic_DEBUG = static_cast<MusicType>(nextMusicType);

	Mix_PlayMusic(music[currentMusic_DEBUG], -1);
}

void WorldSystem::onPlayMusicEvent(const PlayMusicEvent& event)
{
	Mix_PlayMusic(music[event.musicType], -1);
	currentMusic_DEBUG = event.musicType;
	shouldPlayAudioAtStartOfTurn = false;
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
	// Unless the game is in inspect mode 
	// this can happen when pressing inspect right at the end of a skill
	if (!GameStateSystem::instance().isInspecting)
	{
		glfwSetCursor(window, nullptr);
		preInspectCursorType = 0;
	}
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
	inspectCursor = createCursor(uiPath("cursor-inspect.png"), {40.f, 40.f});

	// Listen for a few events to change the mouse cursor
	setActiveSkillListener = EventSystem<SetActiveSkillEvent>::instance().registerListener(
			std::bind(&WorldSystem::onSetActiveSkillEvent, this, std::placeholders::_1));

	finishedMovementListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&WorldSystem::onFinishedMovementEvent, this, std::placeholders::_1));

	finishedSkillListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&WorldSystem::onFinishedSkillEvent, this, std::placeholders::_1));

	resetMouseCursorListener = EventSystem<ResetMouseCursorEvent>::instance().registerListener(
			std::bind(&WorldSystem::onResetMouseCursorEvent, this, std::placeholders::_1));

	enterInspectModeListener = EventSystem<BeginInspectEvent>::instance().registerListener(
		std::bind(&WorldSystem::onEnterInspectMode, this));

	exitInspectModeListener = EventSystem<EndInspectEvent>::instance().registerListener(
		std::bind(&WorldSystem::onExitInspectMode, this));
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
	if (enterInspectModeListener.isValid())
	{
		EventSystem<BeginInspectEvent>::instance().unregisterListener(enterInspectModeListener);
	}
	if (exitInspectModeListener.isValid())
	{
		EventSystem<EndInspectEvent>::instance().unregisterListener(exitInspectModeListener);
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
	if (inspectCursor)
	{
		glfwDestroyCursor(inspectCursor);
	}
}

void WorldSystem::onSetActiveSkillEvent(const SetActiveSkillEvent& event)
{
	auto entity = event.entity;
	if (!entity.has<PlayerComponent>()|| GameStateSystem::instance().isInspecting)
	{
		return;
	}

	// Update the active cursor based on skill type
	if (event.type == SkillType::NONE)
	{
		glfwSetCursor(window, nullptr);
		preInspectCursorType = 0;
	}
	else if (event.type == SkillType::MOVE)
	{
		assert(moveCursor);
		glfwSetCursor(window, moveCursor);
		preInspectCursorType = 1;
	}
	else
	{
		assert(skillCursor);
		glfwSetCursor(window, skillCursor);
		preInspectCursorType = 2;
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
	preInspectCursorType = 0;
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
	preInspectCursorType = 0;
}

void WorldSystem::onResetMouseCursorEvent(const ResetMouseCursorEvent& event)
{
	glfwSetCursor(window, nullptr);
	preInspectCursorType = 0;
}

void WorldSystem::onEnterInspectMode()
{
	glfwSetCursor(window, inspectCursor);
}

void WorldSystem::onExitInspectMode()
{
	if (preInspectCursorType == 1)
	{
		assert(moveCursor);
		glfwSetCursor(window, moveCursor);
	}
	else if (preInspectCursorType == 2)
	{
		assert(skillCursor);
		glfwSetCursor(window, skillCursor);
	}
	else
	{
		glfwSetCursor(window, nullptr);
	}
}
