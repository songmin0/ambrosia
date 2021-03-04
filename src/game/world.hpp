#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"
#include "json.hpp"
using json = nlohmann::json;

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <level_loader/level_loader.hpp>

// Container for all our entities and game logic. Individual rendering / update is 
// deferred to the relative update() methods
class WorldSystem
{
public:
	// Creates a window
	WorldSystem(ivec2 window_size_px);

	// Releases all associated resources
	~WorldSystem();

	// restart level
	void restart();

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Check for collisions
	void handleCollisions();

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool isOver() const;

	// OpenGL window handle
	GLFWwindow* window;
private:
	void createMap(int frameBufferWidth, int frameBufferHeight);
	void createButtons(int frameBufferWidth, int frameBufferHeight);
	void createPlayers(int frameBufferWidth, int frameBufferHeight);
	void createMobs(int frameBufferWidth, int frameBufferHeight);

	// Input callback functions
	void onKey(int key, int, int action, int mod);
	void onMouseClick(int button, int action, int mods) const;

	// Loads the audio
	void initAudio();

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Game state
	float current_speed;

	// Player test
	ECS::Entity player_raoul;
	ECS::Entity player_taji;
	ECS::Entity player_ember;
	ECS::Entity player_chia;
	
	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	//config obj
	json config;
};
