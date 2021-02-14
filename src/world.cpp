// Header
#include "world.hpp"
#include "physics.hpp"
#include "debug.hpp"
#include "render_components.hpp"
#include "animation_components.hpp"
#include "egg.hpp"
#include "map_objects.hpp"
#include "turn_system.hpp"
#include "raoul.hpp"
#include "event_system.hpp"
#include "events.hpp"
#include "button.hpp"
#include "projectile.hpp"

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

		// Reduce window brightness if any of the present salmons is dying
		screen.darken_screen_factor = 1-counter.counter_ms/3000.f;

		// Restart the game once the death timer expired
		if (counter.counter_ms < 0)
		{
			ECS::registry<DeathTimer>.remove(entity);
			screen.darken_screen_factor = 0;
			restart();
			return;
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

	// Create a new Raoul
	player_raoul = Raoul::createRaoul({ 640, 512 }, 1.f);

	//TODO replace these with the real other characters
	auto raoul_2 = Raoul::createRaoul({ 200,700 }, 2.f);
	auto raoul_3 = Raoul::createRaoul({ 400,700 }, 3.f);
	auto raoul_4 = Raoul::createRaoul({ 400,400 }, 4.f);


	//This is not the final way to add eggs just put them here for testing purposes.
	ECS::Entity entity = Egg::createEgg({750, 800});
	entity = Egg::createEgg({1000, 800});
	// Setting random initial position and constant velocity
	//auto& motion = entity.get<Motion>();
	//motion.position = vec2(window_size_in_game_units.x - 150.f, 50.f + uniform_dist(rng) * (window_size_in_game_units.y - 100.f));

	// Removing existing map
	while (!ECS::registry<MapComponent>.entities.empty())
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<MapComponent>.entities.back());

	// Get screen/buffer size
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	// Create the map
	MapComponent::createMap("pizza-arena/pizza-arena-debug", {frameBufferWidth, frameBufferHeight});

	// Create a deforming blob
	CheeseBlob::createCheeseBlob({ 700, 950 });

	// Create UI buttons
	Button::createButton(ButtonShape::RECTANGLE, { frameBufferWidth / 4, 60 }, "placeholder_char_button",
		[]() {
					std::cout << "Character one button clicked!" << std::endl;
					if (!ECS::registry<PlayerComponent>.entities.empty())
					{
						TurnSystem::changeActiveEntity(ECS::registry<PlayerComponent>.entities[0]);
					}
			});
	Button::createButton(ButtonShape::RECTANGLE, { frameBufferWidth / 4 + 200, 60 }, "placeholder_char_button",
		[]() {
					std::cout << "Character two button clicked!" << std::endl;
					if (ECS::registry<PlayerComponent>.entities.size() > 1)
					{
						TurnSystem::changeActiveEntity(ECS::registry<PlayerComponent>.entities[1]);
					}
			});
	Button::createButton(ButtonShape::RECTANGLE, { frameBufferWidth / 4 + 400, 60 }, "placeholder_char_button",
		[]() {
					std::cout << "Character three button clicked!" << std::endl;
					if (ECS::registry<PlayerComponent>.entities.size() > 2)
					{
						TurnSystem::changeActiveEntity(ECS::registry<PlayerComponent>.entities[2]);
					}
			});
	Button::createButton(ButtonShape::RECTANGLE, { frameBufferWidth / 4 + 600, 60 }, "placeholder_char_button",
		[]() {
					std::cout << "Character four button clicked!" << std::endl;
					if (ECS::registry<PlayerComponent>.entities.size() > 3)
					{
						TurnSystem::changeActiveEntity(ECS::registry<PlayerComponent>.entities[3]);
					}
			});

	Button::createButton(ButtonShape::CIRCLE, { 100, frameBufferHeight - 80 }, "skill_buttons/placeholder_skill",
		[]() { std::cout << "Skill one button clicked!" << std::endl; });
	Button::createButton(ButtonShape::CIRCLE, { 250, frameBufferHeight - 80 }, "skill_buttons/placeholder_skill",
		[]() { std::cout << "Skill two button clicked!" << std::endl; });
	Button::createButton(ButtonShape::CIRCLE, { 400, frameBufferHeight - 80 }, "skill_buttons/placeholder_skill",
		[]() { std::cout << "Skill three button clicked!" << std::endl; });
	Button::createButton(ButtonShape::CIRCLE, { 550, frameBufferHeight - 80 }, "skill_buttons/placeholder_skill",
		[]() { std::cout << "Skill four button clicked!" << std::endl; });
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
			if (ECS::registry<Raoul>.has(entity_other) || ECS::registry<Egg>.has(entity_other))
			{
				auto& projComponent = entity.get<ProjectileComponent>();

				HitEvent event;
				event.instigator = projComponent.instigator;
				event.target = entity_other;

				EventSystem<HitEvent>::instance().sendEvent(event);
			}
		}

		// Check for collisions between player and eggs
		if (ECS::registry<Raoul>.has(entity))
		{
			if (ECS::registry<Egg>.has(entity_other))
			{
				///***** This has been commented out because restarting the game breaks the turn system *****//
				// Please uncomment this when developing the turn system to make sure it's fixed//

				// initiate death unless already dying
				if (!ECS::registry<DeathTimer>.has(entity))
				{
					// Scream, reset timer, and make the player sink
					ECS::registry<DeathTimer>.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
				}
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

// On key callback
void WorldSystem::onKey(int key, int, int action, int mod)
{
		// Animation Test
		if (action == GLFW_PRESS && key == GLFW_KEY_3) {
			auto& anim = player_raoul.get<AnimationsComponent>();
			anim.changeAnimation(AnimationType::ATTACK3);
		}
		if (action == GLFW_PRESS && key == GLFW_KEY_4) {
			for (auto entity : ECS::registry<Egg>.entities)
			{
				auto& anim = entity.get<AnimationsComponent>();
				anim.changeAnimation(AnimationType::HIT);
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
