#include "behaviour_tree.hpp"
#include "game/stats_component.hpp"
#include "tree_components.hpp"
#include "game/game_state_system.hpp"
#include "swarm_behaviour.hpp"
#include "ai/ai.hpp"

const float MOB_LOW_HEALTH = 25.f;

void StateSystem::onStartMobTurnEvent()
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	assert(mob.has<BehaviourTreeType>());
	auto mobBTreeType = mob.get<BehaviourTreeType>().mobType;

	switch (mobBTreeType)
	{
	case MobType::EGG:
		activeTree = std::make_shared<BehaviourTree>(EggBehaviourTree());
		break;
	case MobType::PEPPER:
		activeTree = std::make_shared<BehaviourTree>(PepperBehaviourTree());
		break;
	case MobType::MILK:
		activeTree = std::make_shared<BehaviourTree>(MilkBehaviourTree());
		break;
	case MobType::POTATO:
		activeTree = std::make_shared<BehaviourTree>(PotatoBehaviourTree());
		break;
	case MobType::POTATO_CHUNK:
		activeTree = std::make_shared<BehaviourTree>(PotatoChunkBehaviourTree());
		break;
	case MobType::MASHED_POTATO:
		activeTree = std::make_shared<BehaviourTree>(MashedPotatoBehaviourTree());
		break;
	case MobType::TOMATO:
		activeTree = std::make_shared<BehaviourTree>(BasicMeleeBehaviourTree());
		break;
	case MobType::LETTUCE:
		activeTree = std::make_shared<BehaviourTree>(LettuceBehaviourTree());
		break;
	case MobType::SALTNPEPPER:
		activeTree = std::make_shared<BehaviourTree>(SaltnPepperBehaviourTree());
		break;
	case MobType::CHICKEN:
		activeTree = std::make_shared<BehaviourTree>(ChickenBehaviourTree());
		break;
	default:
		break;
	}
}

void StateSystem::onEndMobTurnEvent()
{
	activeTree = nullptr;
}

void StateSystem::step(float elapsed_ms)
{
	if (!GameStateSystem::instance().inGameState()) {
		return;
	}
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

BasicMeleeSequence::BasicMeleeSequence()
{
	addChild(std::make_shared<MoveToClosestPlayerTask>(MoveToClosestPlayerTask()));
	addChild(std::make_shared<MeleeSkillSelector>(MeleeSkillSelector()));
}

void BasicMeleeSequence::run()
{
	Node::run();
	Sequence::run();
}

LettuceTurnSequence::LettuceTurnSequence()
{
	addChild(std::make_shared<MeleeSkillSelector>(MeleeSkillSelector()));
}

void LettuceTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

SaltnPepperTurnSequence::SaltnPepperTurnSequence()
{
	addChild(std::make_shared<EggMoveSelector>(EggMoveSelector()));
	addChild(std::make_shared<RngAttackTask>(RngAttackTask()));
}

void SaltnPepperTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

ChickenTurnSequence::ChickenTurnSequence()
{
	addChild(std::make_shared<RngAttackTask>(RngAttackTask()));
}

void ChickenTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

EggTurnSequence::EggTurnSequence()
{
	addChild(std::make_shared<EggMoveSelector>(EggMoveSelector()));
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
}

void EggTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

PepperTurnSequence::PepperTurnSequence()
{
	addChild(std::make_shared<PepperMoveSelector>(PepperMoveSelector()));
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
}

void PepperTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

RandomMeleeSequence::RandomMeleeSequence()
{
	addChild(std::make_shared<MoveToRandomPlayerTask>(MoveToRandomPlayerTask()));
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
}

void RandomMeleeSequence::run()
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

PotatoChunkTurnSequence::PotatoChunkTurnSequence()
{
	addChild(std::make_shared<PotatoChunkMoveConditional>(PotatoChunkMoveConditional()));
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
}

void PotatoChunkTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

PotatoSkillSelector::PotatoSkillSelector()
{
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
	addChild(std::make_shared<UltimateAttackTask>(UltimateAttackTask()));
}

void PotatoSkillSelector::run()
{
	Node::run();
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	auto& mobStats = mob.get<StatsComponent>();
	float hp = mobStats.getStatValue(StatType::HP);
	float maxHP = mobStats.getStatValue(StatType::MAX_HP);
	float hpPercent = hp / maxHP;
	float numUltLeft = mobStats.getStatValue(StatType::NUM_ULT_LEFT);
	float maxNumUlt = mobStats.getStatValue(StatType::MAX_NUM_ULT);
	std::shared_ptr<Node> basicAttack = children.front();
	std::shared_ptr<Node> ultimateAttack = children.back();
	
	// Currently uses ult on first turn and then the first time it goes below 50% HP
	if  (numUltLeft == maxNumUlt ||
		(numUltLeft == (maxNumUlt - 1) && hpPercent < 0.5))
	{
		basicAttack->onTerminate(Status::FAILURE);
		switch (ultimateAttack->getStatus())
		{
		case Status::SUCCESS:
			mobStats.setBaseValue(StatType::NUM_ULT_LEFT, numUltLeft - 1);
			break;
		default:
			break;
		}
	}
	else
	{
		ultimateAttack->onTerminate(Status::FAILURE);
	}
	Selector::run();
}

MashedPotatoTurnSequence::MashedPotatoTurnSequence()
{
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
}

void MashedPotatoTurnSequence::run()
{
	Node::run();
	Sequence::run();
}

// COMPOSITE BEHAVIOUR NODES

EggMoveSelector::EggMoveSelector()
{
	addChild(std::make_shared<MoveToClosestPlayerTask>(MoveToClosestPlayerTask()));
	addChild(std::make_shared<RunAwayTask>(RunAwayTask()));
}

void EggMoveSelector::run()
{
	Node::run();
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	float hp = mob.get<StatsComponent>().getStatValue(StatType::HP);
	std::shared_ptr<Node> moveToClosestPlayer = children.front();
	std::shared_ptr<Node> runAway = children.back();
	if (hp < MOB_LOW_HEALTH)
	{
		// Want to run away; set move closer to FAILURE
		moveToClosestPlayer->onTerminate(Status::FAILURE);
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
	addChild(std::make_shared<MoveToWeakestPlayerTask>(MoveToWeakestPlayerTask()));
	addChild(std::make_shared<MoveToFarthestPlayerTask>(MoveToFarthestPlayerTask()));
}

void PepperMoveSelector::run()
{
	Node::run(); 
	std::shared_ptr<Node> moveToWeakestPlayer = children.front();
	std::shared_ptr<Node> moveToFarthestPlayer = children.back();
	auto& players = ECS::registry<PlayerComponent>.entities;
	bool weakPlayerExists = false;
	for (ECS::Entity player : players)
	{
		auto& playerStats = player.get<StatsComponent>();
		// Check if there is a living player that is injured
		if (!player.has<DeathTimer>() && playerStats.getStatValue(StatType::HP) < playerStats.getStatValue(StatType::MAX_HP))
		{
			weakPlayerExists = true;
			break;
		}
	}
	if (weakPlayerExists)
		moveToFarthestPlayer->onTerminate(Status::FAILURE);
	else
		moveToWeakestPlayer->onTerminate(Status::FAILURE);
	
	Selector::run();
}

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

PotatoChunkMoveConditional::PotatoChunkMoveConditional()
{
	ECS::Entity chunk = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	auto chunk_pos = ECS::registry<Motion>.get(chunk).position;
	auto potato = ECS::registry<ActivePotatoChunks>.get(chunk).potato;
	auto potato_pos = ECS::registry<Motion>.get(potato).position;

	// get grid positions
	auto chunk_grid_pos = vec2(round(chunk_pos.x / 32), round(chunk_pos.y / 32));
	auto potato_grid_pos = vec2(round(potato_pos.x / 32), round(potato_pos.y / 32));

	// only move if destination and chunk are on different grid positions
	setConditional(std::make_shared<MoveToDeadPotato>(MoveToDeadPotato()), chunk_grid_pos != potato_grid_pos);
}

void PotatoChunkMoveConditional::run()
{
	Node::run();
	Conditional::run();
}

MilkSkillSelector::MilkSkillSelector()
{
	addChild(std::make_shared<HealTask>(HealTask()));
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
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
		if (entity.has<BehaviourTreeType>() && !entity.has<TurnSystem::TurnComponentIsActive>() && !entity.has<DeathTimer>() &&
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

MeleeSkillSelector::MeleeSkillSelector()
{
	addChild(std::make_shared<BasicAttackTask>(BasicAttackTask()));
	addChild(std::make_shared<RangedAttackTask>(RangedAttackTask()));
}

void MeleeSkillSelector::run()
{
	ECS::Entity mob = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	assert(mob.has<AISystem::MobComponent>() && mob.has<Motion>());
	auto& mobM = mob.get<Motion>();

	float skillRange = 0.f;
	assert(mob.has<SkillComponent>());
	auto& skillComponent = mob.get<SkillComponent>();
	skillComponent.setActiveSkill(SkillType::SKILL1);
	std::shared_ptr<Skill> activeSkill = skillComponent.getActiveSkill();
	if (activeSkill)
	{
		skillRange = activeSkill->getRange();
	}

	auto circularProvider = std::make_shared<CircularProvider>(skillRange);
	auto entitiesInRange = circularProvider->getEntities(mob, vec2(0.f));

	bool isPlayerInRange = false;
	for (auto entity : entitiesInRange)
	{
		// Check if there is an alive player in range
		if (entity.has<PlayerComponent>() 
			&& !entity.has<DeathTimer>() && entity.has<Motion>())
		{
			isPlayerInRange = true;
			break;
		}
	}

	std::shared_ptr<Node> basicAttack = children.front();
	std::shared_ptr<Node> rangedAttack = children.back();

	// If within range, use the basic attack
	if (isPlayerInRange)
	{
		rangedAttack->onTerminate(Status::FAILURE);
	}
	else
	{
		basicAttack->onTerminate(Status::FAILURE);
	}
	Selector::run();
}

// MOB BEHAVIOUR TREE
// =====================================================================

BasicMeleeBehaviourTree::BasicMeleeBehaviourTree()
{
	root = std::make_shared<BasicMeleeSequence>(BasicMeleeSequence());
}

LettuceBehaviourTree::LettuceBehaviourTree()
{
	root = std::make_shared<LettuceTurnSequence>(LettuceTurnSequence());
}

SaltnPepperBehaviourTree::SaltnPepperBehaviourTree()
{
	root = std::make_shared<SaltnPepperTurnSequence>(SaltnPepperTurnSequence());
}

ChickenBehaviourTree::ChickenBehaviourTree()
{
	root = std::make_shared<ChickenTurnSequence>(ChickenTurnSequence());
}


RandomMeleeBehaviourTree::RandomMeleeBehaviourTree()
{
	root = std::make_shared<RandomMeleeSequence>(RandomMeleeSequence());
}

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

PotatoBehaviourTree::PotatoBehaviourTree()
{
	root = std::make_shared<PotatoSkillSelector>(PotatoSkillSelector());
}

PotatoChunkBehaviourTree::PotatoChunkBehaviourTree()
{
	root = std::make_shared<PotatoChunkTurnSequence>(PotatoChunkTurnSequence());
}

MashedPotatoBehaviourTree::MashedPotatoBehaviourTree()
{
	root = std::make_shared<MashedPotatoTurnSequence>(MashedPotatoTurnSequence());
}

// TASKS
// Public tasks not meant for any single mob
// =====================================================================

void Task::onFinishedTaskEvent()
{
	this->onTerminate(Status::SUCCESS);
}

MoveTask::~MoveTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedMovementEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

SkillTask::~SkillTask()
{
	if (taskCompletedListener.isValid())
	{
		EventSystem<FinishedSkillEvent>::instance().unregisterListener(taskCompletedListener);
	}
}

void MoveToClosestPlayerTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Moving to closest player\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToClosestPlayerTask::onFinishedTaskEvent, this));
		StartMobMoveEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		event.movement.moveType = MoveType::TO_CLOSEST_PLAYER;
		EventSystem<StartMobMoveEvent>::instance().sendEvent(event);
	}
}

void MoveToFarthestPlayerTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Moving to farthest player\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToFarthestPlayerTask::onFinishedTaskEvent, this));
		StartMobMoveEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		event.movement.moveType = MoveType::TO_FARTHEST_PLAYER;
		EventSystem<StartMobMoveEvent>::instance().sendEvent(event);
	}

}

void MoveToWeakestPlayerTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Moving to weakest player\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToWeakestPlayerTask::onFinishedTaskEvent, this));
		StartMobMoveEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		event.movement.moveType = MoveType::TO_WEAKEST_PLAYER;
		EventSystem<StartMobMoveEvent>::instance().sendEvent(event);
	}

}

void MoveToRandomPlayerTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Moving to random player\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToRandomPlayerTask::onFinishedTaskEvent, this));
		StartMobMoveEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		event.movement.moveType = MoveType::TO_RANDOM_PLAYER;
		EventSystem<StartMobMoveEvent>::instance().sendEvent(event);
	}
}

void MoveToDeadPotato::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Moving to dead potato\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToDeadPotato::onFinishedTaskEvent, this));
		StartMobMoveEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		event.movement.moveType = MoveType::TO_DEAD_POTATO;
		EventSystem<StartMobMoveEvent>::instance().sendEvent(event);
	}

}

void MoveToWeakestMobTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Moving to ally with lowest HP\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&MoveToWeakestMobTask::onFinishedTaskEvent, this));
		StartMobMoveEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		event.movement.moveType = MoveType::TO_WEAKEST_MOB;
		EventSystem<StartMobMoveEvent>::instance().sendEvent(event);
	}

}

void RunAwayTask::run()
{
	if (this->status == Status::INVALID) {
		std::cout << "Running away from closest player\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedMovementEvent>::instance().registerListener(
			std::bind(&RunAwayTask::onFinishedTaskEvent, this));
		StartMobMoveEvent event;
		event.entity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		event.movement.moveType = MoveType::AWAY_CLOSEST_PLAYER;
		EventSystem<StartMobMoveEvent>::instance().sendEvent(event);
	}
}

// ATTACKING

void BasicAttackTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Attacking closest player\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&BasicAttackTask::onFinishedTaskEvent, this));

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
		case MobType::POTATO_CHUNK:
		case MobType::MASHED_POTATO:
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

void UltimateAttackTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Attacking with ultimate\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&UltimateAttackTask::onFinishedTaskEvent, this));
		ECS::Entity activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		SetActiveSkillEvent activeEvent;
		activeEvent.entity = activeEntity;
		
		auto& mobType = activeEntity.get<BehaviourTreeType>().mobType;
		// Choose correct skill based on active mob entity
		switch (mobType)
		{
		case MobType::POTATO:
			activeEvent.type = SkillType::SKILL2;
			break;
		default:
			std::cout << "Mob without ultimate attack was called in UltimateAttackTask\n";
			activeEvent.type = SkillType::SKILL1;
			break;
		}
		EventSystem<SetActiveSkillEvent>::instance().sendEvent(activeEvent);

		StartMobSkillEvent skillEvent;
		skillEvent.entity = activeEntity;
		// Not necessary but we can keep it here for directional AOE later
		skillEvent.targetIsPlayer = true;
		EventSystem<StartMobSkillEvent>::instance().sendEvent(skillEvent);
	}
}

void RangedAttackTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Attacking with alternate ranged attack \n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&RangedAttackTask::onFinishedTaskEvent, this));
		ECS::Entity activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		SetActiveSkillEvent activeEvent;
		activeEvent.entity = activeEntity;
		auto& mobType = activeEntity.get<BehaviourTreeType>().mobType;
		// Choose correct skill based on active mob entity
		switch (mobType)
		{
		case MobType::TOMATO:
		case MobType::LETTUCE:
			activeEvent.type = SkillType::SKILL2;
			break;
		default:
			std::cout << "Mob without alternate ranged attack was called in RangedAttackTask\n";
			activeEvent.type = SkillType::SKILL2;
			break;
		}
		EventSystem<SetActiveSkillEvent>::instance().sendEvent(activeEvent);

		StartMobSkillEvent skillEvent;
		skillEvent.entity = activeEntity;
		skillEvent.targetIsPlayer = true;
		EventSystem<StartMobSkillEvent>::instance().sendEvent(skillEvent);
	}
}

void RngAttackTask::run()
{
	if (this->status == Status::INVALID)
	{
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&RngAttackTask::onFinishedTaskEvent, this));
		ECS::Entity activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
		SetActiveSkillEvent activeEvent;
		activeEvent.entity = activeEntity;
		auto& mobType = activeEntity.get<BehaviourTreeType>().mobType;

		if (mobType == MobType::SALTNPEPPER)
		{
			int skill = rand() % 2; // 0 or 1
			if (skill == 0)
			{
				std::cout << "Attacking with randomly chosen skill 1\n";
				activeEvent.type = SkillType::SKILL1;
			}
			else
			{
				std::cout << "Attacking with randomly chosen skill 2\n";
				activeEvent.type = SkillType::SKILL2;
			}
		}
		else if (mobType == MobType::CHICKEN)
		{
			int skill = rand() % 3; // 0 to 2
			if (skill == 0) // 33% chance of activating Strength Buff
			{
				std::cout << "Using big strength buff\n";
				activeEvent.type = SkillType::SKILL2;
			}
			else
			{
				std::cout << "Attacking random player with drumstick\n";
				activeEvent.type = SkillType::SKILL1;
			}
		}
		else
		{
			std::cout << "Mob not designed for rng attack was called in RngAttackTask\n";
			activeEvent.type = SkillType::SKILL1;
		}

		EventSystem<SetActiveSkillEvent>::instance().sendEvent(activeEvent);

		StartMobSkillEvent skillEvent;
		skillEvent.entity = activeEntity;
		skillEvent.targetIsPlayer = true;
		skillEvent.isRandomTarget = true;
		EventSystem<StartMobSkillEvent>::instance().sendEvent(skillEvent);
	}
}

// HEALING

void HealTask::run()
{
	if (this->status == Status::INVALID)
	{
		std::cout << "Healing ally with lowest HP\n";
		Node::run();
		taskCompletedListener = EventSystem<FinishedSkillEvent>::instance().registerListener(
			std::bind(&HealTask::onFinishedTaskEvent, this));
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