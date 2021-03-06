#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"

#include <unordered_map>

enum class StatType
{
	HP,
	AMBROSIA,
	STRENGTH
};

struct StatModifier
{
	StatType statType;
	float value;
	float timer; // When possible, change this so that StatModifiers expire after a certain number of turns, not seconds
};

struct StatsComponent
{
	float getStatValue(StatType type);

	// The actual stats
	std::unordered_map<StatType, float> stats;

	// The stat modifiers. In this initial implementation, stat modifiers are only additive (e.g., if the stat's value is
	// 1.f and the modifier's value is 0.1f, then getStatValue would return 1.1f. Also, this initial implementation only
	// supports one stat modifier per stat, as you can see in this map
	std::unordered_map<StatType, StatModifier> statModifiers;

	ECS::Entity healthBar;
};