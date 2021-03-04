#pragma once
#include <string>
#include "json.hpp"
using json = nlohmann::json;
class LevelComponent
{
	public:

		struct configObject {        // Declare a local structure 
			std::string map_path;

		};

		// Creates all the associated render resources
		json readLevel(const std::string& levelName);

};