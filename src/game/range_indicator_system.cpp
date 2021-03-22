#include "range_indicator_system.hpp"
#include <functional>

//TODO find a way to use the version from "entity_provider.cpp"
vec2 getCenterOfEntity(ECS::Entity entity)
{
	assert(entity.has<Motion>());
	auto& motion = entity.get<Motion>();

	vec2 center = motion.position;
	center.y -= abs(motion.boundingBox.y / 2.f);

	return center;
}


RangeIndicatorSystem::RangeIndicatorSystem()
{
	skillChangeListener = EventSystem<ChangeRangeIndicatorEvent>::instance().registerListener(
		std::bind(&RangeIndicatorSystem::onRangeIndicatorChange, this, std::placeholders::_1));

	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
		std::bind(&RangeIndicatorSystem::onPlayerChange, this, std::placeholders::_1));

	performActiveSkillListener = EventSystem<PerformActiveSkillEvent>::instance().registerListener(
		std::bind(&RangeIndicatorSystem::performActiveSkillEvent, this, std::placeholders::_1));

	performActiveSkillListener = EventSystem<MouseHoverEvent>::instance().registerListener(
		std::bind(&RangeIndicatorSystem::mouseHoverEvent, this, std::placeholders::_1));
}

RangeIndicatorSystem::~RangeIndicatorSystem()
{
	if (skillChangeListener.isValid()) {
		EventSystem<ChangeRangeIndicatorEvent>::instance().unregisterListener(skillChangeListener);
	}

	if (playerChangeListener.isValid()) {
		EventSystem<PlayerChangeEvent>::instance().unregisterListener(playerChangeListener);
	}

	if (performActiveSkillListener.isValid()) {
		EventSystem<PerformActiveSkillEvent>::instance().unregisterListener(performActiveSkillListener);
	}

	if (performActiveSkillListener.isValid()) {
		EventSystem<MouseHoverEvent>::instance().unregisterListener(performActiveSkillListener);
	}
}

void RangeIndicatorSystem::onPlayerChange(const PlayerChangeEvent event)
{
	ChangeRangeIndicatorEvent ChangeRangeIndicatorEvent;
	ChangeRangeIndicatorEvent.entity = event.newActiveEntity;
	ChangeRangeIndicatorEvent.type = SkillType::NONE;
	onRangeIndicatorChange(ChangeRangeIndicatorEvent);
}

void RangeIndicatorSystem::onRangeIndicatorChange(const ChangeRangeIndicatorEvent& event)
{
	//Remove the old range indicator
	ECS::ContainerInterface::removeAllComponentsOf(rangeIndicator);

	if (event.type == SkillType::MOVE || event.type == SkillType::NONE) {
		return;
	}
	
	auto entity = event.entity;
	auto playerMotion = entity.get<Motion>();
	if (entity.has<SkillComponent>()&& entity.has<PlayerComponent>())
	{
		activeSkillUsesMouseLoc = false;
		
		//Create a range indicator
		rangeIndicator = ECS::Entity();
		ShadedMesh& resource = cacheResource("range_indicator");
		if (resource.effect.program.resource == 0)
		{
			RenderSystem::createSprite(resource, texturesPath("circle-blue-overlay.png"), "textured");
		}

		rangeIndicator.emplace<ShadedMeshRef>(resource);
		rangeIndicator.emplace<RenderableComponent>(RenderLayer::RANGE_INDICATOR);

		auto& skillComponent = entity.get<SkillComponent>();
		std::shared_ptr<Skill> skill = skillComponent.getActiveSkill();
		//set the center position for the range indicator
		//TODO check if the skill is using a MouseClickProvider
	
		auto& motion = ECS::registry<Motion>.emplace(rangeIndicator);

		//Check if the skillParam is an AoESkillParam
		if (auto aoeSkillsParam = std::dynamic_pointer_cast<AoESkillParams>(skill->getParams())) {
			//Check if the entityProvider is a MouseClickProvider
			if (std::dynamic_pointer_cast<MouseClickProvider>(aoeSkillsParam->entityProvider)) {
				motion.position = curMousePos;
				activeSkillUsesMouseLoc = true;
			}
			else {
				//If it isn't a mouse click Provider then the circle should be centered on the player
				motion.position = getCenterOfEntity(entity);
			}
		}
		else {
			//If the skill isn't an AoESkillParam then don't show any range indicator
			ECS::ContainerInterface::removeAllComponentsOf(rangeIndicator);
			return;
		}
		
		
		float radius = skill->getRange();
		
		//This should make it so the scale calculation in the renderer makes this circle the right diameter
		motion.scale = vec2(radius*2 / resource.texture.size.x, radius*2 / resource.texture.size.y);
	}
}

void RangeIndicatorSystem::performActiveSkillEvent(const PerformActiveSkillEvent& event)
{
	activeSkillUsesMouseLoc = false;
	ECS::ContainerInterface::removeAllComponentsOf(rangeIndicator);
}

void RangeIndicatorSystem::mouseHoverEvent(const MouseHoverEvent event)
{
	curMousePos = event.mousePos;
	if (activeSkillUsesMouseLoc && rangeIndicator.has<Motion>()) {
		rangeIndicator.get<Motion>().position = curMousePos;
	}
}

