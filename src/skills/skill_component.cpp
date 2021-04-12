#include "skill_component.hpp"

///////////////////////////////////////////////////////////////////////////////
SkillComponent::Entry::Entry()
	: currLevel(1)
{}

SkillComponent::Entry::Entry(SkillLevels levels)
		: currLevel(1)
		, levels(std::move(levels))
{}

///////////////////////////////////////////////////////////////////////////////
SkillComponent::SkillComponent()
	: activeType(SkillType::NONE)
{}

void SkillComponent::addSkill(SkillType type, const std::shared_ptr<Skill>& skill)
{
	// Since this function will only be used for mobs from now on, just making
	// sure that we don't accidentally try to give a mob an upgradeable skill
	assert(skills.count(type) == 0);

	skills[type].levels.push_back(skill);
}

void SkillComponent::addUpgradeableSkill(SkillType type, const SkillLevels& levels)
{
	skills[type] = Entry(levels);
}

void SkillComponent::setActiveSkill(SkillType type)
{
	activeType = type;
}

std::shared_ptr<Skill> SkillComponent::getActiveSkill()
{
	return skills[activeType].levels[getActiveSkillLevel() - 1];
}

SkillType SkillComponent::getActiveSkillType()
{
	return activeType;
}

unsigned int SkillComponent::getActiveSkillLevel()
{
	return skills[activeType].currLevel;
}

unsigned int SkillComponent::getSkillLevel(SkillType type)
{
	assert(skills.count(type) > 0);
	return skills[type].currLevel;
}

unsigned int SkillComponent::getMaxLevel(SkillType type)
{
	assert(skills.count(type) > 0);
	return skills[type].levels.size();
}

void SkillComponent::upgradeSkillLevel(SkillType type)
{
	assert(skills.count(type) > 0);

	skills[type].currLevel = skills[type].currLevel + 1;
	skills[type].currLevel = std::min(skills[type].currLevel, getMaxLevel(type));
}

void SkillComponent::setSkillLevel(SkillType type, int level)
{
	assert(skills.count(type) > 0);

	skills[type].currLevel = level;
	skills[type].currLevel = std::min(skills[type].currLevel, getMaxLevel(type));
}

void SkillComponent::setAllSkillLevels(json stored_levels)
{
	skills[SkillType::SKILL1].currLevel = stored_levels[0];
	skills[SkillType::SKILL2].currLevel = stored_levels[1];
	skills[SkillType::SKILL3].currLevel = stored_levels[2];
}

json SkillComponent::getAllSkillLevels()
{
	// ya its hard coded dont @ me
	return { skills[SkillType::SKILL1].currLevel , skills[SkillType::SKILL2].currLevel  ,skills[SkillType::SKILL3].currLevel  };
}