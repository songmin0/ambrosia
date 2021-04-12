#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"

enum class StatType
{
	INVALID,
	AMBROSIA, // Amount of ambrosia a mob drops when it dies
	HP,
	MAX_HP,
	BASE_HP,
	BASE_STR,
	NUM_ULT_LEFT, // Given to mobs with ultimate attacks
	MAX_NUM_ULT,
	LEVEL,

	// Buffable/debuffable stats
	HP_SHIELD,
	STRENGTH,
	STUNNED // 0.f if not stunned; 1.f if stunned
};

struct StatModifier
{
	StatType statType;
	float value;
	int numTurns; // Number of turns remaining before the modifier gets removed
};

class StatsComponent
{
	friend class StatsSystem;
public:
	StatsComponent() = default;
	~StatsComponent() = default;

	void setBaseValue(StatType type, float value);
	float getStatValue(StatType type);
	float getEffectiveHP();
	bool isStunned();
	bool atMaxLevel();
	bool levelUp();
	bool setLevel(int level);

	ECS::Entity healthBar;

private:
	// The actual stats
	std::unordered_map<StatType, float> stats;

	// The stat modifiers. In this initial implementation, stat modifiers are only additive (e.g., if the stat's value is
	// 1.f and the modifier's value is 0.1f, then getStatValue would return 1.1f. Also, this initial implementation only
	// supports one stat modifier per stat, as you can see in this map
	std::unordered_map<StatType, StatModifier> statModifiers;
};

struct CCImmunityComponent {};