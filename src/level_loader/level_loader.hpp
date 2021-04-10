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
		void save(const std::string& recipe, int currLevel,
							int ambrosia, std::list<Achievement> achievementsText);

		// load save
		json load();
};