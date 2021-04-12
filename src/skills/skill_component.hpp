#pragma once
#include "skill.hpp"
#include "game/common.hpp"

enum class SkillType
{
	SKILL1,
	SKILL2,
	SKILL3,
	SKILL4,
	MOVE,
	NONE
};

typedef std::vector<std::shared_ptr<Skill>> SkillLevels;

class SkillComponent
{
private:
	struct Entry
	{
		Entry();
		Entry(SkillLevels levels);

		unsigned int currLevel;
		SkillLevels levels;
	};

public:
	SkillComponent();
	~SkillComponent() = default;

	// For mobs. They don't have upgradeable skills, so use this function to set
	// their "level 1" skill for the given skill type
	void addSkill(SkillType type, const std::shared_ptr<Skill>& skill);

	// For players. Adds a list of skills which will correspond to "level 1",
	// "level 2", etc., for the given skill type
	void addUpgradeableSkill(SkillType type, const SkillLevels& levels);

	void setActiveSkill(SkillType type);
	std::shared_ptr<Skill> getActiveSkill();
	SkillType getActiveSkillType();

	unsigned int getActiveSkillLevel();
	unsigned int getSkillLevel(SkillType type);
	unsigned int getMaxLevel(SkillType type);

	void upgradeSkillLevel(SkillType type);

	void setSkillLevel(SkillType type, int level);

	void setAllSkillLevels(json stored_levels);

	json getAllSkillLevels();

private:
	std::unordered_map<SkillType, Entry> skills;
	SkillType activeType;
	int playerLevel;
};
