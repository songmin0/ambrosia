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
