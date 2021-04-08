#include "shop_system.hpp"
#include <ui/button.hpp>
#include <ui/ui_components.hpp>
#include <ui/ui_entities.hpp>
#include <effects/effects.hpp>
#include "rendering/text.hpp"

void ShopSystem::executeShopEffect(int i, std::string player, int index)
{
	ShopSystem::instance().selected_player = player;
	ShopSystem::instance().selected_skill = i;

	if (ShopSystem::instance().selected == -1) {
		ShopSystem::instance().activeFX = ActiveSkillFX::createActiveSkillFX();
	}

	ShopSystem::instance().selected = index;

	ECS::registry<Motion>.get(ShopSystem::instance().activeFX).position = { ECS::registry<Motion>.get(buttons.at(index)).position };
	std::cout << "selected skill " << i << "from " << player;

	// render upgrade description
	ECS::registry<Text>.clear();
	createText(player + "'s skill " + std::to_string(i), {1000, 350} , 0.5);
	createText(player + "'s skill " + std::to_string(i), { 1000, 400 }, 0.5);
	createText(player + "'s skill " + std::to_string(i), { 1000, 450 }, 0.5);
	createText(player + "'s skill " + std::to_string(i), { 1000, 500 }, 0.5);

	// render labels
	renderLabels();
}

void ShopSystem::drawButtons() {

	// Create the buttons and their effects

	int starting_x = 160;
	int x_gap = 225;

	int starting_y = 155;
	int y_gap = 195;

	int i = 0; // i-th gap for x
	int j = 0; // j-th gap for y

	/// Raoul //////////////////////////////////////////

	std::string path = "shop/raoul";

	buttons.push_back(
		Button::createButton(ButtonShape::CIRCLE,
			{ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, path,
			[]() {
				ShopSystem::instance().executeShopEffect(0, "raoul", 0);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::RAOUL, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(1, "raoul", 1);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::RAOUL, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(2, "raoul", 2);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::RAOUL, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(3, "raoul", 3);
			})
	);

	/// Taji //////////////////////////////////////////

	i = 0;
	j++;

	path = "shop/taji";

	buttons.push_back(
		Button::createButton(ButtonShape::CIRCLE,
			{ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, path,
			[]() {
				ShopSystem::instance().executeShopEffect(0, "taji", 4);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::TAJI, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(1, "taji", 5);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::TAJI, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(2, "taji", 6);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::TAJI, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(3, "taji", 7);
			})
	);

	/// Chia //////////////////////////////////////////

	i = 0;
	j++;

	path = "shop/chia";

	buttons.push_back(
		Button::createButton(ButtonShape::CIRCLE,
			{ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, path,
			[]() {
				ShopSystem::instance().executeShopEffect(0, "chia", 8);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::CHIA, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(1, "chia", 9);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::CHIA, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(2, "chia", 10);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::CHIA, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(3, "chia", 11);
			})
	);

	/// Ember //////////////////////////////////////////

	i = 0;
	j++;

	path = "shop/ember";

	buttons.push_back(
		Button::createButton(ButtonShape::CIRCLE,
			{ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, path,
			[]() {
				ShopSystem::instance().executeShopEffect(0, "ember", 12);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::EMBER, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(1, "ember", 13);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::EMBER, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(2, "ember", 14);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::EMBER, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(3, "ember", 15);
			})
	);

}

void ShopSystem::renderLabels()
{
	for (ECS::Entity e : buttons) {
		vec2 button_pos = ECS::registry<Motion>.get(e).position;

		createText("LVL. 1", { button_pos.x - 40 , button_pos.y + 100}, 0.5);
	}
}
