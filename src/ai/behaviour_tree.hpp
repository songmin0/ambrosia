#pragma once

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include <vector>
#include <iostream>

using namespace std;

enum class MobType
{
	MOB,
	BOSS
};

enum class Status
{
	RUNNING,
	SUCCESS,
	FAILURE,
	INVALID
};

// Refers to a tree node which stores shared data
class Task
{
protected:
	Node* node;
public:
	Task(Node& node);
	virtual void onInitilize() {};
	virtual Status update() = 0;
	virtual void onTerminate(Status) {};
};

// Factory for creating tasks
class Node
{
public:
	virtual Task* create() = 0;
	virtual void destroy(Task*) = 0;
};

class BehaviourTree
{
protected:
	Behaviour* root;
public:
	void tick();
};

class Behaviour
{
protected:
	virtual void onInitilize() {};
	virtual Status update() = 0;
	virtual void onTerminate(Status) {};
private:
	Status status;
	// TODO: Needed to link Node and Task classes to Behaviour
	Node node;
	Task task;
public:
	Behaviour() : status(Status::INVALID) {};
	~Behaviour() {};
	Status tick()
	{
		if (status != Status::RUNNING)
			onInitilize();
		status = update();
		if (status != Status::RUNNING)
			onTerminate(status);
		return status;
	}
};

class Decorator : public Behaviour
{
protected:
	Behaviour* root_child;
public:
	Decorator(Behaviour* child) : root_child(child) {};
};

class Composite : public Behaviour
{
protected:
	typedef vector<Behaviour*> Behaviours;
	Behaviours children;
public:
	void addChild(Behaviour*);
	void removeChild(Behaviour*);
	void clearChildren();
};

class Sequence : public Composite
{
protected:
	Behaviours::iterator currentChild;
	virtual void onInitialize() override
	{
		currentChild = children.begin();
	}
	virtual Status update() override
	{
		while (true)
		{
			Status s = (*currentChild)->tick();
			if (s != Status::SUCCESS)
				return s;
			// If all nodes return success, sequence operation is a success
			if (++currentChild == children.end())
				return Status::SUCCESS;
		}
		return Status::INVALID;
	}
};

// Comment: May need for attack cooldown filtration?
class Filter : public Sequence
{
public:
	void addCondition(Behaviour* condition)
	{
		children.insert(condition);
	}
	void addAction(Behaviour* action)
	{
		children.push_back(action);
	}
};

class Selector : public Composite
{
protected:
	Behaviours::iterator currentChild;
	virtual void onInitialize() override
	{
		currentChild = children.begin();
	}
	virtual Status update() override
	{
		while (true)
		{
			Status s = (*currentChild)->tick();
			if (s != Status::FAILURE)
				return s;
			// If all nodes return success, sequence operation is a success
			if (++currentChild == children.end())
				return Status::FAILURE;
		}
		return Status::INVALID;
	}
};