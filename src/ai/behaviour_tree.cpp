#include "behaviour_tree.hpp"
#include "stats_component.hpp"
#include "game/game_state_system.hpp"

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

void Conditional::setConditional(std::shared_ptr<Node> child, bool condition)
{
	this->child = child;
	this->condition = condition;
}

void Conditional::run()
{
	Node::run();
	if (condition)
	{
		// Will always terminate successfully whether child ran or not
		switch (child->getStatus())
		{
		case Status::INVALID:
		case Status::RUNNING:
			child->run();
			break;
		case Status::SUCCESS:
		case Status::FAILURE:
			onTerminate(Status::SUCCESS);
			break;
		}
	}
	else
	{
		this->onTerminate(Status::SUCCESS);
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
	case Status::RUNNING:
		current->run();
		break;
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
	case Status::RUNNING:
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
		}
		break;
	}
}

// MOB AI
// =====================================================================
// ROOT NODES

EggTurnSequence::EggTurnSequence()
{
	addChild(std::make_shared<EggMoveSelector>(EggMoveSelector()));
	addChild(std::make_shared<AttackTask>(AttackTask()));
}

void EggTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

PepperTurnSequence::PepperTurnSequence()
{
	//addChild(std::make_shared<MoveToPlayerTask>(MoveToPlayerTask()));
	addChild(std::make_shared<PepperMoveSelector>(PepperMoveSelector()));
	addChild(std::make_shared<AttackTask>(AttackTask()));
}

void PepperTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

MilkTurnSequence::MilkTurnSequence()
{
	addChild(std::make_shared<MilkMoveConditional>(MilkMoveConditional()));
	addChild(std::make_shared<MilkSkillSelector>(MilkSkillSelector()));
}

void MilkTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

// COMPOSITE BEHAVIOUR NODES

EggMoveSelector::EggMoveSelector()
{
	addChild(std::make_shared<MoveToPlayerTask>(MoveToPlayerTask()));
	addChild(std::make_shared<RunAwayTask>(RunAwayTask()));
}

void EggMoveSelector::run()
{
	Node::run();
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
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

PepperMoveSelector::PepperMoveSelector()
{
	addChild(std::make_shared<MoveToMobTask>(MoveToMobTask()));
	addChild(std::make_shared<MoveToPlayerTask>(MoveToPlayerTask()));
}

void PepperMoveSelector::run()
{
	Node::run(); 
	std::shared_ptr<Node> moveToMob = children.front();
	std::shared_ptr<Node> moveToPlayer = children.back();
	auto& entities = ECS::registry<TurnSystem::TurnComponent>.entities;
	bool shouldProtect = false;
	for (ECS::Entity entity : entities)
	{
		auto& allyStats = entity.get<StatsComponent>();
		// Check if there is an ally that needs protecting (low HP)
		if (entity.has<BehaviourTreeType>() && !entity.has<TurnSystem::TurnComponentIsActive>() &&
			allyStats.getStatValue(StatType::HP) < MOB_LOW_HEALTH)
		{
			shouldProtect = true;
			break;
		}
	}
	if (shouldProtect)
		moveToPlayer->onTerminate(Status::FAILURE);
	else
		moveToMob->onTerminate(Status::FAILURE);
	
	Selector::run();
}

MilkSkillSelector::MilkSkillSelector()
{
	addChild(std::make_shared<HealTask>(HealTask()));
	addChild(std::make_shared<AttackTask>(AttackTask()));
}

void MilkSkillSelector::run()
{
	Node::run();
	std::shared_ptr<Node> healMob = children.front();
	std::shared_ptr<Node> attackPlayer = children.back();
	bool shouldHeal = false;
	auto& entities = ECS::registry<TurnSystem::TurnComponent>.entities;
	for (ECS::Entity entity : entities)
	{
		auto& allyStats = entity.get<StatsComponent>();
		// Check if there is an ally that needs healing
		if (entity.has<BehaviourTreeType>() && !entity.has<TurnSystem::TurnComponentIsActive>() &&
			allyStats.getStatValue(StatType::HP) < allyStats.getStatValue(StatType::MAX_HP))
		{
			shouldHeal = true;
			break;
		}
	}
	if (shouldHeal)
		attackPlayer->onTerminate(Status::FAILURE);
	else
		healMob->onTerminate(Status::FAILURE);

	Selector::run();
}

// Currently the same as egg; leaving in case we want to have different behaviour later
MilkMoveSelector::MilkMoveSelector()
{
	addChild(std::make_shared<MoveToPlayerTask>(MoveToPlayerTask()));
	addChild(std::make_shared<RunAwayTask>(RunAwayTask()));
}

void MilkMoveSelector::run()
{
	Node::run();
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
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

// Run away if low HP; otherwise, don't move
MilkMoveConditional::MilkMoveConditional()
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
	setConditional(std::make_shared<RunAwayTask>(RunAwayTask()), hp < MOB_LOW_HEALTH);
}

void MilkMoveConditional::run()
{
	Node::run();
	Conditional::run();
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
	root = std::make_shared<MilkTurnSequence>(MilkTurnSequence());
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
		std::cout << "Moving to closest player\n";
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
		std::cout << "Moving to ally with lowest HP\n";
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
	this->onTerminate(Status::SUCCESS);
}

void RunAwayTask::run()
{
	if (this->status == Status::INVALID) {
		std::cout << "Running away from closest player\n";
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
		std::cout << "Attacking closest player\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&AttackTask::onFinishedAttackEvent, this, std::placeholders::_1));
		ECS::Entity activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		SetActiveSkillEvent activeEvent;
		activeEvent.entity = activeEntity;
		auto& mobType = activeEntity.get<BehaviourTreeType>().mobType;
		// Choose correct skill based on active mob entity
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
		skillEvent.targetIsPlayer = true;
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
		std::cout << "Healing ally with lowest HP\n";
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
		skillEvent.targetIsPlayer = false;
		EventSystem<StartMobSkillEvent>::instance().sendEvent(skillEvent);
	}
}