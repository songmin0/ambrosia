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
#include "entities/egg.hpp"
#include "rendering/render_components.hpp"
#include "animation/animation_components.hpp"
#include "maps/map_objects.hpp"
#include "ui/button.hpp"
#include "ui/ui_system.hpp"
#include "ui/effects.hpp"
#include "ui/ui_entities.hpp"
#include "ai/ai.hpp"
#include <level_loader/level_loader.hpp>

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

// Create the world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px) :
	points(0),
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


	LevelLoader lc;
	config = lc.readLevel("pizza-arena");

	initAudio();
	std::cout << "Loaded music\n";
}

WorldSystem::~WorldSystem(){
	releaseAudio();

	// Destroy all created components
	ECS::ContainerInterface::clearAllComponents();

	// Close the window
	glfwDestroyWindow(window);
}

// Update our game world
void WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
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
				//If the entity has a stats component get rid of the health bar too
			if (entity.has<StatsComponent>()) {
						ECS::ContainerInterface::removeAllComponentsOf(entity.get<StatsComponent>().healthBar);
			}
			ECS::ContainerInterface::removeAllComponentsOf(entity);
			// Check if there are no more players left, restart game
			if (ECS::registry<PlayerComponent>.entities.empty())
			{
				EventSystem<PlaySoundEffectEvent>::instance().sendEvent({SoundEffect::GAME_OVER});
				restart();
				return;
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

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (!ECS::registry<Motion>.entities.empty())
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<Motion>.entities.back());

	// Remove camera entity
	while (!ECS::registry<CameraComponent>.entities.empty()) {
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<CameraComponent>.entities.back());
	}

	// Debugging for memory/component leaks
	ECS::ContainerInterface::listAllComponents();

	// Get screen/buffer size
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	Camera::createCamera(config.at("camera"));
	createMap(frameBufferWidth, frameBufferHeight);
	createPlayers(frameBufferWidth, frameBufferHeight);
	createMobs(frameBufferWidth, frameBufferHeight);
	createButtons(frameBufferWidth, frameBufferHeight);
	createEffects(frameBufferWidth, frameBufferHeight);

	playAudio();
	shouldPlayAudioAtStartOfTurn = false;
} 

// Compute collisions between entities
void WorldSystem::handleCollisions()
{
	// Loop over all collisions detected by the physics system
	auto& registry = ECS::registry<PhysicsSystem::Collision>;
	for (unsigned int i=0; i< registry.components.size(); i++)
	{
		// The entity and its collider
		auto entity = registry.entities[i];
		auto entity_other = registry.components[i].other;

		// Check for projectiles colliding with the player or with the eggs
		if (ECS::registry<ProjectileComponent>.has(entity))
		{
			auto& projComponent = entity.get<ProjectileComponent>();

			// Only allowing a projectile to collide with an entity once. It can collide with multiple entities, but only once
			// per entity
			if (projComponent.canCollideWith(entity_other))
			{
				projComponent.collideWith(entity_other);

				HitEvent event;
				event.instigator = projComponent.instigator;
				event.target = entity_other;
				event.damage = projComponent.params.damage;
				EventSystem<HitEvent>::instance().sendEvent(event);
			}
		}
	}

	// Remove all collisions from this simulation step
	ECS::registry<PhysicsSystem::Collision>.clear();
}

// Should the game be over ?
bool WorldSystem::isOver() const
{
	return glfwWindowShouldClose(window)>0;
}

void WorldSystem::createMap(int frameBufferWidth, int frameBufferHeight)
{
	// Create the map
	MapComponent::createMap(config.at("map"), {frameBufferWidth, frameBufferHeight});

	// Create a deforming blob for pizza arena
	// maybe add own section in level file we have more of these
	if (config.at("map") == "pizza-arena") {
		CheeseBlob::createCheeseBlob({ 700, 950 });
	}

	if (config.at("map") == "dessert-arena") {
		DessertForeground::createDessertForeground({ 1920, 672 });
	}
}

void WorldSystem::createButtons(int frameBufferWidth, int frameBufferHeight)
{

	// Create UI buttons
	auto player_button_1 = Button::createPlayerButton(PlayerType::RAOUL, { frameBufferWidth / 4, 60 },
		[]() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::RAOUL }); });

	auto player_button_2 = Button::createPlayerButton(PlayerType::TAJI, { frameBufferWidth / 4 + 200, 60 },
		[]() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::TAJI }); });

	auto player_button_3 = Button::createPlayerButton(PlayerType::EMBER, { frameBufferWidth / 4 + 400, 60 },
		[]() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::EMBER }); });

	auto player_button_4 = Button::createPlayerButton(PlayerType::CHIA, { frameBufferWidth / 4 + 600, 60 },
		[]() { EventSystem<PlayerButtonEvent>::instance().sendEvent(PlayerButtonEvent{ PlayerType::CHIA }); });

	SkillButton::createMoveButton({ 100, frameBufferHeight - 80 }, "skill_buttons/skill_generic_move",
		[]() {
		std::cout << "Move button clicked!" << std::endl;
	});

	SkillButton::createSkillButton({ 250, frameBufferHeight - 80 }, PlayerType::RAOUL, SkillType::SKILL1, "skill1",
		[]() {
			std::cout << "Skill one button clicked!" << std::endl;

			if (!ECS::registry<TurnSystem::TurnComponentIsActive>.entities.empty())
			{
				auto activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities.front();

				// Skill buttons should only affect players
				if (activeEntity.has<PlayerComponent>())
				{
					SetActiveSkillEvent event;
					event.entity = activeEntity;
					event.type = SkillType::SKILL1;

					EventSystem<SetActiveSkillEvent>::instance().sendEvent(event);
				}
			}
		});

	SkillButton::createSkillButton({ 400, frameBufferHeight - 80 }, PlayerType::RAOUL, SkillType::SKILL2, "skill2",
		[]() {
			std::cout << "Skill two button clicked!" << std::endl;

			if (!ECS::registry<TurnSystem::TurnComponentIsActive>.entities.empty())
			{
				auto activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities.front();

				// Skill buttons should only affect players
				if (activeEntity.has<PlayerComponent>())
				{
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

				// Skill buttons should only affect players
				if (activeEntity.has<PlayerComponent>())
				{
					SetActiveSkillEvent event;
					event.entity = activeEntity;
					event.type = SkillType::SKILL3;

					EventSystem<SetActiveSkillEvent>::instance().sendEvent(event);
				}
			}
		});

	ToolTip::createMoveToolTip({ 100, frameBufferHeight - 120 });
	ToolTip::createToolTip(PlayerType::RAOUL, SkillType::SKILL1, { 250, frameBufferHeight - 120 });
	ToolTip::createToolTip(PlayerType::RAOUL, SkillType::SKILL2, { 400, frameBufferHeight - 120 });
	ToolTip::createToolTip(PlayerType::RAOUL, SkillType::SKILL3, { 550, frameBufferHeight - 120 });
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

	playerRaoul = Raoul::createRaoul(config.at("raoul"));
	playerTaji = Taji::createTaji(config.at("taji"));
	playerEmber = Ember::createEmber(config.at("ember"));
	playerChia = Chia::createChia(config.at("chia"));
}

void WorldSystem::createMobs(int frameBufferWidth, int frameBufferHeight)
{
	// TODO: come back and expand this when we have multiple mobs
	auto mobs = config.at("mobs");

	for (json mob : mobs) {
		auto type = mob["type"];
		if (type == "egg") {
			Egg::createEgg({ mob.at("position")[0], mob["position"][1] });
		}
	}
}

// On key callback
void WorldSystem::onKey(int key, int, int action, int mod)
{
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
		anim.changeAnimation(AnimationType::ATTACK1);
	}
	if (action == GLFW_RELEASE && key == GLFW_KEY_4) {
		for (auto entity : ECS::registry<Egg>.entities)
		{
			auto& anim = entity.get<AnimationsComponent>();
			anim.changeAnimation(AnimationType::HIT);
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

	// Debugging
	if (key == GLFW_KEY_D)
		DebugSystem::in_debug_mode = (action != GLFW_RELEASE);

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
	{
		current_speed -= 0.1f;
		std::cout << "Current speed = " << current_speed << std::endl;
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
	{
		current_speed += 0.1f;
		std::cout << "Current speed = " << current_speed << std::endl;
	}
	current_speed = std::max(0.f, current_speed);

	LevelLoader lc;
	// swap maps
	if (action == GLFW_RELEASE && key == GLFW_KEY_M) {
		config = lc.readLevel("dessert-arena");
		restart();
	}

	if (action == GLFW_RELEASE && key == GLFW_KEY_N) {
		config = lc.readLevel("pizza-arena");
		restart();
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
	 * */
	music[MusicType::MAIN_MENU] = Mix_LoadMUS(audioPath("music/Title_screen.wav").c_str());
	music[MusicType::SHOP] = Mix_LoadMUS(audioPath("music/Overworld_Theme.wav").c_str());
	music[MusicType::VICTORY] = Mix_LoadMUS(audioPath("music/Victory_Fanfare_Loop.wav").c_str());
	music[MusicType::BOSS] = Mix_LoadMUS(audioPath("music/Boss_Battle_Loop.wav").c_str());
	music[MusicType::PIZZA_ARENA] = Mix_LoadMUS(audioPath("music/Battle_Theme_Loop.wav").c_str());
	music[MusicType::DESSERT_ARENA] = Mix_LoadMUS(audioPath("music/Lullaby_Loop.wav").c_str());
	music[MusicType::PLACEHOLDER1] = Mix_LoadMUS(audioPath("music/Town_Theme.wav").c_str());
	music[MusicType::PLACEHOLDER2] = Mix_LoadMUS(audioPath("music/Evil_Gloating_Loop.wav").c_str());
	music[MusicType::PLACEHOLDER3] = Mix_LoadMUS(audioPath("music/Deep_Forest.wav").c_str());
	music[MusicType::PLACEHOLDER4] = Mix_LoadMUS(audioPath("music/Time_Cave.wav").c_str());

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

	if (config.at("map") == "pizza-arena")
	{
		nextMusicType = MusicType::PIZZA_ARENA;
	}
	else if (config.at("map") == "dessert-arena")
	{
		nextMusicType = MusicType::DESSERT_ARENA;
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
}
