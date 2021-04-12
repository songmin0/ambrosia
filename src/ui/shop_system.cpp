#include <shop_system.hpp>
#include <ui/button.hpp>
#include <ui/ui_components.hpp>
#include <ui/ui_entities.hpp>
#include <effects/effects.hpp>
#include <rendering/text.hpp>
#include <game/game_state_system.hpp>
#include <cctype>

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

void ShopSystem::printDescriptionText(std::string text) {
	auto str_len = text.length();
	int line_width = 20;
	// hacky text wrapping
	int firstUnprintedChar = 0;
	int y_pos = 300;

	for (int i = line_width; i < str_len; i += line_width) {
		while (!isspace(text.at(i))) {
			i--;
		}

		if (isspace(text.at(i))) {
			createText(text.substr(firstUnprintedChar, i - firstUnprintedChar + 1), { 1000, y_pos+= 50 }, 0.5);
			firstUnprintedChar = i + 1;
		}
	}

	createText(text.substr(firstUnprintedChar, str_len - firstUnprintedChar + 1), { 1000, y_pos += 50 }, 0.5);

	// cost
	createText("Cost: " + std::to_string(ShopSystem::instance().getCost()), { 1000, y_pos += 50 }, 0.5);
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
	printDescriptionText(descriptions[selected]);

	// render labels
	renderLabels();
}

int ShopSystem::getCost() {
	if (selected_skill == SkillType::NONE) {
		StatsComponent stats = ECS::registry<StatsComponent>.get(getPlayerEntity(selected_player));
		return (int)stats.getStatValue(StatType::LEVEL) * cost_multiplier;
	} else {
		SkillComponent component = ECS::registry<SkillComponent>.get(getPlayerEntity(selected_player));
		return (int)component.getSkillLevel(selected_skill) * cost_multiplier;
	}
}

bool ShopSystem::checkIfAbleToBuy(int level) {

	auto ambrosia = GameStateSystem::instance().getAmbrosia();
	if (ambrosia >= ShopSystem::instance().getCost()) {
		ambrosia -= ShopSystem::instance().getCost();
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

	std::cout << "Buying " << getPlayerName(selected_player) << "'s skill " << (int)selected_skill << std::endl;

	if (selected_skill == SkillType::NONE) {
		StatsComponent stats = ECS::registry<StatsComponent>.get(getPlayerEntity(selected_player));
		if (!stats.atMaxLevel()) {
			if (checkIfAbleToBuy(stats.getStatValue(StatType::LEVEL))) {
				ECS::registry<StatsComponent>.get(getPlayerEntity(selected_player)).levelUp();
			};
		}
		else {
			std::cout << "player level maxed already!" << std::endl;
			return;
		}
	}
	else {
		SkillComponent component = ECS::registry<SkillComponent>.get(getPlayerEntity(selected_player));
		if (component.getSkillLevel(selected_skill) == component.getMaxLevel(selected_skill)) {
			std::cout << "level maxed already!" << std::endl;
			return;
		}

		if (checkIfAbleToBuy(component.getSkillLevel(selected_skill))) {
			ECS::registry<SkillComponent>.get(getPlayerEntity(selected_player)).upgradeSkillLevel(selected_skill);
		}
	}

	ECS::registry<Text>.clear();
	printDescriptionText(descriptions[selected]);

	// render labels
	renderLabels();
}

void ShopSystem::initialize(ECS::Entity raoul, ECS::Entity chia, ECS::Entity ember, ECS::Entity taji)
{
	this->raoul = raoul;
	this->chia = chia;
	this->ember = ember;
	this->taji = taji;

	buttons.clear();
	selected = -1;

	drawButtons();
	renderAmbrosia();
	renderLabels();
}

void ShopSystem::drawButtons() {


	int starting_x = 200;
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
	for (int i = 0; i < buttons.size(); i++) {
		auto e = buttons.at(i);
		vec2 button_pos = ECS::registry<Motion>.get(e).position;
		ECS::Entity player;
		std::string text;
		if (!ECS::registry<SkillInfoComponent>.has(e)) {
			// player upgrades
			switch ((i) / 4) {
			case 0:
				player = raoul;
				break;
			case 1:
				player = taji;
				break;
			case 2:
				player = chia;
				break;
			case 3:
				player = ember;
				break;
			}

			auto stats = player.get<StatsComponent>();
			if (stats.atMaxLevel()) {
				text = "MAX!";
			}
			else {
				text = "LVL. " + std::to_string((int)stats.getStatValue(StatType::LEVEL));
			}
		} else {
			auto buttonInfo = ECS::registry<SkillInfoComponent>.get(e);
			auto skillComponent = ECS::registry<SkillComponent>.get(getPlayerEntity(buttonInfo.player));

			if (skillComponent.getSkillLevel(buttonInfo.skillType) == skillComponent.getMaxLevel(buttonInfo.skillType)) {
				text = "MAX!";
			}
			else {
				text = "LVL. " + std::to_string(skillComponent.getSkillLevel(buttonInfo.skillType));
			}
		}
		createText(text, { button_pos.x - 40 , button_pos.y + 100 }, 0.5);
	}
	GameStateSystem::instance().setAmbrosia(GameStateSystem::instance().getAmbrosia());
}

void ShopSystem::renderAmbrosia()
{
	GameStateSystem::instance().createAmbrosiaUI();
}