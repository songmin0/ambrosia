#pragma once
#include "button.hpp"
#include "effects.hpp"
#include <functional>
#include <iostream>
#include <optional>

class ShopSystem
{
	protected:
		int selected_skill;
		std::string selected_player;
		
		void executeShopEffect(int i, std::string player, int index);

	public:
		static ShopSystem& instance() {
			static ShopSystem shopSystem;
			return shopSystem;
		}

		int selected = -1;
		ECS::Entity activeFX;
		std::vector<ECS::Entity> buttons;

		void drawButtons();
		void renderLabels();

};

