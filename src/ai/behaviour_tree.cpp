#include "behaviour_tree.hpp"
#include "stats_component.hpp"
#include "game/game_state_system.hpp"

const float MOB_LOW_HEALTH = 25.f;

void StateSystem::onStartMobTurnEvent(const StartMobTurnEvent& event)
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	assert(mob.has<BehaviourTreeType>());
	if (mob.get<BehaviourTreeType>().mobType == MobType::EGG)
	{
		activeTree = std::make_shared<BehaviourTree>(EggBehaviourTree());
	}
}

void StateSystem::step(float elapsed_ms)
{
		if (GameStateSystem::instance().inGameState()) {
				if (activeTree != nullptr && activeTree->root != nullptr)
				{
						if (activeTree->root->getStatus() == Status::INVALID)
								activeTree->root->run();
						else if (activeTree->root->getStatus() == Status::RUNNING)
								activeTree->root->run();
						else
								activeTree = nullptr;
				}
		}
}

void Composite::addChild(std::shared_ptr<Node> n)
{
	children.push_back(n);
}

void Selector::run()
{
	Node::run();
	// Check nodes, success if even one of them succeeds
	std::shared_ptr<Node> current = children[i];
	switch (current->getStatus())
	{
	case Status::INVALID:
		current->run();
		break;
	case Status::SUCCESS:
		i = 0;
		onTerminate(Status::SUCCESS);
		std::cout << "Finished selector task\n";
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
}

void Sequence::run()
{
	Node::run();
	// Check all nodes, success if all of them succeed
	std::shared_ptr<Node> current = children[i];
	switch (current->getStatus())
	{
	case Status::INVALID:
		current->run();
		break;
	case Status::FAILURE:
		i = 0;
		onTerminate(Status::FAILURE);
		break;
	case Status::SUCCESS:
		// Go to the next node
		i++;
		// If at end, terminate with SUCCESS
		if (i == children.size())
		{
			i = 0;
			onTerminate(Status::SUCCESS);
			std::cout << "Finished sequence task\n";
		}
		break;
	case Status::RUNNING:
		current->run();
		break;
	}
}

// MOB AI
// =====================================================================

MobTurnSequence::MobTurnSequence()
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
	addChild(std::make_shared<MoveSelector>(MoveSelector(hp)));
	addChild(std::make_shared<AttackTask>(AttackTask()));
}

void MobTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

// MOVING

MoveSelector::MoveSelector(float hp)
	: hp(hp)
{
	addChild(std::make_shared<MoveCloserTask>(MoveCloserTask()));
	addChild(std::make_shared<RunAwayTask>(RunAwayTask()));
}

void MoveSelector::run()
{
	Node::run();
	std::shared_ptr<Node> moveCloser = children.front();
	std::shared_ptr<Node> runAway = children.back();
	if (hp < MOB_LOW_HEALTH)
	{
		// Want to run away; set move closer to FAILURE
		moveCloser->onTerminate(Status::FAILURE);
	}
	else
	{
		// Want to move closer; set running away to FAILURE
		runAway->onTerminate(Status::FAILURE);
	}
	Selector::run();
}

MoveCloserTask::~MoveCloserTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

void MoveCloserTask::onFinishedMoveCloserEvent(const FinishedMovementEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void MoveCloserTask::run()
{
	if (this->status == Status::INVALID)
	{
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveCloserTask::onFinishedMoveCloserEvent, this, std::placeholders::_1));
		StartMobMoveCloserEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		EventSystem<StartMobMoveCloserEvent>::instance().sendEvent(event);
	}
	
}

RunAwayTask::~RunAwayTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

void RunAwayTask::onFinishedRunAwayEvent(const FinishedMovementEvent& event)
{
	std::cout << "Heard run away event finished\n";
	this->onTerminate(Status::SUCCESS);
}

void RunAwayTask::run()
{
	if (this->status == Status::INVALID) {
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&RunAwayTask::onFinishedRunAwayEvent, this, std::placeholders::_1));

		StartMobRunAwayEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		EventSystem<StartMobRunAwayEvent>::instance().sendEvent(event);
	}
}

// ATTACKING

AttackTask::~AttackTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedSkillEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

void AttackTask::onFinishedAttackEvent(const FinishedSkillEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void AttackTask::run()
{
	if (this->status == Status::INVALID)
	{
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&AttackTask::onFinishedAttackEvent, this, std::placeholders::_1));
		StartMobSkillEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		EventSystem<StartMobSkillEvent>::instance().sendEvent(event);
	}
}

EggBehaviourTree::EggBehaviourTree()
{
	root = std::make_shared<MobTurnSequence>(MobTurnSequence());
}