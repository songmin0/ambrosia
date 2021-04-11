#pragma once
#include <string>
#include "achievement_system.hpp"
#include "../ext/nlohmann/json.hpp"
using json = nlohmann::json;
class LevelLoader
{
	public:
		// Creates all the associated render resources
		json readLevel(const std::string& levelName);


		// save current recipe and level
		void save(const std::string& levelName, int currLevel, int ambrosia, std::list<Achievement> achievements, json skillLevel);

		// load save
		json load();
};