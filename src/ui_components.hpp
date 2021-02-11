#pragma once
#include "common.hpp"

struct ClickableCircleComponent
{
	vec2 position = { 0, 0 };
	float radius = 0;
	void (*callback)();
	ClickableCircleComponent(vec2 position, float radius, void (*callback)());
};

struct ClickableRectangleComponent
{
	vec2 position = { 0, 0 };
	float width = 0;
	float height = 0;
	void (*callback)();
	ClickableRectangleComponent(vec2 position, float width, float height, void (*callback)());
};
