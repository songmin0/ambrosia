#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"

#include <unordered_map>

const int MAX_NUM_ULT = 2;

enum class StatType
{
	INVALID,
	HP,
	MAX_HP,
	HP_SHIELD, // Should only be set using a BuffEvent (and should never be negative)
	AMBROSIA,
	STRENGTH,
	NUM_ULT_LEFT, // Given to mobs with ultimate attacks
	STUNNED // 0.f if not stunned; 1.f if stunned
};

struct StatModifier
{
	StatType statType;
	float value;
	int numTurns; // Number of turns remaining before the modifier gets removed
};

struct StatsComponent
{
	float getStatValue(StatType type);
	float getEffectiveHP();
	bool isStunned();

	// The actual stats
	std::unordered_map<StatType, float> stats;

	// The stat modifiers. In this initial implementation, stat modifiers are only additive (e.g., if the stat's value is
	// 1.f and the modifier's value is 0.1f, then getStatValue would return 1.1f. Also, this initial implementation only
	// supports one stat modifier per stat, as you can see in this map
	std::unordered_map<StatType, StatModifier> statModifiers;

	ECS::Entity healthBar;
};

struct CCImmunityComponent {};