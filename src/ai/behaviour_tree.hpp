#pragma once

#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
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

// Factory for creating tasks
class Node
{
public:
	virtual Task* create() = 0;
	virtual void destroy(Task*) = 0;
	virtual ~Node() {};
};

// Refers to a tree node which stores shared data
class Task
{
protected:
	Node* node;
public:
	Task(Node& node) {};
	virtual ~Task() {};
	virtual void onInitilize() {};
	virtual Status update() = 0;
	virtual void onTerminate(Status) {};
};

class Behaviour
{
protected:
	Status status;
	Node* node;
	Task* task;
public:
	Behaviour() : task(NULL), node(NULL), status(Status::INVALID) {};
	Behaviour(Nide& node) : task(NULL), node(NULL), status(Status::INVALID)
	{
		setup(node);
	};
	~Behaviour()
	{
		status = Status::INVALID;
		teardown();
	};
	void setup(Node& node)
	{
		teardown();
		node = &node;
		task = node.create();
	}
	void teardown()
	{
		if (task == NULL)
			return;
		assert(status != Status::RUNNING);
		node->destroy(task);
		task = NULL;
	}
	Status tick()
	{
		if (status = Status::INVALID)
			task->onInitilize();
		status = update();
		if (status != Status::RUNNING)
			task->onTerminate(status);
		return status;
	}

	template <class TASK>
	TASK* get() const
	{
		return dynamic_cast<TASK*>(task);
	}
};

//class BehaviourTree
//{
//protected:
//	Behaviour* root;
//public:
//	void tick();
//};

class Decorator : public Behaviour
{
protected:
	Behaviour* root_child;
public:
	Decorator(Behaviour* child) : root_child(child) {};
};

// ==============================================================

typedef vector<Node*> Nodes;

class Composite : public Node
{
public:
	Nodes children;
};

class Sequence : public Task
{
protected:
	Nodes::iterator currentChild;
	Behaviour currentBehaviour;
public:
	Sequence(Composite& node) : Task(node) {};
	Composite& getNode()
	{
		return *static_cast<Composite*>(node);
	}
	virtual void onInitialize()
	{
		currentChild = getNode().children.begin();
		currentBehaviour.setup(**currentChild);
	}
	virtual Status update()
	{
		for (;;)
		{
			Status s = currentBehaviour.tick();
			if (s != Status::SUCCESS)
				return s;
			// If all nodes return success, sequence operation is a success
			if (++currentChild == getNode().children.end())
				return Status::SUCCESS;
		}
		currentBehaviour.setup(**currentChild);
	}
};

class Selector : public Task
{
protected:
	Nodes::iterator currentChild;
	Behaviour behaviour;
public:
	Selector(Composite& node) : Task(node) {};
	Composite& getNode() { return *static_cast<Composite*>(node); };

	virtual void onInitialize() override
	{
		currentChild = getNode().children.begin();
		behaviour.setup(**currentChild);
	}
	virtual Status update() override
	{
		for (;;)
		{
			Status s = behaviour.tick();
			if (s != Status::FAILURE)
				return s;
			// If all nodes return success, sequence operation is a success
			if (++currentChild == getNode().children.end())
				return Status::FAILURE;
		}
		behaviour.setup(**currentChild);
	}
};

struct 

template <class TASK>
class MockComposite : public Composite
{
public:
	MockComposite(size_t size)
	{
		for (size_t i = 0; i < size; i++)
		{
			children.push_back(new MockNode);
		}
	}
};