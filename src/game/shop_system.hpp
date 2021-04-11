#pragma once
#include "button.hpp"
#include "effects.hpp"
#include <functional>
#include <iostream>
#include <optional>

class ShopSystem
{
	protected:
		SkillType selected_skill;
		PlayerType selected_player;
		
		ECS::Entity getPlayerEntity(PlayerType player);

		void executeShopEffect(SkillType skillType, PlayerType player, int index);

		bool checkIfAbleToBuy(int level);

		void drawButtons();
		void renderLabels();
		void renderAmbrosia();

		std::vector<std::string> descriptions = {
			"Increase Raoul's damage and health by 10%",
			"Increase the damage of this skill by 5",
			"Increase the area and strength of this buff by 10%",
			"Increase the damage of the projectile by 5",
			"Increase Taji's damage and health by 10%",
			"Increase the area by 10% and damage by 5",
			"Increase the area by 25% and damage by 10",
			"Increase both heal and damage of this skill by 4",
			"Increase Chia's damage and health by 10%",
			"Increase both heal and damage of this skill by 10",
			"Increase the debuff strength by 10% and damage by 5",
			"Increase the shield strength by 10 and duration by 1",
			"Increase Ember's damage and health by 10%",
			"Increase area by 10% and damage by 5",
			"Increase area by 10% and damage by 8",
			"Increase area, damage, and knockback by 10%",
		};

		ECS::Entity raoul;
		ECS::Entity chia;
		ECS::Entity ember;
		ECS::Entity taji;
	public:
		static ShopSystem& instance() {
			static ShopSystem shopSystem;
			return shopSystem;
		}

		int selected = -1;
		ECS::Entity activeFX;
		std::vector<ECS::Entity> buttons;

		void buySelectedSkill();
		void initialize(ECS::Entity raoul, ECS::Entity chia, ECS::Entity ember, ECS::Entity taji);
};

