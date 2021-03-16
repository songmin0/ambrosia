#pragma once

#include "game/common.hpp"
#include "game/stats_component.hpp"
#include "entities/tiny_ecs.hpp"

class EntityHandler
{
public:
	EntityHandler() = default;
	~EntityHandler() = default;

	virtual void process(ECS::Entity instigator, ECS::Entity target) = 0;
};

// Applies damage to the given target
class DamageHandler : public EntityHandler
{
public:
	DamageHandler(float damage)
		: damage(damage)
	{}

	~DamageHandler() = default;

	void process(ECS::Entity instigator, ECS::Entity target) override;

private:
	float damage;
};

// Heals the given target
class HealHandler : public EntityHandler
{
public:
	HealHandler(float healAmount)
		: healAmount(healAmount)
	{}

	~HealHandler() = default;

	void process(ECS::Entity instigator, ECS::Entity target) override;

private:
	float healAmount;
};

// Buffs/debuffs the given target
class BuffHandler : public EntityHandler
{
public:
	BuffHandler(StatType statType, float value, float timer)
		: statType(statType)
		, value(value)
		, timer(timer)
	{}

	~BuffHandler() = default;

	void process(ECS::Entity instigator, ECS::Entity target) override;

private:
	StatType statType;
	float value;
	float timer;
};
