#include "tutorials.hpp"
#include "game/game_state_system.hpp"
#include "game/turn_system.hpp"
#include "ui/ui_system.hpp"
#include "maps/map.hpp"
#include "ai/ai.hpp"
#include "physics/physics.hpp"
#include "entities/enemies.hpp"
#include <iostream>

TutorialSystem::TutorialSystem()
{
	tutorialStartListener = EventSystem<StartTutorialEvent>::instance().registerListener(
		std::bind(&TutorialSystem::onTutorialStart, this, std::placeholders::_1));

	tutorialAdvanceListener = EventSystem<AdvanceTutorialEvent>::instance().registerListener(
		std::bind(&TutorialSystem::onTutorialAdvance, this, std::placeholders::_1));

	tutorialEndListener = EventSystem<EndTutorialEvent>::instance().registerListener(
		std::bind(&TutorialSystem::onTutorialEnd, this, std::placeholders::_1));

	hideHelpListener = EventSystem<HideHelpEvent>::instance().registerListener(
		std::bind(&TutorialSystem::onHideHelp, this, std::placeholders::_1));

	showHelpListener = EventSystem<ShowHelpEvent>::instance().registerListener(
		std::bind(&TutorialSystem::onShowHelp, this, std::placeholders::_1));

	advanceStoryListener = EventSystem<AdvanceStoryEvent>::instance().registerListener(
		std::bind(&TutorialSystem::onAdvanceStory, this, std::placeholders::_1));

	onMouseHoverListener = EventSystem<MouseHoverEvent>::instance().registerListener(
		std::bind(&TutorialSystem::onMouseHover, this, std::placeholders::_1));
};

TutorialSystem::~TutorialSystem()
{
	if (tutorialStartListener.isValid()) {
		EventSystem<StartTutorialEvent>::instance().unregisterListener(tutorialStartListener);
	}

	if (tutorialAdvanceListener.isValid()) {
		EventSystem<AdvanceTutorialEvent>::instance().unregisterListener(tutorialAdvanceListener);
	}

	if (tutorialEndListener.isValid()) {
		EventSystem<EndTutorialEvent>::instance().unregisterListener(tutorialEndListener);
	}

	if (hideHelpListener.isValid()) {
		EventSystem<HideHelpEvent>::instance().unregisterListener(hideHelpListener);
	}

	if (showHelpListener.isValid()) {
		EventSystem<ShowHelpEvent>::instance().unregisterListener(showHelpListener);
	}

	if (advanceStoryListener.isValid()) {
		EventSystem<AdvanceStoryEvent>::instance().unregisterListener(advanceStoryListener);
	}

	if (onMouseHoverListener.isValid()) {
		EventSystem<MouseHoverEvent>::instance().unregisterListener(onMouseHoverListener);
	}
};

void TutorialSystem::onTutorialStart(const StartTutorialEvent& event)
{
	assert(GameStateSystem::instance().isInTutorial && GameStateSystem::instance().currentTutorialIndex == 0);
	vec2 screenSize = GameStateSystem::instance().getScreenBufferSize();
	TajiHelper::createTajiHelper(screenSize / 2.f + vec2(-250.f, 60.f), vec2(-1.f, 1.f));
	ClickFilter::createClickFilter(screenSize / 2.f + vec2(-130.f, 40.f), true);
	TutorialText::createTutorialText(screenSize / 2.f + vec2(0.f, -120.f), 0);
};

void TutorialSystem::onTutorialAdvance(const AdvanceTutorialEvent& event)
{
	TutorialSystem::cleanTutorial();
	int index = ++GameStateSystem::instance().currentTutorialIndex;
	std::cout << "Advancing Tutorial. Displaying Tutorial index: " << index << std::endl;
	vec2 screenSize = GameStateSystem::instance().getScreenBufferSize();

	switch(index)
	{
		case 1: // Raoul is the active player
			TajiHelper::createTajiHelper(screenSize / 2.f + vec2(20.f, -300.f));
			ClickFilter::createClickFilter(screenSize / 2.f + vec2(-38.f, -100.f), true, true, vec2(0.35f, 1.f));
			TutorialText::createTutorialText(screenSize / 2.f + vec2(380.f, -100.f), 1);
			break;
		case 2: // Point out Raoul's active player button
			TajiHelper::createTajiHelper(vec2(screenSize.x / 2.f - 500.f, 80.f), vec2(-1.f, 1.f));
			ClickFilter::createClickFilter(vec2(screenSize.x / 2.f, 160.f), true, true, vec2(1.2f, 0.85f));
			TutorialText::createTutorialText(vec2(screenSize.x / 2.f - 50.f, 200.f), 2);
			break;
		case 3: // Activate move button - wait move button activate
			TajiHelper::createTajiHelper(vec2(250.f, screenSize.y - 80.f));
			ClickFilter::createClickFilter(vec2(100.f, screenSize.y - 80.f));
			TutorialText::createTutorialText(vec2(370.f, screenSize.y - 300.f), 3);
			break;
		case 4: // Click on map to move - wait stop moving
			TajiHelper::createTajiHelper(screenSize / 2.f + vec2(10.f, -40.f), vec2(-1.f, 1.f));
			ClickFilter::createClickFilter(screenSize / 2.f + vec2(150.f, -40.f), false, false, vec2(0.7f));
			TutorialText::createTutorialText(screenSize / 2.f + vec2(300.f, -160.f), 4);
			break;
		case 5: // Activate Raoul Skill 1 - wait skill button activate
			TajiHelper::createTajiHelper(vec2(420.f, screenSize.y - 80.f));
			ClickFilter::createClickFilter(vec2(250.f, screenSize.y - 80.f));
			TutorialText::createTutorialText(vec2(550.f, screenSize.y - 250.f), 5);
			break;
		case 6: // Use Raoul Skill 1 on mob - wait player switch
			TajiHelper::createTajiHelper(vec2(1200, 400));
			ClickFilter::createClickFilter(screenSize / 2.f + vec2(340.f, -150.f), false, true, vec2(0.55f, 1.1f));
			TutorialText::createTutorialText(screenSize / 2.f + vec2(-220.f, -100.f), 6);
			break;
		case 7: // Switch players to Chia - wait player switch 
			TajiHelper::createTajiHelper(vec2(screenSize.x / 2.f + 430.f, 80.f));
			ClickFilter::createClickFilter(vec2(screenSize.x / 2.f + 300.f, 60.f), false, false, vec2(1.3f, 0.55f));
			TutorialText::createTutorialText(vec2(screenSize.x / 2.f, 210.f), 7);
			break;
		case 8: // Activate Chia's Skill 3 - wait skill activate
			TajiHelper::createTajiHelper(vec2(650, screenSize.y - 50.f));
			ClickFilter::createClickFilter(vec2(550.f, screenSize.y - 80.f));
			TutorialText::createTutorialText(vec2(screenSize.x / 2.f + 50.f, screenSize.y - 300.f), 8);
			break;
		case 9: // Use Chia's Skill 3 - wait player switch 
			TajiHelper::createTajiHelper(screenSize / 2.f + vec2(100.f, 50.f));
			ClickFilter::createClickFilter(screenSize / 2.f + vec2(0.f, -60.f), false, true, vec2(1.5f, 1.7f));
			TutorialText::createTutorialText(vec2(screenSize.x / 2.f, 300.f), 9);
			break;
		case 10: // Turns end when you use your skill
			TajiHelper::createTajiHelper(vec2(screenSize.x / 2.f + 500.f, 80.f));
			ClickFilter::createClickFilter(vec2(screenSize.x / 2.f, 220.f), true, true, vec2(1.2f, 1.2f));
			TutorialText::createTutorialText(vec2(screenSize.x / 2.f, 250.f), 10);
			break;
		case 11: // Conclusion
			TajiHelper::createTajiHelper(screenSize / 2.f + vec2(0.f, 100.f));
			ClickFilter::createClickFilter(screenSize / 2.f + vec2(0.f, -60.f), true, true, vec2(1.5f, 1.7f));
			TutorialText::createTutorialText(vec2(screenSize.x / 2.f, 300.f), 11);
			break;
		default:
			break;
	}
};

void TutorialSystem::onTutorialEnd(const EndTutorialEvent& event)
{
	GameStateSystem::instance().isInTutorial = false;
	GameStateSystem::instance().currentTutorialIndex = 0;
	TutorialSystem::cleanTutorial();

	// Send achievement event
	EventSystem<FinishedTutorialEvent>::instance().sendEvent(FinishedTutorialEvent{});
};

void TutorialSystem::onShowHelp(const ShowHelpEvent& event)
{
	if (!GameStateSystem::instance().isInTutorial)
	{
		HelpOverlay::createHelpOverlay();
		GameStateSystem::instance().isInHelpScreen = true;
	}
};

void TutorialSystem::onHideHelp(const HideHelpEvent& event)
{
	while (!ECS::registry<HelpOverlay>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<HelpOverlay>.entities.back());
	}
	GameStateSystem::instance().isInHelpScreen = false;
};


void TutorialSystem::cleanTutorial()
{
	while (!ECS::registry<TutorialComponent>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<TutorialComponent>.entities.back());
	}
};

ECS::Entity TutorialText::createTutorialText(vec2 position, int tutorialStage)
{
	auto entity = ECS::Entity();

	std::string tutorialKey = "tutorial_" + std::to_string(tutorialStage);
	ShadedMesh& resource = cacheResource(tutorialKey);
	if (resource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(resource, uiPath("tutorial/text/" + tutorialKey + ".png"), "textured");
	}

	entity.emplace<ShadedMeshRef>(resource);
	entity.emplace<UIComponent>();
	entity.emplace<TutorialComponent>();
	entity.emplace<RenderableComponent>(RenderLayer::UI);

	auto& motion = ECS::registry<Motion>.emplace(entity);
	motion.position = position;

	entity.emplace<TutorialText>();
	return entity;
};

ECS::Entity StoryScene::createStoryScene(vec2 screenSize, int storyStage, bool hasAmbrosia)
{
	assert(0 <= storyStage);

	auto background = ECS::Entity();
	std::string key = "story-" + std::to_string(storyStage);
	ShadedMesh& storyResource = cacheResource(key);
	if (storyResource.effect.program.resource == 0)
	{
		RenderSystem::createSprite(storyResource, uiPath("story/" + key + ".png"), "textured");
	}
	background.emplace<ShadedMeshRef>(storyResource);
	background.emplace<RenderableComponent>(RenderLayer::MAP);
	background.emplace<Motion>();
	auto& mapComponent = background.emplace<MapComponent>();
	mapComponent.name = key;
	mapComponent.mapSize = static_cast<vec2>(storyResource.texture.size);
	background.emplace<StoryComponent>();

	if (hasAmbrosia)
	{
		// Glow
		auto glow = ECS::Entity();
		ShadedMesh& glowResource = cacheResource("story-glow");
		if (glowResource.effect.program.resource == 0)
		{
			RenderSystem::createSprite(glowResource, uiPath("story/glow.png"), "fading");
		}
		glow.emplace<ShadedMeshRef>(glowResource);
		glow.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);
		glow.emplace<Motion>().position = vec2(screenSize.x / 2, 300.f);
		glow.emplace<StoryComponent>();
	}

	return background;
};

void TutorialSystem::onAdvanceStory(const AdvanceStoryEvent& event)
{
	while (!ECS::registry<StoryComponent>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<StoryComponent>.entities.back());
	}

	int storyStage = GameStateSystem::instance().currentStoryIndex;
	// done story, begin tutorial
	if (storyStage > 9)
	{
		GameStateSystem::instance().isTransitioning = true;
		TransitionEvent event;
		event.callback = []() {
			GameStateSystem::instance().beginTutorial();
		};
		EventSystem<TransitionEvent>::instance().sendEvent(event);
		return;
	}

	StoryScene::createStoryScene(GameStateSystem::instance().getScreenBufferSize(), storyStage);
};


void TutorialSystem::toggleInspectMode()
{
	std::cout << "Inspect button clicked." << std::endl;
	if (GameStateSystem::instance().isInHelpScreen || GameStateSystem::instance().isInTutorial)
	{
		std::cout << "Cannot inspect when in help screen or during tutorial." << std::endl;
		return;
	}

	ECS::Entity activeEntity = ECS::registry<TurnSystem::TurnComponentIsActive>.entities[0];
	if (!activeEntity.has<PlayerComponent>() || activeEntity.has<DeathTimer>())
	{
		std::cout << "Cannot inspect. The active entity is not a valid player." << std::endl;
		UISystem::createCentralMessage("Cannot inspect. Please wait until a player turn.");
		return;
	}

	auto& turnComponent = activeEntity.get<TurnSystem::TurnComponent>();
	if (turnComponent.isMoving || turnComponent.isUsingSkill)
	{
		std::cout << "Cannot inspect. The active player is busy moving or using a skill." << std::endl;
		UISystem::createCentralMessage("Cannot inspect. The active player is busy.");
		return;
	}

	if (GameStateSystem::instance().isInspecting)
	{
		std::cout << "Exiting inspect mode." << std::endl;
		GameStateSystem::instance().isInspecting = false;
		EventSystem<EndInspectEvent>::instance().sendEvent(EndInspectEvent{});
		UISystem::createCentralMessage("Left inspect mode.");
	}
	else
	{
		std::cout << "Entering inspect mode." << std::endl;
		UISystem::createCentralMessage("Entering inspect mode. Hover over an enemy to see details and tips.");
		GameStateSystem::instance().isInspecting = true;
		EventSystem<BeginInspectEvent>::instance().sendEvent(BeginInspectEvent{});
	}
};

void TutorialSystem::onMouseHover(const MouseHoverEvent& event)
{
	// Clear all mob cards regardless
	while (!ECS::registry<MobCard>.entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<MobCard>.entities.back());
	}

	// Only handle Inspecting Mode
	if (!GameStateSystem::instance().isInspecting)
	{
		return;
	}

	// Check if there is a mob beneath the mouse hover
	// If there are multiple mobs, get the mob whose position is closest to the mouse
	float closestDist = 10000.f;
	bool didFindMob = false;
	ECS::Entity closestMobEntity;

	// Lambda to check if point position is within an entity's bounding box
	auto isOverEntity = [=](const vec2& point, const BoundingBox& box)
	{
		return (box.left < point.x) && (box.right > point.x) &&
			(box.top < point.y) && (box.bottom > point.y);
	};

	// Check if hovering over any alive mobs
	for (auto& mob : ECS::registry<AISystem::MobComponent>.entities)
	{
		if (mob.has<DeathTimer>() || !mob.has<Motion>())
		{
			continue;
		}

		auto& motionC = mob.get<Motion>();
		auto mobBB = PhysicsSystem::getBoundingBox(mob, motionC);

		if (!isOverEntity(event.mousePos, mobBB))
		{
			// not hovering over this mob
			continue;
		}

		// is hovering over mob, check if mob is closer to mouse
		didFindMob = true;
		float distToMouse = distance(motionC.position, event.mousePos);
		if (distToMouse < closestDist)
		{
			closestDist = distToMouse;
			closestMobEntity = mob;
		}
	}

	if (didFindMob)
	{
		showInspectToolTip(closestMobEntity);
	}
};

void TutorialSystem::showInspectToolTip(ECS::Entity& entity)
{
	vec2 position = vec2(683.f, 400.f); // center of screen
	if (entity.has<Egg>())
	{
		MobCard::createMobCard(position, "egg");
	}
	else if (entity.has<Pepper>())
	{
		MobCard::createMobCard(position, "pepper");
	}
	else if (entity.has<Tomato>())
	{
		MobCard::createMobCard(position, "tomato");
	}
	else if (entity.has<Chicken>())
	{
		MobCard::createMobCard(position, "chicken");
	}
	else if (entity.has<Lettuce>())
	{
		MobCard::createMobCard(position, "lettuce");
	}
	else if (entity.has<MashedPotato>())
	{
		MobCard::createMobCard(position, "mashed-potato");
	}
	else if (entity.has<Potato>())
	{
		MobCard::createMobCard(position, "potato");
	}
	else if (entity.has<PotatoChunk>())
	{
		MobCard::createMobCard(position, "potato-chunk");
	}
	else if (entity.has<SaltnPepper>())
	{
		MobCard::createMobCard(position, "saltnpepper");
	}
	else if (entity.has<Milk>())
	{
		MobCard::createMobCard(position, "milk");
	}
}