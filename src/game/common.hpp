#pragma once

// stlib
#include <string>
#include <tuple>
#include <vector>
#include <stdexcept>
#include <stack>

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
static const float PI = 3.14159265359f;

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

enum class PlayerType { RAOUL, TAJI, CHIA, EMBER };
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
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0, 0 };
	vec2 scale = { 1, 1 };
	vec2 boundingBox = { 0, 0 };
	float moveRange = 100.f;

	// (orientation * scale.x) faces right when positive 
	// ie. if a sprite's texture faces left, then orientation should be -1
	float orientation = 1;

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