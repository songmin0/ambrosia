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
	EGG
};

struct BehaviourTreeType
{
	MobType mobType;
};

// Nodes of a BehaviourTree; Either a task or composite node
class Node
{
public:
	Status status = Status::INVALID;
	//public:
	virtual void run() { status = Status::RUNNING; };
	virtual void onTerminate(Status s) { status = s; };
};

// Parent class for all mob BehaviourTrees
class BehaviourTree
{
public:
	std::shared_ptr<Node> root;
	//public:
	BehaviourTree() : root(new Node) {};
	BehaviourTree(std::shared_ptr<Node> root) : root(root) {};
	~BehaviourTree() { root = nullptr; };
	void run() { root->run(); };
};

// Runs through BehaviourTree of active mob entity
class StateSystem
{
public:
	EventListenerInfo startMobTurnListener;
	//static ECS::Entity currentMobEntity;
	std::shared_ptr<BehaviourTree> activeTree;
//public:
	StateSystem()
	{
		startMobTurnListener = EventSystem<StartMobTurnEvent>::instance().registerListener(
			std::bind(&StateSystem::onStartMobTurnEvent, this, std::placeholders::_1));
		activeTree = nullptr;
		//currentMobEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
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
//public:
	//vector<Node*> getChildren() { return children; };
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

// Composite sequence of moving (selection) and attacking
class MobTurnSequence : public Sequence
{
public:
	MobTurnSequence();
	void run();
};

// Make mob move closer to player or run away (if low HP)
class MoveSelector : public Selector
{
private:
	// HP of active mob entity
	float hp;
public:
	MoveSelector(float);
	void run();
};

struct Task : public Node
{
	EventListenerInfo taskCompletedListener;
};

// Task to move closer to closest player
class MoveCloserTask : public Task
{
public:
	~MoveCloserTask();
	void onFinishedMoveCloserEvent(const FinishedMovementEvent& event);
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

// Egg mob Behaviour Tree
// Root node is a MobTurnSequence
struct EggBehaviourTree : public BehaviourTree
{
public:
	EggBehaviourTree();
};