#include "world.hpp"
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

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

// Game configuration
const size_t MAX_EGGS = 3;

// Create the world
// Note, this has a lot of OpenGL specific things, could be moved to the renderer; but it also defines the callbacks to the mouse and keyboard. That is why it is called here.
WorldSystem::WorldSystem(ivec2 window_size_px) :
	points(0)
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


	// Playing background music indefinitely
	initAudio();
	Mix_PlayMusic(background_music, -1);
	std::cout << "Loaded music\n";
}

WorldSystem::~WorldSystem(){
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);
	Mix_CloseAudio();

	// Destroy all created components
	ECS::ContainerInterface::clearAllComponents();

	// Close the window
	glfwDestroyWindow(window);
}

void WorldSystem::initAudio()
{
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
		throw std::runtime_error("Failed to initialize SDL Audio");

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
		throw std::runtime_error("Failed to open audio device");

	background_music = Mix_LoadMUS(audioPath("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audioPath("salmon_dead.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audioPath("salmon_eat.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr)
		throw std::runtime_error("Failed to load sounds make sure the data directory is present: " +
																 audioPath("music.wav") +
														 audioPath("salmon_dead.wav") +
														 audioPath("salmon_eat.wav"));

}

// Update our game world
void WorldSystem::step(float elapsed_ms, vec2 window_size_in_game_units)
{
	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());
	
	// Removing out of screen entities
	auto& registry = ECS::registry<Motion>;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current upon delete)
	for (int i = static_cast<int>(registry.components.size())-1; i >= 0; --i)
	{
		auto& motion = registry.components[i];
		if (motion.position.x < -200.f || motion.position.x > (window_size_in_game_units.x + 200.f)
		    || motion.position.y < -200.f || motion.position.y > (window_size_in_game_units.y + 200.f))
		{
			ECS::ContainerInterface::removeAllComponentsOf(registry.entities[i]);
		}
	}

	/*
	// Spawning new eggs
	while (ECS::registry<Egg>.components.size() < MAX_EGGS)
	{
		// Create egg mob
		ECS::Entity entity = Egg::createEgg({0, 0});
		// Setting random initial position and constant velocity
		auto& motion = entity.get<Motion>();
		motion.position = vec2(window_size_in_game_units.x - 150.f, 50.f + uniform_dist(rng) * (window_size_in_game_units.y - 100.f));
		motion.velocity = vec2(-100.f, 0.f );
	}
	*/

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

	// Debugging for memory/component leaks
	ECS::ContainerInterface::listAllComponents();

	// Get screen/buffer size
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	createMap(frameBufferWidth, frameBufferHeight);
	createPlayers(frameBufferWidth, frameBufferHeight);
	createMobs(frameBufferWidth, frameBufferHeight);
	createButtons(frameBufferWidth, frameBufferHeight);
	createEffects(frameBufferWidth, frameBufferHeight);
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
	MapComponent::createMap("pizza-arena/pizza-arena-debug", {frameBufferWidth, frameBufferHeight});

	// Create a deforming blob
	CheeseBlob::createCheeseBlob({ 700, 950 });
}

void WorldSystem::createButtons(int frameBufferWidth, int frameBufferHeight)
{
	// Temporarily here so it renders behind the buttons
	ActiveSkillFX::createActiveSkillFX();

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
	/*ActiveSkillFX::createActiveSkillFX();*/
}

void WorldSystem::createPlayers(int frameBufferWidth, int frameBufferHeight)
{
	playerRaoul = Raoul::createRaoul({ 640, 512 });
	playerTaji = Taji::createTaji({ 200,700 });
	playerEmber = Ember::createEmber({ 400,700 });
	playerChia = Chia::createChia({ 400,400 });
}

void WorldSystem::createMobs(int frameBufferWidth, int frameBufferHeight)
{
	//This is not the final way to add eggs just put them here for testing purposes.
	Egg::createEgg({750, 800});
	Egg::createEgg({1000, 800});
}

// On key callback
void WorldSystem::onKey(int key, int, int action, int mod)
{
	// Animation Test
	if (action == GLFW_PRESS && key == GLFW_KEY_3) {
		auto& anim = playerTaji.get<AnimationsComponent>();
		anim.changeAnimation(AnimationType::ATTACK1);
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_4) {
		for (auto entity : ECS::registry<Egg>.entities)
		{
			auto& anim = entity.get<AnimationsComponent>();
			anim.changeAnimation(AnimationType::HIT);
		}
	}

	// Debug info for stats...press 'S' at any time to print out the stats for all entities. It's useful for checking
	// whether buffs and debuffs are working properly and to see how much damage was applied in an attack
	if (action == GLFW_PRESS && key == GLFW_KEY_S)
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
	}
}

void WorldSystem::onMouseHover(double xpos, double ypos) const
{
	RawMouseHoverEvent event;
	event.mousePos = { xpos, ypos };
	EventSystem<RawMouseHoverEvent>::instance().sendEvent(event);
}
