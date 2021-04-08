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

ECS::Entity commonInitFX(const std::string& key, const int numFrames, const bool doesCycle,
												 vec2 position, vec2 scale);

// Skill Effects
// Note, some mobs (Potato, Mashed Potato) are substantially bigger than the others and will require fx's to be scaled
// some mobs (like bosses) should not be affected by CC (like Taji's Skill 2 stun)

struct SkillFXData {
	SkillFXData(ECS::Entity refEntity, bool doesCycle)
		: refEntity(refEntity)
		, order(nextOrderId())
		, doesCycle(doesCycle)
		, offset(0.f)
	{}

	// The entity to which the FX are being applied
	ECS::Entity refEntity;

	unsigned int order;
	bool doesCycle;
	vec2 offset;

private:
	// Used to assign an order to skillFX so render order does not change if an
	// entity has multiple FXs
	static unsigned int nextOrderId() {
		static unsigned int order = 1;
		return order++;
	}
};

// Raoul's Skill 2 buffs player strength - buff effect lasts {x} turns
struct BuffedFX {
	static ECS::Entity createBuffedFX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(1.f));
};

// Chia's Skill 2 debuffs enemies - debuff lasts {x} turns
struct DebuffedFX {
	static ECS::Entity createDebuffedFX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(1.f));
};

// Any entity receiving heals should play the healedFX once
struct HealedFX {
	static ECS::Entity createHealedFX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(1.f));
};

// Chia's Skill 3 gives players shields - shield lasts {x} turns
struct ShieldedFX {
	static ECS::Entity createShieldedFX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(1.f));
};

// Taji's Skill 1 drops a candy on the enemy's head
struct Candy1FX {
	static ECS::Entity createCandy1FX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(1.f));
};

// Taji's Skill 2 drops a bigger candy on the enemy's head
struct Candy2FX {
	static ECS::Entity createCandy2FX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(1.f));
};

// (optional) Play a blueberried explosion on any target of Chia's blueberry shoot (Skill 1)
struct BlueberriedFX {
	static ECS::Entity createBlueberriedFX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(1.f));
};

// Taji's Skill 2 stuns the target for {x} (1?) turn
// *note, the resolution of this is effect is higher
struct StunnedFX {
	static ECS::Entity createStunnedFX(ECS::Entity refEntity, vec2 position, vec2 scale = vec2(0.7f));
};
