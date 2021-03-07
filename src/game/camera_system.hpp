#pragma once
#include "entities/tiny_ecs.hpp"
#include "game/common.hpp"
#include "game/events.hpp"
#include "game/event_system.hpp"

class CameraSystem
{
public:
	CameraSystem(vec2 window_size_in_px);
	~CameraSystem();

	void step(float elapsed_ms);

	static void moveCamera(vec2 distance, vec2 window_size_in_px);
	static void viewPosition(vec2 position, vec2 window_size_in_px);
	static bool isPositionInView(vec2 position, vec2 window_size_in_px);

private:
	vec2 window_size_in_px;

	EventListenerInfo playerChangeListener;
	void onPlayerChange(const PlayerChangeEvent& event);

	static void preventViewingOutOfBounds(ECS::Entity camera, vec2 window_size_in_px);
};