#include "ui_entities.hpp"
#include "rendering/render.hpp"
#include "rendering/text.hpp"
#include "game/game_state_system.hpp"
#include "ui/tutorials.hpp"
#include "game/turn_system.hpp"
#include <iostream>

ECS::Entity HPBar::createHPBar(vec2 position, vec2 scale)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("hp_bar");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("hp_bar.png"), "hp_bar");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::UI_ACTIVE_SKILL_FX);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	ECS::registry<HPBar>.emplace(entity);
	return entity;
}

ECS::Entity ToolTip::createToolTip(PlayerType player, SkillType skillType, vec2 position)
{
	auto entity = ECS::Entity();

	std::string skillString = "skill1";

	switch (skillType) {
	case SkillType::SKILL1:
		skillString = "skill1";
		break;
	case SkillType::SKILL2:
		skillString = "skill2";
		break;
	case SkillType::SKILL3:
		skillString = "skill3";
		break;
	default:
		entity.emplace<MoveToolTipComponent>();
		break;
	}

	ShadedMesh& resource = cacheResource(skillString + "_tooltip");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createPlayerSpecificMesh(resource, uiPath("tooltips/" + skillString), "skill_button");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TOOLTIP);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position + vec2(resource.texture.size.x / 2.f, -resource.texture.size.y) / 2.f;

	entity.emplace<SkillInfoComponent>(player, skillType);
	entity.emplace<VisibilityComponent>().isVisible = false;

	entity.emplace<ToolTip>();

	return entity;
}

ECS::Entity ToolTip::createMoveToolTip(vec2 position)
{
	auto entity = ECS::Entity();
	entity.emplace<MoveToolTipComponent>();

	ShadedMesh& resource = cacheResource("move_tooltip");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tooltips/move_tooltip.png"), "textured");
	}

	ECS::registry<ShadedMeshRef>.emplace(entity, resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TOOLTIP);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position + vec2(resource.texture.size.x / 2.f, -resource.texture.size.y) / 2.f;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2({ 1.f, 1.f });
	motion.boundingBox = vec2(0.f);

	entity.emplace<SkillInfoComponent>(PlayerType::RAOUL, SkillType::MOVE);
	entity.emplace<VisibilityComponent>().isVisible = false;

	entity.emplace<ToolTip>();

	return entity;
};

ECS::Entity TajiHelper::createTajiHelper(vec2 position, vec2 scale)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("tajihelper_static");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tutorial/taji_help/taji_help_003.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<UIComponent>();
	entity.emplace<TutorialComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TUTORIAL1);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	auto effect_anim = AnimationData("tajihelper_anim", uiPath("tutorial/taji_help/taji_help"), 10);
	AnimationsComponent& anims = entity.emplace<AnimationsComponent>(AnimationType::EFFECT, std::make_shared<AnimationData>(effect_anim));
	entity.emplace<TajiHelper>();

	return entity;
}

ECS::Entity ClickFilter::createClickFilter(vec2 position, bool doAbsorbClick, bool isLarge, vec2 scale)
{
	auto entity = ECS::Entity();

	void(*callback)() = []() {
		std::cout << "Mouse click detected within click filter." << std::endl;
		TutorialSystem::cleanTutorial();

		int index = GameStateSystem::instance().currentTutorialIndex;
		// tutorial states 0, 1, 2 proceed right away
		if (index <= 2 || index == 10)
		{
			EventSystem<AdvanceTutorialEvent>::instance().sendEvent(AdvanceTutorialEvent{});
		}
		else if (index == 11)
		{
			EventSystem<EndTutorialEvent>::instance().sendEvent(EndTutorialEvent{});
		}
		else
		{
			// fake filter to nullify all mouseclick events while waiting
			ClickFilter::createClickFilter(vec2(0.f), true, false, vec2(0.f));
		}
	};

	if (isLarge)
	{
		ShadedMesh& resource = cacheResource("clickfilter_large");
		if (resource.effect.program.resource == 0)
		{
			RenderSystem::createSprite(resource, uiPath("tutorial/clickfilter-large.png"), "textured");
		}
		entity.emplace<ShadedMeshRef>(resource);
		entity.emplace<ClickableRectangleComponent>(position, resource.texture.size.x * scale.x * 0.8f, resource.texture.size.y * scale.y * 0.8f, callback);
	}
	else
	{
		ShadedMesh& resource = cacheResource("clickfilter_small");
		if (resource.effect.program.resource == 0)
		{
			RenderSystem::createSprite(resource, uiPath("tutorial/clickfilter-small.png"), "textured");
		}
		entity.emplace<ShadedMeshRef>(resource);

		// small click filters usually overlay buttons, decrease their clickable area to ensure only the center is clickable
		entity.emplace<ClickableRectangleComponent>(position, resource.texture.size.x * scale.x * 0.5f, resource.texture.size.y * scale.y * 0.5f, callback);
	}

	entity.emplace<UIComponent>();
	entity.emplace<TutorialComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TUTORIAL2);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	entity.emplace<ClickFilter>().doAbsorbClick = doAbsorbClick;
	return entity;
}

ECS::Entity HelpOverlay::createHelpOverlay(vec2 scale)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("help_overlay");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tutorial/help-overlay.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TUTORIAL1);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.scale = scale;
	motion.position = vec2(683.f, 450.f);

	entity.emplace<HelpOverlay>();
	return entity;
}

ECS::Entity HelpButton::createHelpButton(vec2 position)
{
	// There should only ever be one of this type of entity
	while (!ECS::registry<HelpButton>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<HelpButton>.entities.back());
	}

	auto entity = ECS::Entity();

	void(*callback)() = []() {
		std::cout << "Help button clicked." << std::endl;
		if (GameStateSystem::instance().isInHelpScreen)
		{
			EventSystem<HideHelpEvent>::instance().sendEvent(HideHelpEvent{});
		}
		else if (!GameStateSystem::instance().isInTutorial)
		{
			EventSystem<ShowHelpEvent>::instance().sendEvent(ShowHelpEvent{});
		}
	};

	ShadedMesh& resource = cacheResource("help_button");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tutorial/help-button.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<ClickableRectangleComponent>(position, resource.texture.size.x, resource.texture.size.y, callback);
	entity.emplace<Button>();
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::HELP_BUTTON);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;

	entity.emplace<HelpButton>();
	return entity;
};

ECS::Entity InspectButton::createInspectButton(vec2 position)
{
	// There should only ever be one of this type of entity
	while (!ECS::registry<InspectButton>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<InspectButton>.entities.back());
	}

	auto entity = ECS::Entity();

	void(*callback)() = []() {
		TutorialSystem::toggleInspectMode();
	};

	ShadedMesh& resource = cacheResource("inspect_button");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tutorial/inspect-button.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<ClickableRectangleComponent>(position, resource.texture.size.x, resource.texture.size.y, callback);
	entity.emplace<Button>();
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::HELP_BUTTON);

	ECS::registry<Motion>.emplace(entity).position = position;

	entity.emplace<InspectButton>();
	return entity;
};

ECS::Entity ActiveArrow::createActiveArrow(vec2 position, vec2 scale)
{
	// There should only ever be one of this type of entity
	while (!ECS::registry<ActiveArrow>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<ActiveArrow>.entities.back());
	}

	auto entity = ECS::Entity();
	ShadedMesh& resource = cacheResource("active_arrow");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("active_arrow.png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	ECS::registry<ActiveArrow>.emplace(entity);
	return entity;
}

ECS::Entity AmbrosiaIcon::createAmbrosiaIcon(vec2 position, vec2 scale)
{
	auto entity = ECS::Entity();

	ShadedMesh& resource = cacheResource("ambrosia_icon");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("ambrosia-icon.png"), "textured");
	}
	entity.emplace<ShadedMeshRef>(resource);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;
	motion.scale = scale;

	entity.emplace<AmbrosiaIcon>();
	return entity;
}

ECS::Entity AmbrosiaDisplay::createAmbrosiaDisplay()
{
	// There should only ever be one of this type of entity
	while (!ECS::registry<AmbrosiaDisplay>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<AmbrosiaDisplay>.entities.back());
	}

	static constexpr vec2 AMBROSIA_SCALE(0.8f);

	auto entity = AmbrosiaIcon::createAmbrosiaIcon(AMBROSIA_DISPLAY_OFFSET, AMBROSIA_SCALE);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::HELP_BUTTON);
	entity.emplace<AmbrosiaDisplay>();

	return entity;
}

ECS::Entity MobCard::createMobCard(vec2 position, const std::string& mobType)
{
	auto entity = ECS::Entity();
	ShadedMesh& resource = cacheResource(mobType + "_card");
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("mob_cards/" + mobType + ".png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<UIComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI_TUTORIAL1);

	ECS::registry<Motion>.emplace(entity).position = position;

	entity.emplace<MobCard>();
	return entity;
}
