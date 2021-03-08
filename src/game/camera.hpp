#pragma once
#include "tiny_ecs.hpp"
#include "game/common.hpp"

struct Camera
{
	static ECS::Entity createCamera(json configValues);
	static ECS::Entity createCamera(vec2 position);
};

struct CameraComponent {
	vec2 position = { 0, 0 };
	vec2 velocity = { 0, 0 };
	float speed = 400.f;
};

struct CameraDelayedMoveComponent {
	CameraDelayedMoveComponent(float delay) : delay{ delay } {}
	float delay = 0.f;
	vec2 position = { 0, 0 };
};