#pragma once
#include "game/common.hpp"
#include "entities/tiny_ecs.hpp"
#include "ui/ui_components.hpp"

struct MouseClickFX
{
	static ECS::Entity createMouseClickFX();
};

struct ActiveSkillFX
{
	static ECS::Entity createActiveSkillFX();
};

ECS::Entity commonInitFX(const std::string& key, const int numFrames, const bool doesCycle);

// Skill Effects
// Note, some mobs (Potato, Mashed Potato) are substantially bigger than the others and will require fx's to be scaled
// some mobs (like bosses) should not be affected by CC (like Taji's Skill 2 stun)

// Raoul's Skill 2 buffs player strength - buff effect lasts {x} turns
struct BuffedFX {
	static ECS::Entity createBuffedFX(vec2 position, vec2 scale = vec2(1.f));
	ECS::Entity fxEntity;
};

// Chia's Skill 2 debuffs enemies - debuff lasts {x} turns
struct DebuffedFX {
	static ECS::Entity createDebuffedFX(vec2 position, vec2 scale = vec2(1.f));
	ECS::Entity fxEntity;
};

// Any entity receiving heals should play the healedFX once
struct HealedFX {
	static ECS::Entity createHealedFX(vec2 position, vec2 scale = vec2(1.f));
	ECS::Entity fxEntity;
};

// Chia's Skill 3 gives players shields - shield lasts {x} turns
struct ShieldedFX { 
	static ECS::Entity createShieldedFX(vec2 position, vec2 scale = vec2(1.f));
	ECS::Entity fxEntity;
};

// Taji's Skill 1 drops a candy on the enemy's head
struct Candy1FX {
	static ECS::Entity createCandy1FX(vec2 position, vec2 scale = vec2(1.f));
	ECS::Entity fxEntity;
};

// Taji's Skill 2 drops a bigger candy on the enemy's head
struct Candy2FX {
	static ECS::Entity createCandy2FX(vec2 position, vec2 scale = vec2(1.f));
	ECS::Entity fxEntity;
};

// (optional) Play a blueberried explosion on any target of Chia's blueberry shoot (Skill 1)
struct BlueberriedFX {
	static ECS::Entity createBlueberriedFX(vec2 position, vec2 scale = vec2(1.f));
	ECS::Entity fxEntity;
};

// Taji's Skill 2 stuns the target for {x} (1?) turn
// *note, the resolution of this is effect is higher
struct StunnedFX {
	static ECS::Entity createStunnedFX(vec2 position, vec2 scale = vec2(0.7f));
	ECS::Entity fxEntity;
};

// ECS-style Label
struct SkillFX {
	vec2 offset = { 0.f, 0.f };
	unsigned int order = 0;
};