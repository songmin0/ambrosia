#include "level_loader.hpp"
#include "game/common.hpp"
#include <iostream>
#include <iomanip>
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

void LevelLoader::save(const std::string& levelName, int currLevel,
											 int ambrosia, std::list<Achievement> achievements, json skillLevel) {
	json saveFile;

	saveFile["recipe"] = levelName;
	saveFile["level"] = currLevel;
	saveFile["ambrosia"] = ambrosia;
	saveFile["achievements"] = achievements;
	saveFile["skill_levels"] = skillLevel;

	std::ofstream file("data/save.json");
	file << std::setw(4) << saveFile;
}

json LevelLoader::load() {
	const std::string savePath = "data/save.json";
	json save_obj;
	if (is_file_exist(savePath)) {
		std::ifstream ifs(savePath);

		save_obj = json::parse(ifs);
	}
	else {
		std::cout << "save does not exist!";
		return NULL;
	}

	return save_obj;
}

