#include "level_loader.hpp"
#include <game/common.hpp>
#include <iostream>
#include <fstream>
#include <sys/stat.h> 

bool is_file_exist(const std::string& name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

json LevelLoader::readLevel(const std::string& levelName) {
	const std::string levelPath =  levelsPath(levelName + ".json");
	json config_obj;
	if (is_file_exist(levelPath)) {
		std::ifstream ifs(levelPath);

		config_obj = json::parse(ifs);
	}
	else {
		std::cout << "level does not exist";
	}
	
	return config_obj;

}
