#pragma once

// stlib
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>
#include <stack>
#include <list>
#include <unordered_map>
#include <utility>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3

// JSON Library 
#include "../ext/nlohmann/json.hpp"
using json = nlohmann::json;

using namespace glm;
static constexpr float PI = 3.14159265359f;
static constexpr float FLOAT_MIN = std::numeric_limits<float>::min();
static constexpr float FLOAT_MAX = std::numeric_limits<float>::max();

// Colour of ambrosia text
static constexpr vec3 AMBROSIA_COLOUR(1.f, 1.f, 0.f);
// Position of AmbrosiaDisplay, relative to top-left corner of screen
static constexpr vec2 AMBROSIA_DISPLAY_OFFSET(100.f, 65.f);

// Simple utility functions to avoid mistyping directory name
inline std::string dataPath() { return "data"; };
inline std::string shaderPath(const std::string& name) { return dataPath() + "/shaders/" + name;};
inline std::string texturesPath(const std::string& name) { return dataPath() + "/textures/" + name; };
inline std::string audioPath(const std::string& name) { return dataPath() + "/audio/" + name; };
inline std::string meshPath(const std::string& name) { return dataPath() + "/meshes/" + name; };
inline std::string spritePath(const std::string& name) { return dataPath() + "/sprites/" + name; };
inline std::string mapsPath(const std::string& name) { return dataPath() + "/maps/" + name; };
inline std::string objectsPath(const std::string& name) { return dataPath() + "/objects/" + name; };
inline std::string uiPath(const std::string& name) { return dataPath() + "/ui/" + name; };
inline std::string levelsPath(const std::string& name) { return dataPath() + "/levels/" + name; };
inline std::string fxPath(const std::string& name) { return dataPath() + "/effects/" + name; };
inline std::string fontPath(const std::string& name) { return dataPath() + "/fonts/" + name; };

enum class PlayerType { RAOUL, TAJI, EMBER, CHIA };
float playerToFloat(const PlayerType& player);

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

//////////////////////////////////////////
// Collider flags
enum CollisionGroup : unsigned int
{
	NONE = 0u,
	PLAYER = 1u << 0u,
	MOB = 1u << 1u,
	ALL = ~0u
};
constexpr CollisionGroup operator&(CollisionGroup a, CollisionGroup b)
{
	return static_cast<CollisionGroup>(
			static_cast<std::underlying_type_t<CollisionGroup>>(a) &
			static_cast<std::underlying_type_t<CollisionGroup>>(b));
}
constexpr CollisionGroup operator|(CollisionGroup a, CollisionGroup b)
{
	return static_cast<CollisionGroup>(
			static_cast<std::underlying_type_t<CollisionGroup>>(a) |
			static_cast<std::underlying_type_t<CollisionGroup>>(b));
}
//////////////////////////////////////////

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = vec2(0.f);
	vec2 velocity = vec2(0.f);
	float angle = 0.f;

	// Values from previous simulation step
	vec2 prevPosition = vec2(FLOAT_MIN);
	float prevAngle = FLOAT_MIN;

	// Interpolated between previous and current
	vec2 renderPosition = vec2(0.f);
	float renderAngle = 0.f;

	vec2 scale = vec2(1.f);
	vec2 boundingBox = vec2(0.f);
	float moveRange = 100.f;
	float mass = 1.f;

	// (orientation * scale.x) faces right when positive 
	// ie. if a sprite's texture faces left, then orientation should be -1
	float orientation = 1.f;

	std::stack<vec2> path;

	CollisionGroup colliderType = CollisionGroup::NONE;
};

//PlaceHolder please replace with the real one once someone has made them or continue to use these but rename
struct PlayerComponent {
	PlayerType player;
};

enum MusicType
{
	START_SCREEN,
	SHOP,
	VICTORY,
	BOSS,
	PIZZA_ARENA,
	DESSERT_ARENA,
	PLACEHOLDER1,
	PLACEHOLDER2,
	PLACEHOLDER3,
	PLACEHOLDER4,
	PLACEHOLDER5,

	// The `LAST` value is only needed because WorldSystem::playNextAudioTrack_DEBUG()
	// needs to be able to loop through the music.
	LAST
};

enum class SoundEffect
{
	NONE,
	MOUSE_CLICK,
	TURN_START,
	GAME_OVER,
	HIT_PLAYER,
	HIT_MOB,
	DEFEAT,
	MELEE,
	PROJECTILE,
	BUFF,
	DEBUFF
};

enum class FXType
{
	NONE,
	BUFFED,
	DEBUFFED,
	HEALED,
	SHIELDED,
	CANDY1,
	CANDY2,
	BLUEBERRIED,
	STUNNED
};

// Converts floating point dmg number to a string up to 2 decimal places with no trailing zeros
std::string floatToString(float number);

static const vec3 RED = vec3(1.0, 0.0, 0.0);
static const vec3 GREEN = vec3(0.0, 1.0, 0.0);
static const vec3 BLUE = vec3(0.0, 0.2, 1.0);
static const vec3 CYAN = vec3(0.0, 1.0, 1.0);
static const vec3 ORANGE = vec3(0.8, 0.5, 0.0);
static const vec3 PINK = vec3(1.0, 0.5, 0.7);