#include "behaviour_tree.hpp"
#include "stats_component.hpp"

const float MOB_LOW_HEALTH = 25.f;

void StateSystem::onStartMobTurnEvent(const StartMobTurnEvent& event)
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	assert(mob.has<BehaviourTreeType>());
	auto mobBTreeType = mob.get<BehaviourTreeType>().mobType;

	switch (mobBTreeType)
	{
	case MobType::EGG :
	case MobType::POTATO :
		activeTree = std::make_shared<BehaviourTree>(EggBehaviourTree());
		break;
	case MobType::PEPPER :
		activeTree = std::make_shared<BehaviourTree>(PepperBehaviourTree());
		break;
	case MobType::MILK :
		activeTree = std::make_shared<BehaviourTree>(MilkBehaviourTree());
		break;
	default :
		break;
	}
}

void StateSystem::step(float elapsed_ms)
{
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
		current->run();
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
// ROOT NODES

EggTurnSequence::EggTurnSequence()
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
	addChild(std::make_shared<EggMoveSelector>(EggMoveSelector(hp)));
	addChild(std::make_shared<AttackTask>(AttackTask()));
}

void EggTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

PepperTurnSequence::PepperTurnSequence()
{
	addChild(std::make_shared<MoveToPlayerTask>(MoveToPlayerTask()));
	addChild(std::make_shared<AttackTask>(AttackTask()));
}

void PepperTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

MilkTurnSelector::MilkTurnSelector(bool alliesLeft)
	: alliesLeft(alliesLeft)
{
	addChild(std::make_shared<MilkHealSequence>(MilkHealSequence()));
	addChild(std::make_shared<MilkAttackSequence>(MilkAttackSequence()));
}

void MilkTurnSelector::run()
{
	Node::run();
	std::shared_ptr<Node> healSequence = children.front();
	std::shared_ptr<Node> attackSequence = children.back();
	if (alliesLeft)
	{
		// Want to heal; set attack sequence to FAILURE
		attackSequence->onTerminate(Status::FAILURE);
	}
	else
	{
		// Want to attack; set heal sequence to FAILURE
		healSequence->onTerminate(Status::FAILURE);
	}
	Selector::run();
}

// COMPOSITE BEHAVIOUR NODES

EggMoveSelector::EggMoveSelector(float hp)
	: hp(hp)
{
	addChild(std::make_shared<MoveToPlayerTask>(MoveToPlayerTask()));
	addChild(std::make_shared<RunAwayTask>(RunAwayTask()));
}

void EggMoveSelector::run()
{
	Node::run();
	std::shared_ptr<Node> moveToPlayer = children.front();
	std::shared_ptr<Node> runAway = children.back();
	if (hp < MOB_LOW_HEALTH)
	{
		// Want to run away; set move closer to FAILURE
		moveToPlayer->onTerminate(Status::FAILURE);
	}
	else
	{
		// Want to move closer; set running away to FAILURE
		runAway->onTerminate(Status::FAILURE);
	}
	Selector::run();
}

MilkHealSequence::MilkHealSequence()
{
	addChild(std::make_shared<MoveToMobTask>(MoveToMobTask()));
	addChild(std::make_shared<HealTask>(HealTask()));
}

void MilkHealSequence::run()
{
	Node::run();
	Sequence::run();
}

MilkAttackSequence::MilkAttackSequence()
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
	addChild(std::make_shared<MilkMoveSelector>(MilkMoveSelector(hp)));
	addChild(std::make_shared<AttackTask>(AttackTask()));
}

void MilkAttackSequence::run()
{
	Node::run();
	Sequence::run();
}

// Currently the same as egg; leaving in case we want to have different behaviour later
MilkMoveSelector::MilkMoveSelector(float hp)
	: hp(hp)
{
	addChild(std::make_shared<MoveToPlayerTask>(MoveToPlayerTask()));
	addChild(std::make_shared<RunAwayTask>(RunAwayTask()));
}

void MilkMoveSelector::run()
{
	Node::run();
	std::shared_ptr<Node> moveToPlayer = children.front();
	std::shared_ptr<Node> runAway = children.back();
	if (hp < MOB_LOW_HEALTH)
	{
		// Want to run away; set move closer to FAILURE
		moveToPlayer->onTerminate(Status::FAILURE);
	}
	else
	{
		// Want to move closer; set running away to FAILURE
		runAway->onTerminate(Status::FAILURE);
	}
	Selector::run();
}

// MOB BEHAVIOUR TREE
// =====================================================================

EggBehaviourTree::EggBehaviourTree()
{
	root = std::make_shared<EggTurnSequence>(EggTurnSequence());
}

PepperBehaviourTree::PepperBehaviourTree()
{
	root = std::make_shared<PepperTurnSequence>(PepperTurnSequence());
}

MilkBehaviourTree::MilkBehaviourTree()
{
	bool alliesLeft = false;
	auto& entities = ECS::registry<TurnSystem::TurnComponent>.entities;
	int numMobs = 0;
	for (ECS::Entity entity : entities)
	{
		// Check if entity is a mob
		if (entity.has<BehaviourTreeType>())
			numMobs++;
	}
	// We should always at least have the active milk mob
	assert(numMobs >= 1);
	if (numMobs > 1)
		alliesLeft = true;
	root = std::make_shared<MilkTurnSelector>(MilkTurnSelector(alliesLeft));
}

// TASKS
// Public tasks not meant for any single mob
// =====================================================================

MoveToPlayerTask::~MoveToPlayerTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

void MoveToPlayerTask::onFinishedMoveToPlayerEvent(const FinishedMovementEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void MoveToPlayerTask::run()
{
	if (this->status == Status::INVALID)
	{
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToPlayerTask::onFinishedMoveToPlayerEvent, this, std::placeholders::_1));
		StartMobMoveToPlayerEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		EventSystem<StartMobMoveToPlayerEvent>::instance().sendEvent(event);
	}

}

MoveToMobTask::~MoveToMobTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

void MoveToMobTask::onFinishedMoveToMobEvent(const FinishedMovementEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void MoveToMobTask::run()
{
	if (this->status == Status::INVALID)
	{
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToMobTask::onFinishedMoveToMobEvent, this, std::placeholders::_1));
		StartMobMoveToMobEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		EventSystem<StartMobMoveToMobEvent>::instance().sendEvent(event);
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
		ECS::Entity activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		SetActiveSkillEvent activeEvent;
		activeEvent.entity = activeEntity;
		auto& mobType = activeEntity.get<BehaviourTreeType>().mobType;
		switch (mobType)
		{
		case MobType::EGG:
		case MobType::PEPPER:
		case MobType::POTATO:
			activeEvent.type = SkillType::SKILL1;
			break;
		case MobType::MILK:
			activeEvent.type = SkillType::SKILL2;
			break;
		default:
			activeEvent.type = SkillType::SKILL1;
			break;
		}
		EventSystem<SetActiveSkillEvent>::instance().sendEvent(activeEvent);

		StartMobSkillEvent skillEvent;
		skillEvent.entity = activeEntity;
		EventSystem<StartMobSkillEvent>::instance().sendEvent(skillEvent);
	}
}

// HEALING

HealTask::~HealTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedSkillEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

void HealTask::onFinishedHealEvent(const FinishedSkillEvent& event)
{
	this->onTerminate(Status::SUCCESS);
}

void HealTask::run()
{
	if (this->status == Status::INVALID)
	{
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&HealTask::onFinishedHealEvent, this, std::placeholders::_1));
		ECS::Entity activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		SetActiveSkillEvent activeEvent;
		activeEvent.entity = activeEntity;
		auto& mobType = activeEntity.get<BehaviourTreeType>().mobType;
		switch (mobType)
		{
		case MobType::MILK:
			activeEvent.type = SkillType::SKILL1;
			break;
		default:
			activeEvent.type = SkillType::SKILL1;
			break;
		}
		EventSystem<SetActiveSkillEvent>::instance().sendEvent(activeEvent);

		StartMobSkillEvent skillEvent;
		skillEvent.entity = activeEntity;
		EventSystem<StartMobSkillEvent>::instance().sendEvent(skillEvent);
	}
}