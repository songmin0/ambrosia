#include "behaviour_tree.hpp"

struct MoveCloserTask : public Task
{
	int initializeCalled;
	int terminateCalled;
	int updateCalled;
	Status returnStatus;
	Status terminateStatus;

	MoveCloserTask(Node& node) :
		Task(node),
		initializeCalled(0),
		terminateCalled(0),
		updateCalled(0),
		returnStatus(Status::RUNNING),
		terminateStatus(Status::INVALID)
	{
	}

	virtual void onInitialize()
	{
		++initializeCalled;
	}
	virtual void onTerminate()
	{
		++terminateCalled;
	}
	virtual Status update()
	{
		++updateCalled;
		return returnStatus;
	}
};

struct MoveCloserNode : public Node
{
	virtual void destroy(Task*) {};
	virtual Task* create()
	{
		task = new MoveCloserTask(*this);
		return task;
	}
	virtual ~MoveCloserNode() { delete task; };
	MockNode() : task(NULL) {}

	MoveCloserTask* task;
};

template <class TASK>
class MoveComposite : public Composite
{
public:
	MoveComposite()
	{

	}
};