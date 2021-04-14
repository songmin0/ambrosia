#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "skills/skill_component.hpp"
#include "ui_components.hpp"

struct HPBar
{
	static ECS::Entity createHPBar(vec2 position, vec2 scale = vec2(0.5f, 0.4f));
	vec2 offset = vec2(0.f);
	ECS::Entity statsCompEntity;
	bool isMob = false;
};

struct ToolTip
{
	static ECS::Entity createToolTip(PlayerType player, SkillType skillType, vec2 position = vec2(0.f));
	static ECS::Entity createMoveToolTip(vec2 position = vec2(0.f));
};

struct TajiHelper { static ECS::Entity createTajiHelper(vec2 position, vec2 scale = vec2(1.f)); };
struct ClickFilter 
{ 
	static ECS::Entity createClickFilter(vec2 position, bool doAbsorbClick = false, bool isLarge = false, vec2 scale = vec2(1.f));
	bool doAbsorbClick = false; // if true, clicks within the clickfilter will not allow clicks to pass (be "absorbed")
};

struct HelpOverlay { static ECS::Entity createHelpOverlay(vec2 scale = vec2(1.f)); };
struct HelpButton { static ECS::Entity createHelpButton(vec2 position); };
struct InspectButton { static ECS::Entity createInspectButton(vec2 position); };

struct ActiveArrow {
	static ECS::Entity createActiveArrow(vec2 position = vec2(0.f), vec2 scale = vec2(1.f));
	vec2 offset = { 0.f, -275.f };
};

// Creates an entity that manages an ambrosia icon
struct AmbrosiaIcon
{
	static ECS::Entity createAmbrosiaIcon(vec2 position, vec2 scale);
};

// Creates an entity that manages the ambrosia icon and text in the top-left
// corner of the screen
struct AmbrosiaDisplay
{
	static ECS::Entity createAmbrosiaDisplay();
	float textScale = 0.75f;
	vec2 textOffset = {40.f, 16.f};
};

struct MobCard { static ECS::Entity createMobCard(vec2 position, const std::string& mobType); };