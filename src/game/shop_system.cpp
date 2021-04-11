#include "shop_system.hpp"
#include <ui/button.hpp>
#include <ui/ui_components.hpp>
#include <ui/ui_entities.hpp>
#include <effects/effects.hpp>
#include "rendering/text.hpp"
#include <game/game_state_system.hpp>

std::string getPlayerName(PlayerType player) {
	switch (player)
	{
	case PlayerType::RAOUL:
		return "raoul";
		break;
	case PlayerType::TAJI:
		return "taji";
		break;
	case PlayerType::EMBER:
		return "ember";
		break;
	case PlayerType::CHIA:
		return "chia";
		break;
	default:
		return "";
		break;
	}
	return "";
}

ECS::Entity ShopSystem::getPlayerEntity(PlayerType player) {
	switch (player)
	{
	case PlayerType::RAOUL:
		return raoul;
		break;
	case PlayerType::TAJI:
		return taji;
		break;
	case PlayerType::EMBER:
		return ember;
		break;
	case PlayerType::CHIA:
		return chia;
		break;
	default:
		break;
	}
	return raoul;
}


void ShopSystem::executeShopEffect(SkillType skillType, PlayerType player, int index)
{
	ShopSystem::instance().selected_player = player;
	ShopSystem::instance().selected_skill = skillType;

	if (ShopSystem::instance().selected == -1) {
		// initialize selection fx on first selection
		ShopSystem::instance().activeFX = ActiveSkillFX::createActiveSkillFX();
	}

	ShopSystem::instance().selected = index;

	ECS::registry<Motion>.get(ShopSystem::instance().activeFX).position = { ECS::registry<Motion>.get(buttons.at(index)).position };

	// render upgrade description
	ECS::registry<Text>.clear();
	createText(getPlayerName(player) + "'s skill " + std::to_string((int)skillType), {1000, 350} , 0.5);
	createText(getPlayerName(player) + "'s skill " + std::to_string((int)skillType), { 1000, 400 }, 0.5);
	createText(getPlayerName(player) + "'s skill " + std::to_string((int)skillType), { 1000, 450 }, 0.5);
	createText(getPlayerName(player) + "'s skill " + std::to_string((int)skillType), { 1000, 500 }, 0.5);

	// render labels
	renderLabels();
}

bool ShopSystem::checkIfAbleToBuy(int level) {
	auto ambrosia = GameStateSystem::instance().getAmbrosia();
	if (ambrosia >= level) {
		ambrosia -= level;
		GameStateSystem::instance().setAmbrosia(ambrosia);
		return true;
	}
	return false;
}

void ShopSystem::buySelectedSkill() {
	if (ShopSystem::instance().selected == -1) {
		// nothing is selected
		std::cout << "Nothing to buy";
		return;
	}

	int level;

	std::cout << "Buying " << getPlayerName(selected_player) << "'s skill " << (int)selected_skill << std::endl;
	SkillComponent component;
	

	
	
	if (selected_skill == SkillType::NONE) {
		StatsComponent stats;
		stats = ECS::registry<StatsComponent>.get(getPlayerEntity(selected_player));
		if (!stats.atMaxLevel()) {
			checkIfAbleToBuy(stats.getStatValue(StatType::LEVEL));
		}
		else {
			std::cout << "player level maxed already!" << std::endl;
			return;
		}
	}

	component = ECS::registry<SkillComponent>.get(getPlayerEntity(selected_player));
	if (component.getSkillLevel(selected_skill) == component.getMaxLevel(selected_skill)) {
		std::cout << "level maxed already!" << std::endl;
		return;
	}

	if (checkIfAbleToBuy(component.getSkillLevel(selected_skill))) {
		ECS::registry<SkillComponent>.get(getPlayerEntity(selected_player)).upgradeSkillLevel(selected_skill);
	}

	//std::cout << "Skill level has now went from " << component. - 1 << "to " << level << std::endl;

	ECS::registry<Text>.clear();
	renderLabels();
}

void ShopSystem::initialize(ECS::Entity raoul, ECS::Entity chia, ECS::Entity ember, ECS::Entity taji)
{
	this->raoul = raoul;
	this->chia = chia;
	this->ember = ember;
	this->taji = taji;

	drawButtons();
	renderAmbrosia();
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
				ShopSystem::instance().executeShopEffect(SkillType::NONE, PlayerType::RAOUL, 0);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::RAOUL, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL1, PlayerType::RAOUL, 1);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::RAOUL, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL2, PlayerType::RAOUL, 2);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::RAOUL, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL3, PlayerType::RAOUL, 3);
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
				ShopSystem::instance().executeShopEffect(SkillType::NONE, PlayerType::TAJI, 4);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::TAJI, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL1, PlayerType::TAJI, 5);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::TAJI, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL2, PlayerType::TAJI, 6);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::TAJI, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL3, PlayerType::TAJI, 7);
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
				ShopSystem::instance().executeShopEffect(SkillType::NONE, PlayerType::CHIA, 8);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::CHIA, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL1, PlayerType::CHIA, 9);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::CHIA, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL2, PlayerType::CHIA, 10);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::CHIA, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL3, PlayerType::CHIA, 11);
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
				ShopSystem::instance().executeShopEffect(SkillType::NONE, PlayerType::EMBER, 12);
			})
	);



	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::EMBER, SkillType::SKILL1, "skill1",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL1, PlayerType::EMBER, 13);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::EMBER, SkillType::SKILL2, "skill2",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL2, PlayerType::EMBER, 14);
			})
	);

	i++;

	buttons.push_back(
		UpgradeButton::createUpgradeButton({ starting_x + (x_gap * i) , starting_y + (y_gap * j) }, PlayerType::EMBER, SkillType::SKILL3, "skill3",
			[]() {
				ShopSystem::instance().executeShopEffect(SkillType::SKILL3, PlayerType::EMBER, 15);
			})
	);

}

void ShopSystem::renderLabels()
{
	for (ECS::Entity e : buttons) {
		vec2 button_pos = ECS::registry<Motion>.get(e).position;

		if (!ECS::registry<SkillInfoComponent>.has(e)) {
			createText("LVL. 1", { button_pos.x - 40 , button_pos.y + 100 }, 0.5);
		}
		else {
			auto buttonInfo = ECS::registry<SkillInfoComponent>.get(e);
			auto skillComponent = ECS::registry<SkillComponent>.get(getPlayerEntity(buttonInfo.player));
			std::string text;

			if (skillComponent.getSkillLevel(buttonInfo.skillType) == skillComponent.getMaxLevel(buttonInfo.skillType)) {
				text = "MAXED";
			} else {
				text = "LVL. " + std::to_string(skillComponent.getSkillLevel(buttonInfo.skillType));
			}

			createText(text, { button_pos.x - 40 , button_pos.y + 100 }, 0.5);
		}
	}
	GameStateSystem::instance().setAmbrosia(GameStateSystem::instance().getAmbrosia());
}

void ShopSystem::renderAmbrosia()
{
	//auto ambrosia_icon = ECS::Entity();
	//ShadedMesh& logoResource = cacheResource("ambrosia-icon");
	//if (logoResource.effect.program.resource == 0)
	//{
	//	RenderSystem::createSprite(logoResource, uiPath("ambrosia-icon.png"), "textured");
	//}

	//ambrosia_icon.emplace<ShadedMeshRef>(logoResource);
	//ambrosia_icon.emplace<RenderableComponent>(RenderLayer::MAP_OBJECT);
	//ambrosia_icon.emplace<Motion>().position = vec2(60, 40);
	//ambrosia_icon.get<Motion>().scale = { 0.5, 0.5 };
	//createText(std::to_string(ambrosia), { 110 , 50 }, 0.5);

	GameStateSystem::instance().createAmbrosiaUI();

}

void ShopSystem::updateAmbrosia() {
	ECS::registry<Text>.clear();
	createText(std::to_string(GameStateSystem::instance().getAmbrosia()), { 110 , 50 }, 0.5);
	renderLabels();
}
