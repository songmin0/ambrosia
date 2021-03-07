#pragma once
#include <string>
#include "../ext/nlohmann/json.hpp"
using json = nlohmann::json;
class LevelLoader
{
	public:
		// Creates all the associated render resources
		json readLevel(const std::string& levelName);

};