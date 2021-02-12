#pragma once

using namespace glm;

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
