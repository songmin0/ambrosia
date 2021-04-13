#pragma once
#include <button.hpp>
#include <effects.hpp>
#include <functional>
#include <iostream>
#include <optional>

class ShopSystem
{
	protected:
		SkillType selected_skill;
		PlayerType selected_player;
		int cost_multiplier = 5;
		
		ECS::Entity getPlayerEntity(PlayerType player);

		void printDescriptionText(std::string text);

		void executeShopEffect(SkillType skillType, PlayerType player, int index);

		int getCost();

		bool checkIfAbleToBuy(int level);

		void drawButtons();
		void renderLabels();
		void renderAmbrosia();

		std::vector<std::string> descriptions = {
			"Increase Raoul's damage and health by 10%",
			"Increase the damage of this skill by 5",
			"Increase the range and strength of this buff by 10%",
			"Increase the damage of the projectile by 5",
			"Increase Taji's damage and health by 10%",
			"Increase the range by 10% and damage by 5",
			"Increase damage by 5. At max level, increases stun duration by 1 turn",
			"Increase both heal and damage of this skill by 4",
			"Increase Chia's damage and health by 10%",
			"Increase both heal and damage of this skill by 10",
			"Increase the debuff strength by 10% and damage by 5",
			"Increase the shield strength by 10 and duration by 1 turn",
			"Increase Ember's damage and health by 10%",
			"Increase range by 10% and damage by 5",
			"Increase range by 10% and damage by 8",
			"Increase range and knockback by 10% and damage by 5",
		};

		ECS::Entity raoul;
		ECS::Entity chia;
		ECS::Entity ember;
		ECS::Entity taji;

		int selected = -1;
		ECS::Entity activeFX;
		std::vector<ECS::Entity> buttons;

	public:
		static ShopSystem& instance() {
			static ShopSystem shopSystem;
			return shopSystem;
		}
		void buySelectedSkill();
		void initialize(ECS::Entity raoul, ECS::Entity chia, ECS::Entity ember, ECS::Entity taji);
};

