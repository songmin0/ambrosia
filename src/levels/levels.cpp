#include "levels.hpp"
#include "json.hpp"
#include <game/common.hpp>
#include <iostream>
#include <fstream>
#include  <sys/stat.h> 

// RENAME AS LOADER __________
using json = nlohmann::json;
bool is_file_exist(const std::string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

json LevelComponent::readLevel(const std::string& levelName) {
	const std::string levelPath = "../../" + levelsPath(levelName + ".json");
	json config_obj;
	if (is_file_exist(levelPath)) {
		std::ifstream ifs(levelPath);

		config_obj = json::parse(ifs);
	}
	else {
		std::cout << "level does not exist";
	}

	struct LevelComponent::configObject co;
	
	return config_obj;

}

