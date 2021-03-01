
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "game/common.hpp"
#include "game/world.hpp"
#include "game/turn_system.hpp"
#include "entities/tiny_ecs.hpp"
#include "rendering/render.hpp"
#include "physics/physics.hpp"
#include "physics/debug.hpp"
#include "physics/projectile_system.hpp"
#include "ai/ai.hpp"
#include "animation/animation_system.hpp"
#include "maps/path_finding_system.hpp"
#include "ui/ui_system.hpp"
#include "particle_system.hpp"
#include "skills/skill_system.hpp"
#include "game/stats_system.hpp"


using Clock = std::chrono::high_resolution_clock;

const ivec2 window_size_in_px = {1280, 1024};
const vec2 window_size_in_game_units = { 1280, 1024};
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
	particle_system particleSystem;
	RenderSystem renderer(*world.window, &particleSystem);
	PhysicsSystem physics;
	PathFindingSystem pathFindingSystem;
	AISystem ai(pathFindingSystem);
	TurnSystem turnSystem(pathFindingSystem);
	AnimationSystem animations;
	UISystem ui;
	ProjectileSystem projectileSystem;
	SkillSystem skillSystem;
	StatsSystem statsSystem;

	// Set all states to default
	world.restart();

	float dtMax = (1.f / 60.f) * 1000.f; // 60 FPS

	auto t = Clock::now();
	// Variable timestep loop
	while (!world.isOver())
	{
		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms = static_cast<float>((std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count()) / 1000.f;
		t = now;

		while (elapsed_ms > 0.f)
		{
			// Reference: https://gafferongames.com/post/fix_your_timestep/#semi-fixed-timestep
			float deltaTime = std::min(elapsed_ms, dtMax);

			// Processes system messages, if this wasn't present the window would become unresponsive
			glfwPollEvents();

			DebugSystem::clearDebugComponents();
			ai.step(deltaTime, window_size_in_game_units);
			world.step(deltaTime, window_size_in_game_units);
			physics.step(deltaTime, window_size_in_game_units);
			world.handleCollisions();
			projectileSystem.step(deltaTime);
			skillSystem.step(deltaTime);
			statsSystem.step(deltaTime);
			animations.step();
			turnSystem.step(deltaTime);
			particleSystem.step(deltaTime);
			renderer.draw(window_size_in_game_units);


			elapsed_ms -= deltaTime;
		}
	}

	return EXIT_SUCCESS;
}
