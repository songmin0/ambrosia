#include "common.hpp"
#include "rendering/text.hpp"

// Note, we could also use the functions from GLM but we write the transformations here to show the uderlying math
void Transform::scale(vec2 scale)
{
	mat3 S = { { scale.x, 0.f, 0.f },{ 0.f, scale.y, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * S;
}

void Transform::rotate(float radians)
{
	float c = std::cos(radians);
	float s = std::sin(radians);
	mat3 R = { { c, s, 0.f },{ -s, c, 0.f },{ 0.f, 0.f, 1.f } };
	mat = mat * R;
}

void Transform::translate(vec2 offset)
{
	mat3 T = { { 1.f, 0.f, 0.f },{ 0.f, 1.f, 0.f },{ offset.x, offset.y, 1.f } };
	mat = mat * T;
}

float playerToFloat(const PlayerType& player)
{
	float output = 0.f;
	switch (player) {
	case PlayerType::RAOUL:
		output = 0.f;
		break;
	case PlayerType::TAJI:
		output = 1.f;
		break;
	case PlayerType::CHIA:
		output = 2.f;
		break;
	case PlayerType::EMBER:
		output = 3.f;
		break;
	default:
		break;
	}

	return output;
}

std::string floatToString (float number)
{
	int roundedNum = (int)(number * 100 + .5);
	float fixedNum = (float)roundedNum / 100.f;
	std::string numString = std::to_string(fixedNum);
	numString.erase(numString.find_last_not_of('0') + 1, std::string::npos);
	numString.erase(numString.find_last_not_of('.') + 1, std::string::npos);
	return numString;
};
