#pragma once
#include "entities/tiny_ecs.hpp"

using namespace glm;

struct RawMouseClickEvent
{
	vec2 mousePos;
};

struct MouseClickEvent
{
	vec2 mousePos;
};

struct LaunchEvent
{
	ECS::Entity instigator;
	vec2 targetPosition;
};

struct LaunchBulletEvent : public LaunchEvent
{};

struct LaunchBoneEvent : public LaunchEvent
{};

struct HitEvent
{
	ECS::Entity instigator;
	ECS::Entity target;
};
