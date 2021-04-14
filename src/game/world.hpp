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

	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms, vec2 window_size_in_game_units);

	// Check for collisions
	void handleCollisions();

	// Should the game be over ?
	bool isOver() const;

	// OpenGL window handle
	GLFWwindow* window;
private:
	// Input callback functions
	void onKey(int key, int, int action, int mod);
	void onMouseClick(int button, int action, int mods) const;
	void onMouseHover(double xpos, double ypos) const;

	// Music and sound effects
	void initAudio();
	void releaseAudio();
	void playNextAudioTrack_DEBUG();
	void onPlayMusicEvent(const PlayMusicEvent& event);
	void onPlaySoundEffectEvent(const PlaySoundEffectEvent& event);
	void onPlayerChangeEvent(const PlayerChangeEvent& event);
	void processTimers(float elapsed_ms);

	// Mouse cursor
	void initCursors();
	void releaseCursors();
	void onSetActiveSkillEvent(const SetActiveSkillEvent& event);
	void onFinishedMovementEvent(const FinishedMovementEvent& event);
	void onFinishedSkillEvent(const FinishedSkillEvent& event);
	void onResetMouseCursorEvent(const ResetMouseCursorEvent& event);
	void onEnterInspectMode();
	void onExitInspectMode();

	// Music and sound effects
	std::unordered_map<MusicType, Mix_Music*> music;
	std::unordered_map<SoundEffect, Mix_Chunk*> soundEffects;
	MusicType currentMusic_DEBUG;
	EventListenerInfo musicListener;
	EventListenerInfo soundEffectListener;
	EventListenerInfo playerChangeListener;

	// A hack to prevent playing the TURN_START sound effect when the game first starts
	bool shouldPlayAudioAtStartOfTurn;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	void onTransition(TransitionEvent);
	EventListenerInfo transitionEventListener;
	void (*transition)();

	// Mouse cursor
	GLFWcursor* moveCursor = nullptr;
	GLFWcursor* skillCursor = nullptr;
	GLFWcursor* inspectCursor = nullptr;
	int preInspectCursorType = 0; // 0 default, 1 move, 2 skill
	EventListenerInfo setActiveSkillListener;
	EventListenerInfo finishedMovementListener;
	EventListenerInfo finishedSkillListener;
	EventListenerInfo resetMouseCursorListener;
	EventListenerInfo enterInspectModeListener;
	EventListenerInfo exitInspectModeListener;
};
