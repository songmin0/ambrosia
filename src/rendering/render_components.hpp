#pragma once
#include "game/common.hpp"

#include <vector>
#include <unordered_map>
#include "stb_image.h"

enum GLResourceType {BUFFER, RENDER_BUFFER, SHADER, PROGRAM, TEXTURE, VERTEX_ARRAY};

// Enum for rendering layer order (first element is rendered on top, last is on bottom)
enum class RenderLayer {
	DEBUG,
	CLICK_FX,
	HELP_BUTTON,
	UI_TUTORIAL1,
	UI_TOOLTIP,
	UI_TUTORIAL2,
	UI,
	UI_ACTIVE_SKILL_FX,
	RANGE_INDICATOR,
	MAP_FOREGROUND,
	PROJECTILE,
	SKILL,
	PLAYER_AND_MOB,
	MAP_OBJECT,
	MAP,
	MAP2,
	MAP_BACKGROUND
};

// This class is a wrapper around OpenGL resources that deletes allocated memory on destruction.
// Moreover, copy constructors are disabled to ensure that the resource is only deleted when the original object is destroyed, not its copies.
template <GLResourceType Resource>
struct GLResource
{
	GLuint resource = 0;

	///////////////////////////////////////////////////
	// Constructors and operators to mimick GLuint
	operator GLuint() const {
		return resource;
	};
	GLuint* data() {
		return &resource;
	};
	bool operator== (const GLuint& other) { return resource == other; }
	bool operator!= (const GLuint& other) { return resource != other; }
	bool operator<= (const GLuint& other) { return resource <= other; }
	bool operator>= (const GLuint& other) { return resource >= other; }
	bool operator< (const GLuint& other)  { return resource < other; }
	bool operator> (const GLuint& other) { return resource > other; }
	GLResource& operator= (const GLuint& other) { resource = other; return *this;}
	GLResource() = default;
	GLResource(const GLuint& other) {
		this->resource = other;
	};
	// Destructor that frees OpenGL resources, specializations for each supported type are in the .cpp
	~GLResource() noexcept;

	///////////////////////////////////////////////////
	// Operators that maintain exactly one indstance of every resources, e.g., OpenGL vertex buffer
	// Prevent copy
	GLResource(const GLResource& other) = delete; // copy constructor disabled
	GLResource& operator=(const GLResource&) = delete; // copy assignment disabled
	// Move constructor that invalidates the .bo in the source after copying
	GLResource(GLResource&& source) noexcept {
		// same as this->resource = source.resource; source.resource = 0;
		this->resource = std::exchange(source.resource, 0);
	};
	// Move operator that exchanges source.bo and this->bo, such that the this->bo lives on until destruction of the source
	GLResource& operator=(GLResource&& source) noexcept {
		// The exchange makes sure that the vbo and ibo of the target aren't overwritten in a move,
		// such that the destructor is called on every instanciated object exactly once. Leaving the 'other'
		// value unchanged would create a dublicated of it. Not exchanging the 'this' value would erase it without
		// release of associated resources (e.g., OpenGL buffers).
		if (this != &source)
			this->resource = std::exchange(source.resource, this->resource);
		return *this;
	};
};

// Single Vertex Buffer element for non-textured meshes (colored_mesh.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Texture wrapper
struct Texture
{
	GLResource<TEXTURE> texture_id;
	ivec2 size = {0, 0};
	vec3 color = {1,1,1};

	// for animation
	int frames = 1;
	
	// Loads texture from file specified by path
	void loadFromFile(const std::string& path);
	bool isValid() const; // True if texture is valid
	void createFromScreen(GLFWwindow const * const window, GLuint* depth_render_buffer_id); // Screen texture

	void loadArrayFromFile(const std::string& path, int maxFrames);
	void loadPlayerSpecificTextures(const std::string& path);

	std::unordered_map<std::string, stbi_uc*> texture_cache;
};

// Effect component for Vertex and Fragment shader, which are then put(linked) together in a
// single program that is then bound to the pipeline.
struct Effect
{
	GLResource<SHADER> vertex;
	GLResource<SHADER> fragment;
	GLResource<PROGRAM> program;

	void loadFromFile(const std::string& vs_path, const std::string& fs_path); // load shaders from files and link into program
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	void loadFromOBJFile(const std::string& obj_path);
	//void LoadFromTexturedOBJFile(std::string obj_path);
	vec2 original_size = {1.f,1.f};
	GLResource<BUFFER> vbo;
	GLResource<BUFFER> ibo;
	GLResource<VERTEX_ARRAY> vao;
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct ScreenState
{
	float darken_screen_factor = -1;
};

// ShadedMesh datastructure for storing mesh, shader, and texture objects
struct ShadedMesh
{
	Mesh mesh;
	Effect effect;
	Texture texture;
};

// Cache for ShadedMesh resources (mesh consisting of vertex and index buffer, the vertex and fragment shaders, and the texture)
ShadedMesh& cacheResource(std::string key);

// A wrapper that points to the ShadedMesh in the resource_cache
struct ShadedMeshRef
{
	ShadedMesh* reference_to_cache;
	ShadedMeshRef(ShadedMesh& mesh);
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying players/mobs
struct DeathTimer
{
	float counter_ms = 1500;
	void CustomDeathTimer(float counter_ms);
};

struct ColourShift
{
	// values 1-4 will set colours
	// other values will do nothing
	float colour = 0;
};

struct VisibilityComponent
{
	bool isVisible = true;
};

struct RenderableComponent
{
	RenderLayer layer;
	RenderableComponent(RenderLayer layer);
};

struct DistendableComponent
{
	float xamplitude = 0.1f;
	float xfrequency = 0.2f;
	float yamplitude = 0.2f;
	float yfrequency = 0.5f;
	DistendableComponent() : xamplitude{ 0.1f }, xfrequency{ 0.2f }, yamplitude{ 0.2f }, yfrequency{ 0.5f } {};
	DistendableComponent(float xA, float xF, float yA, float yF) : xamplitude{ xA }, xfrequency{ xF }, yamplitude{ yA }, yfrequency{ yF } {};
};

struct TimerComponent
{
	float maxTime_ms = 1000.f;
	float counter_ms = 0.f;
	bool isCountingUp = true;
	bool complete = false;
	TimerComponent() : maxTime_ms{ 1000.f }, counter_ms{ 0.f }, isCountingUp{ true }, complete{ false } {};
	TimerComponent(float maxTime_ms)
		: maxTime_ms{ maxTime_ms }, counter_ms{ 0.f }, isCountingUp{ true }, complete{ false } {};
	TimerComponent(float maxTime_ms, float initCounter_ms, bool isCountingUp) 
		: maxTime_ms{ maxTime_ms }, counter_ms{ initCounter_ms }, isCountingUp{ isCountingUp }, complete{ false } {};
};