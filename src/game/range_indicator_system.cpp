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
	skillAvtiveListener = EventSystem<SetActiveSkillEvent>::instance().registerListener(
		std::bind(&RangeIndicatorSystem::skillActiveEvent, this, std::placeholders::_1));

	performActiveSkillListener = EventSystem<PerformActiveSkillEvent>::instance().registerListener(
		std::bind(&RangeIndicatorSystem::performAcvtiveSkillEvent, this, std::placeholders::_1));

	performActiveSkillListener = EventSystem<MouseHoverEvent>::instance().registerListener(
		std::bind(&RangeIndicatorSystem::mouseHoverEvent, this, std::placeholders::_1));

	activeSkillUsesMouseLoc = false;
}

void RangeIndicatorSystem::skillActiveEvent(const SetActiveSkillEvent& event)
{
	//Remove the old range indicator
	

	auto entity = event.entity;
	auto playerMotion = entity.get<Motion>();
	if (entity.has<SkillComponent>()&& entity.has<PlayerComponent>())
	{
		ECS::ContainerInterface::removeAllComponentsOf(rangeIndicator);
		activeSkillUsesMouseLoc = false;
		

		//Create a range indicator
		rangeIndicator = ECS::Entity();
		ShadedMesh& resource = cacheResource("range_indicator");
		if (resource.effect.program.resource == 0)
		{
			RenderSystem::createSprite(resource, texturesPath("circle-blue-overlay.png"), "textured");
		}

		rangeIndicator.emplace<ShadedMeshRef>(resource);
		//TODO what RenderLayer should this be on
		rangeIndicator.emplace<RenderableComponent>(RenderLayer::CLICK_FX);

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
		
		//This should make it so the scale calculation in the renderer makes this cirlce the right diameter
		motion.scale = vec2(radius / resource.texture.size.x, radius / resource.texture.size.y);
	}
}

void RangeIndicatorSystem::performAcvtiveSkillEvent(const PerformActiveSkillEvent& event)
{
	activeSkillUsesMouseLoc = false;
	ECS::ContainerInterface::removeAllComponentsOf(rangeIndicator);
}

void RangeIndicatorSystem::mouseHoverEvent(const MouseHoverEvent event)
{
	curMousePos = event.mousePos;
	if (activeSkillUsesMouseLoc) {
		assert(rangeIndicator.has<Motion>());
		rangeIndicator.get<Motion>().position = curMousePos;
	}
}
