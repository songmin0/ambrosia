#pragma once

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "game/events.hpp"
#include "game/event_system.hpp"
#include "game/turn_system.hpp"
#include <vector>
#include <iostream>

using namespace std;

enum class Status
{
	RUNNING,
	SUCCESS,
	FAILURE,
	INVALID
};

enum class MobType
{
	EGG,
	PEPPER,
	MILK,
	POTATO
};

struct BehaviourTreeType
{
	MobType mobType;
};

// Nodes of a BehaviourTree; Either a task or composite node
class Node
{
protected:
	Status status = Status::INVALID;
public:
	virtual void run() { status = Status::RUNNING; };
	virtual void onTerminate(Status s) { status = s; };
	virtual Status getStatus() { return status; };
};

// Parent class for all mob BehaviourTrees
class BehaviourTree
{
public:
	std::shared_ptr<Node> root;
	BehaviourTree() : root(new Node) {};
	BehaviourTree(std::shared_ptr<Node> root) : root(root) {};
	~BehaviourTree() { root = nullptr; };
	void run() { root->run(); };
};

// Runs through BehaviourTree of active mob entity
class StateSystem
{
protected:
	EventListenerInfo startMobTurnListener;
	std::shared_ptr<BehaviourTree> activeTree;
public:
	StateSystem()
	{
		startMobTurnListener = EventSystem<StartMobTurnEvent>::instance().registerListener(
			std::bind(&StateSystem::onStartMobTurnEvent, this, std::placeholders::_1));
		activeTree = nullptr;
	};

	void onStartMobTurnEvent(const StartMobTurnEvent& event);
	//static ECS::Entity getCurrentMobEntity() { return currentMobEntity; };
	void step(float elapsed_ms);
};

// Parent class for all composite nodes
class Composite : public Node
{
public:
	vector<std::shared_ptr<Node>> children;
	// Current node index to run
	int i = 0;
	void addChild(std::shared_ptr<Node>);
};

// Type of composite node; tries to run one task
class Selector : public Composite
{
public:
	virtual void run();
};

// Type of composite node; tries to run all children tasks
class Sequence : public Composite
{
public:
	virtual void run();
};

// Composite sequence of moving (closer or away) and attacking
// Root node of Egg BehaviourTree
class EggTurnSequence : public Sequence
{
public:
	EggTurnSequence();
	void run();
};

// Composite sequence of moving closer and attacking
// Root node of Pepper BehaviourTree
class PepperTurnSequence : public Sequence
{
public:
	PepperTurnSequence();
	void run();
};

// Make milk heal closest mob or attack closest player (if no allies left)
// Root node of Milk BehaviourTree
class MilkTurnSelector : public Selector
{
private:
	bool alliesLeft;
public:
	MilkTurnSelector(bool);
	void run();
};

// Make egg move closer to player or run away (if low HP)
class EggMoveSelector : public Selector
{
private:
	// HP of active mob entity
	float hp;
public:
	EggMoveSelector(float);
	void run();
};

// Make milk move closer to mob and heal
class MilkHealSequence : public Sequence
{
public:
	MilkHealSequence();
	void run();
};

// Make milk move closer to player and attack
class MilkAttackSequence : public Sequence
{
public:
	MilkAttackSequence();
	void run();
};

// Make milk move closer to player or run away (if low HP)
class MilkMoveSelector : public Selector
{
private:
	// HP of active mob entity
	float hp;
public:
	MilkMoveSelector(float);
	void run();
};

// Egg BehaviourTree
struct EggBehaviourTree : public BehaviourTree
{
public:
	EggBehaviourTree();
};

// Pepper BehaviourTree
struct PepperBehaviourTree : public BehaviourTree
{
public:
	PepperBehaviourTree();
};

// Milk BehaviourTree
struct MilkBehaviourTree : public BehaviourTree
{
public:
	MilkBehaviourTree();
};

struct Task : public Node
{
	EventListenerInfo taskCompletedListener;
};

// Task to move closer to closest player
class MoveToPlayerTask : public Task
{
public:
	~MoveToPlayerTask();
	void onFinishedMoveToPlayerEvent(const FinishedMovementEvent& event);
	void run();
};

// Task to move closer to closest mob
class MoveToMobTask : public Task
{
public:
	~MoveToMobTask();
	void onFinishedMoveToMobEvent(const FinishedMovementEvent& event);
	void run();
};

// Task to run away from closest player
class RunAwayTask : public Task
{
public:
	~RunAwayTask();
	void onFinishedRunAwayEvent(const FinishedMovementEvent& event);
	void run();
};

// Task to attack closest player
class AttackTask : public Task
{
public:
	~AttackTask();
	void onFinishedAttackEvent(const FinishedSkillEvent& event);
	void run();
};

// Task to heal closest mob
class HealTask : public Task
{
public:
	~HealTask();
	void onFinishedHealEvent(const FinishedSkillEvent& event);
	void run();
};