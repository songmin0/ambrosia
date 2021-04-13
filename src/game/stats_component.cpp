#include "stats_component.hpp"

void StatsComponent::setBaseValue(StatType type, float value)
{
	stats[type] = value;
}

float StatsComponent::getStatValue(StatType type)
{
	assert(type != StatType::INVALID);

	float value = stats[type];

	if (statModifiers.find(type) != statModifiers.end())
	{
		value += statModifiers[type].value;
	}

	return value;
}

float StatsComponent::getEffectiveHP()
{
	return getStatValue(StatType::HP) + getStatValue(StatType::HP_SHIELD);
}

bool StatsComponent::isStunned()
{
	// Check that the effective value of STUNNED is greater than zero, accounting
	// for floating point precision
	return std::fabs(getStatValue(StatType::STUNNED)) > 0.0005f;
}

bool StatsComponent::atMaxLevel()
{
	return getStatValue(StatType::LEVEL) == 3;
}

bool StatsComponent::levelUp()
{
	return setLevel(getStatValue(StatType::LEVEL) + 1);
	
}

bool StatsComponent::setLevel(int level)
{
	if (level > 3) {
		return false;
	}
	setBaseValue(StatType::LEVEL, level);

	auto hp = getStatValue(StatType::BASE_HP);
	auto str = getStatValue(StatType::BASE_STR);

	hp *= 1 + (0.1 * ((int)level - 1));
	str *= 1 + (0.1 * ((int)level - 1));

	setBaseValue(StatType::MAX_HP, hp);
	setBaseValue(StatType::HP, hp);

	setBaseValue(StatType::STRENGTH, str);
	
	return true;
}
