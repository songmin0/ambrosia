#include "stats_component.hpp"

float StatsComponent::getStatValue(StatType type)
{
	float value = stats[type];

	if (statModifiers.find(type) != statModifiers.end())
	{
		value += statModifiers[type].value;
	}

	return value;
}
