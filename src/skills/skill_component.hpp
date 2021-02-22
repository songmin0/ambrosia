#pragma once
#include "skill.hpp"

#include <unordered_map>

enum class SkillType
{
	SKILL1,
	SKILL2,
	SKILL3,
	SKILL4
};

class SkillComponent
{
public:
	SkillComponent() = default;
	~SkillComponent() = default;

	inline void addSkill(SkillType type, const std::shared_ptr<Skill>& skill) {skills[type] = skill;};
	inline void setActiveSkill(SkillType type) {activeType = type;}
	inline std::shared_ptr<Skill> getActiveSkill() {return skills[activeType];}

private:
	std::unordered_map<SkillType, std::shared_ptr<Skill>> skills;
	SkillType activeType;
};
