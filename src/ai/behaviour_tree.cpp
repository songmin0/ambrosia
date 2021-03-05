#include "behaviour_tree.hpp"
#include "stats_component.hpp"

void StateSystem::onStartMobTurnEvent(const StartMobTurnEvent& event)
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	assert(mob.has<BehaviourTreeType>());
	if (mob.get<BehaviourTreeType>().mobType == MobType::EGG)
	{
		activeTree = &EggBehaviourTree::EggBehaviourTree();
		//EggBehaviourTree::EggBehaviourTree().run();
	}
}

void StateSystem::step(float elapsed_ms)
{
	if (activeTree != NULL)
	{
		if (activeTree->root->status == Status::INVALID)
			activeTree->root->run();
		else if (activeTree->root->status == Status::RUNNING)
			activeTree->root->run();
		else
			activeTree = NULL;
	}
}

void Composite::addChild(Node* n)
{
	children.push_back(n);
}

void Selector::run()
{
	Node::run();
	// Check nodes, success if even one of them succeeds
	Node* current = children[i];
	switch (current->status)
	{
	case Status::INVALID:
		current->run();
	case Status::SUCCESS:
		i = 0;
		onTerminate(Status::SUCCESS);
		break;
	case Status::FAILURE:
		i++;
		if (i == children.size())
		{
			i = 0;
			onTerminate(Status::FAILURE);
		}
		break;
	case Status::RUNNING:
		break;
	}
	/*for (Node* n : getChildren())
	{
		n->run();
		if (n->status == Status::SUCCESS)
			onTerminate(Status::SUCCESS);
		return;
	}
	onTerminate(Status::FAILURE);*/
}

void Sequence::run()
{
	Node::run();
	// Check all nodes, success if all of them succeed
	Node* current = children[i];
	switch (current->status)
	{
	case Status::INVALID: 
		current->run();
	case Status::FAILURE: 
		i = 0;
		onTerminate(Status::FAILURE);
		break;
	case Status::SUCCESS: 
		i++;
		if (i == children.size())
		{
			i = 0;
			onTerminate(Status::SUCCESS);
		}
		break;
	case Status::RUNNING:
		break;
	}
	//if (current.status == Status::INVALID)
	//	current.run();
	//else if (current.status == Status::FAILURE)
	//	onTerminate(Status::FAILURE);
	//if (current.status == Status::SUCCESS)
	//{
	//	i++;
	//	// If all succeeded, succeed
	//	if (i == getChildren().size())
	//		onTerminate(Status::SUCCESS);
	//}
}

// MOB AI
// =====================================================================

MobTurnSequence::MobTurnSequence()
{
	//ECS::Entity mob = StateSystem::getCurrentMobEntity();
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
	MoveSelector moveSelector(hp);
	addChild(&moveSelector);
	AttackTask attack;
	addChild(&attack);
}

// MOVING

MoveSelector::MoveSelector(float hp)
	: hp(hp)
{
	MoveCloserTask moveCloser;
	addChild(&moveCloser);
	RunAwayTask runAway;
	addChild(&runAway);
}

void MoveSelector::run()
{
	Node::run();
	if (hp < 25.f)
	{
		// Last node should be RunAwayTask
		Node* runAway = children.back();
		runAway->run();
		return;
	}
	// Otherwise, MoveCloserTask
	Node* moveCloser = children.front();
	moveCloser->run();
}

void MoveCloserTask::onFinishedMoveCloserEvent(const FinishedMovementEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void MoveCloserTask::run()
{
	taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
		std::bind(&MoveCloserTask::onFinishedMoveCloserEvent, this, std::placeholders::_1));
	Node::run();
	StartMobMoveCloserEvent event;
	//event.entity = StateSystem::getCurrentMobEntity();
	event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	EventSystem<StartMobMoveCloserEvent>::instance().sendEvent(event);
}

void RunAwayTask::onFinishedRunAwayEvent(const FinishedMovementEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void RunAwayTask::run()
{
	taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
		std::bind(&RunAwayTask::onFinishedRunAwayEvent, this, std::placeholders::_1));
	Node::run();

	StartMobRunAwayEvent event;
	//event.entity = StateSystem::getCurrentMobEntity();
	event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	EventSystem<StartMobRunAwayEvent>::instance().sendEvent(event);
}

// ATTACKING

void AttackTask::onFinishedAttackEvent(const FinishedSkillEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void AttackTask::run()
{
	taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
		std::bind(&AttackTask::onFinishedAttackEvent, this, std::placeholders::_1));
	Node::run();
	StartMobSkillEvent event;
	//event.entity = StateSystem::getCurrentMobEntity();
	event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	EventSystem<StartMobSkillEvent>::instance().sendEvent(event);
}

EggBehaviourTree::EggBehaviourTree()
{
	//BehaviourTree::BehaviourTree(new &MobTurnSequence);
	root = &MobTurnSequence::MobTurnSequence();
}