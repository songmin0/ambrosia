#pragma once
#include "game/common.hpp"

// Data structure for pebble-specific information
namespace DebugSystem {
	extern bool in_debug_mode;

	// draw a red line for debugging purposes
	void createLine(vec2 position, vec2 size);

	// draw a box, centered at the given position
	void createBox(vec2 position, vec2 size);

	// draw a dotted line between two points
	void createDottedLine(vec2 position1, vec2 position2);

	// draw a dotted path along the given points
	void createPath(std::stack<vec2> path);

	// Removes all debugging graphics in ECS, called at every iteration of the game loop
	void clearDebugComponents();
};
