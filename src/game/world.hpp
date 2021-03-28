#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"
#include "../ext/nlohmann/json.hpp"
#include "event_system.hpp"
#include "events.hpp"
#include <functional>

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

	// preload resource cache
	void preloadResources();

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Check for collisions
	void handleCollisions();

	// Should the game be over ?
	bool isOver() const;

	// OpenGL window handle
	GLFWwindow* window;
private:
	void createMap(int frameBufferWidth, int frameBufferHeight);
	void createButtons(int frameBufferWidth, int frameBufferHeight);
	void createPlayers(int frameBufferWidth, int frameBufferHeight);
	void createMobs(int frameBufferWidth, int frameBufferHeight);
	void createEffects(int frameBufferWidth, int frameBufferHeight);

	// Input callback functions
	void onKey(int key, int, int action, int mod);
	void onMouseClick(int button, int action, int mods) const;
	void onMouseHover(double xpos, double ypos) const;

	// Music and sound effects
	void initAudio();
	void releaseAudio();
	void playAudio();
	void playNextAudioTrack_DEBUG();
	void onPlaySoundEffectEvent(const PlaySoundEffectEvent& event);
	void onPlayerChangeEvent(const PlayerChangeEvent& event);
	void processTimers(float elapsed_ms);

	// Level loading
	void onLoadLevelEvent(const LoadLevelEvent& event);

	// Mouse cursor
	void initCursors();
	void releaseCursors();
	void onSetActiveSkillEvent(const SetActiveSkillEvent& event);
	void onFinishedMovementEvent(const FinishedMovementEvent& event);
	void onFinishedSkillEvent(const FinishedSkillEvent& event);

	// Player test
	ECS::Entity playerRaoul;
	ECS::Entity playerTaji;
	ECS::Entity playerEmber;
	ECS::Entity playerChia;
	
	// Music and sound effects
	std::unordered_map<MusicType, Mix_Music*> music;
	std::unordered_map<SoundEffect, Mix_Chunk*> soundEffects;
	MusicType currentMusic_DEBUG;
	EventListenerInfo soundEffectListener;
	EventListenerInfo playerChangeListener;

	// A hack to prevent playing the TURN_START sound effect when the game first starts
	bool shouldPlayAudioAtStartOfTurn;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Level loading
	LevelLoader lc;
	//config obj
	//json config;
	EventListenerInfo loadLevelListener;

	void onTransition(TransitionEvent);
	EventListenerInfo transitionEventListener;
	void (*transition)();

	// Mouse cursor
	GLFWcursor* moveCursor = nullptr;
	GLFWcursor* skillCursor = nullptr;
	EventListenerInfo setActiveSkillListener;
	EventListenerInfo finishedMovementListener;
	EventListenerInfo finishedSkillListener;
};
