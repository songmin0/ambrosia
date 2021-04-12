
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "game/camera_system.hpp"
#include "game/common.hpp"
#include "game/world.hpp"
#include "game/turn_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "rendering/render.hpp"
#include "physics/physics.hpp"
#include "physics/debug.hpp"
#include "physics/projectile_system.hpp"
#include "ai/ai.hpp"
#include "ai/behaviour_tree.hpp"
#include "animation/animation_system.hpp"
#include "maps/path_finding_system.hpp"
#include "ui/ui_system.hpp"
#include "ui/tutorials.hpp"
#include "particles/particle_system.hpp"
#include "skills/skill_system.hpp"
#include "game/stats_system.hpp"
#include "game/game_state_system.hpp"
#include "ai/swarm_behaviour.hpp"
#include "effects/effect_system.hpp"
#include "game/range_indicator_system.hpp"
#include "game/achievement_system.hpp"
#include "ui/shop_system.hpp"


using Clock = std::chrono::high_resolution_clock;

const ivec2 window_size_in_px = { 1366, 900 };
const vec2 window_size_in_game_units = { 1366, 900 };
// Note, here the window will show a width x height part of the game world, measured in px. 
// You could also define a window to show 1.5 x 1 part of your game world, where the aspect ratio depends on your window size.

struct Description {
	std::string name;
	Description(const char* str) : name(str) {};
};

// Entry point
int main()
{
	// Initialize the main systems 
	WorldSystem world(window_size_in_px);
	ParticleSystem particleSystem;
	//particleSystem.emitters.push_back(std::make_shared<BasicEmitter>(BasicEmitter(5)));
	CameraSystem camera(window_size_in_px);
	RenderSystem renderer(*world.window, &particleSystem);
	PathFindingSystem pathFindingSystem;
	PhysicsSystem physics(pathFindingSystem);
	AISystem ai(pathFindingSystem);
	StateSystem stateSystem;
	TurnSystem turnSystem(pathFindingSystem);
	AnimationSystem animations;
	EffectSystem effectSystem;
	UISystem ui;
	ProjectileSystem projectileSystem;
	SkillSystem skillSystem;
	StatsSystem statsSystem;
	TutorialSystem tutorialSystem;
	RangeIndicatorSystem rangeIndicatorSystem;
	SwarmBehaviour swarmBehaviour;
	AchievementSystem::instance();

	GameStateSystem::instance().setWindow(world.window);
	GameStateSystem::instance().preloadResources();
	GameStateSystem::instance().launchMainMenu();

	// Reference: https://gafferongames.com/post/fix_your_timestep/#the-final-touch
	float t = 0.f;
	float dt = 16.67f; // milliseconds

	auto prevTime = Clock::now();
	float accumulator = 0.0;

	// Variable timestep loop
	while (!world.isOver())
	{
		// Calculate elapsed time in milliseconds from the previous iteration
		auto currTime = Clock::now();
		float frameTime = static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(currTime - prevTime)).count()) / 1000.f;
		prevTime = currTime;

		accumulator += frameTime;
		while (accumulator >= dt)
		{
			// Processes system messages, if this wasn't present the window would become unresponsive
			glfwPollEvents();

			DebugSystem::clearDebugComponents();
			world.step(dt, window_size_in_game_units);
			camera.step(dt);
			physics.step(dt, window_size_in_game_units);
			swarmBehaviour.step(dt, window_size_in_game_units);
			world.handleCollisions();
			projectileSystem.step(dt);
			skillSystem.step(dt);
			animations.step();
			effectSystem.step();
			ui.step(dt);
			turnSystem.step(dt);
			stateSystem.step(dt);
			particleSystem.step(dt);

			t += dt;
			accumulator -= dt;

		}

		// Blend physics data between previous and current state
		float alpha = accumulator / dt;
		physics.blendMotionData(alpha);

		renderer.draw(window_size_in_game_units);
	}

	return EXIT_SUCCESS;
}
