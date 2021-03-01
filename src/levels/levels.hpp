#pragma once
#include <string>
struct LevelComponent
{
	// Creates all the associated render resources
	static void readLevel(const std::string& levelPath);
};