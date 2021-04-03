#pragma once

#include "game/common.hpp"
#include "game/stats_component.hpp"
#include "entities/tiny_ecs.hpp"

class EntityHandler
{
public:
	EntityHandler() = default;
	virtual ~EntityHandler() = default;

	void process(ECS::Entity instigator, ECS::Entity target);
	inline void addFX(FXType fxType) {this->fxTypes.push_back(fxType);}

private:
	virtual void processInternal(ECS::Entity instigator, ECS::Entity target) = 0;

	std::vector<FXType> fxTypes;
};

// Applies damage to the given target
class DamageHandler : public EntityHandler
{
public:
	DamageHandler(float damage)
		: damage(damage)
	{}

	~DamageHandler() override = default;

	void processInternal(ECS::Entity instigator, ECS::Entity target) override;

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

	~HealHandler() override = default;

	void processInternal(ECS::Entity instigator, ECS::Entity target) override;

private:
	float healAmount;
};

// Buffs/debuffs the given target
class BuffHandler : public EntityHandler
{
public:
	BuffHandler(StatType statType, float value, int numTurns)
		: statType(statType)
		, value(value)
		, numTurns(numTurns)
	{}

	~BuffHandler() override = default;

	void processInternal(ECS::Entity instigator, ECS::Entity target) override;

private:
	StatType statType;
	float value;
	int numTurns;
};

// Heals one type of entity and damages the other type
class HealAndDamageHandler : public EntityHandler
{
public:
	HealAndDamageHandler(CollisionGroup typeToHeal, float healAmount,
											 CollisionGroup typeToDamage, float damage)
		: typeToHeal(typeToHeal)
		, healAmount(healAmount)
		, typeToDamage(typeToDamage)
		, damage(damage)
	{}

	~HealAndDamageHandler() override = default;

	void processInternal(ECS::Entity instigator, ECS::Entity target) override;

private:
	CollisionGroup typeToHeal;
	float healAmount;

	CollisionGroup typeToDamage;
	float damage;
};

// Debuffs and damages the given target. You could also use it to buff and
// damage the target, of course, but that wouldn't make much sense.
class DebuffAndDamageHandler : public EntityHandler
{
public:
	DebuffAndDamageHandler(StatType statType, float value, int numTurns,
												 float damage)
		: statType(statType)
		, value(value)
		, numTurns(numTurns)
		, damage(damage)
	{}

	~DebuffAndDamageHandler() override = default;

	void processInternal(ECS::Entity instigator, ECS::Entity target) override;

private:
	StatType statType;
	float value;
	int numTurns;

	float damage;
};

class KnockbackHandler : public EntityHandler
{
public:
	KnockbackHandler(float radius, float maxForce, float maxDamage)
		: radius(radius)
		, maxForce(maxForce)
		, maxDamage(maxDamage)
	{}

	~KnockbackHandler() override = default;

	void processInternal(ECS::Entity instigator, ECS::Entity target) override;

private:
	float radius;
	float maxForce;
	float maxDamage;
};
