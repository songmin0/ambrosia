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


		// TODO : this is temporary until ambrosia drops are added to the game
		
		ECS::Entity getPlayerEntity(PlayerType player);

		void executeShopEffect(SkillType skillType, PlayerType player, int index);

		bool checkIfAbleToBuy(int level);

		void drawButtons();
		void renderLabels();
		void renderAmbrosia();
		void updateAmbrosia();

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

