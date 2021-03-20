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
	POTATO,
	POTATO_CHUNK
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
			std::bind(&StateSystem::onStartMobTurnEvent, this));
		activeTree = nullptr;
	};

	void onStartMobTurnEvent();
	void step(float elapsed_ms);
};

// Check condition before running child; always return SUCCESS
class Conditional : public Node
{
public:
	std::shared_ptr<Node> child;
	bool condition = false;
	void setConditional(std::shared_ptr<Node>, bool);
	virtual void run();
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

// Make milk run away if low HP then heal mob or attack if no allies left
// Root node of Milk BehaviourTree
class MilkTurnSequence : public Sequence
{
public:
	MilkTurnSequence();
	void run();
};

// Composite selector to choose melee or AOE skill
// Root node of Potato BehaviourTree
class PotatoSkillSelector : public Selector
{
public:
	PotatoSkillSelector();
	void run();
};

// Composite sequence of moving to potato or waiting
// Root node of Potato Chunk BehaviourTree
class PotatoChunkTurnSequence : public Sequence
{
public:
	PotatoChunkTurnSequence();
	void run();
};

// Make egg move closer to player or run away (if low HP)
class EggMoveSelector : public Selector
{
public:
	EggMoveSelector();
	void run();
};

// Make pepper move closer to weakest player or farthest player (if all full HP)
class PepperMoveSelector : public Selector
{
public:
	PepperMoveSelector();
	void run();
};

// Run away if low HP; otherwise, don't move
class MilkMoveConditional : public Conditional
{
public:
	MilkMoveConditional();
	void run();
};

// Make milk heal weakest mob or attack (if all full HP)
class MilkSkillSelector : public Selector
{
public:
	MilkSkillSelector();
	void run();
};

class PotatoChunkMoveConditional : public Conditional
{
public:
	PotatoChunkMoveConditional();
	void run();
};

// Run away if low HP; otherwise, don't move
class PotatoChunkMoveSelector : public Selector
{
public:
	PotatoChunkMoveSelector();
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

// Potato BehaviourTree
struct PotatoBehaviourTree : public BehaviourTree
{
public:
	PotatoBehaviourTree();
};

struct PotatoChunkBehaviourTree : public BehaviourTree
{
public:
	PotatoChunkBehaviourTree();
};

// Parent class of all leaf nodes
// Make sure to implement destructors of the correct Event type for children
class Task : public Node
{
public:
	EventListenerInfo taskCompletedListener;
	virtual void onFinishedTaskEvent();
};

// Parent class to all movement tasks
class MoveTask : public Task 
{
public:
	~MoveTask();
};

// Parent class to all skill tasks
class SkillTask : public Task 
{
public:
	~SkillTask();
};

// Task to move closer to closest player
class MoveToClosestPlayerTask : public MoveTask
{
public:
	void run();
};

// Task to move closer to farthest player
class MoveToFarthestPlayerTask : public MoveTask
{
public:
	void run();
};

// Task to move closer to weakest player (low HP)
class MoveToWeakestPlayerTask : public MoveTask
{
public:
	void run();
};

// Task to move closer to weakest mob
// May use later - maybe for protecting
class MoveToWeakestMobTask : public MoveTask
{
public:
	void run();
};

// Task to move closer potato death point
class MoveToDeadPotato : public MoveTask
{
public:
	void run();
};

// Task to run away from closest player
class RunAwayTask : public MoveTask
{
public:
	void run();
};

// Task to use basic attack on closest player
class BasicAttackTask : public SkillTask
{
public:
	void run();
};

// Task to use ultimate attack
class UltimateAttackTask : public SkillTask
{
public:
	void run();
};

// Task to heal closest mob
class HealTask : public SkillTask
{
public:
	void run();
};