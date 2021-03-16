#include "camera_system.hpp"
#include "camera.hpp"
#include "maps/map.hpp"
#include "game/game_state_system.hpp"

CameraSystem::CameraSystem(vec2 window_size_in_px) {
	this->window_size_in_px = window_size_in_px;

	playerChangeListener = EventSystem<PlayerChangeEvent>::instance().registerListener(
		std::bind(&CameraSystem::onPlayerChange, this, std::placeholders::_1));
}

CameraSystem::~CameraSystem() {
	if (playerChangeListener.isValid()) {
		EventSystem<PlayerChangeEvent>::instance().unregisterListener(playerChangeListener);
	}
}

// Move camera entity
void CameraSystem::step(float elapsed_ms) {
	if (!GameStateSystem::instance().inGameState()) {
		return;
	}
	for (auto camera : ECS::registry<CameraComponent>.entities) {
		auto& cameraComponent = camera.get<CameraComponent>();

		float step_seconds = 1.0f * (elapsed_ms / 1000.f);

		// Handle delayed camera move
		if (camera.has<CameraDelayedMoveComponent>()) {
			auto& cameraDelayedMoveComponent = camera.get<CameraDelayedMoveComponent>();
			cameraDelayedMoveComponent.delay -= step_seconds;
			if (cameraDelayedMoveComponent.delay <= 0) {
				viewPosition(cameraDelayedMoveComponent.position, window_size_in_px);
				camera.remove<CameraDelayedMoveComponent>();
			}
		}

		cameraComponent.position += step_seconds * cameraComponent.velocity;
		preventViewingOutOfBounds(camera, window_size_in_px);
	}
}

// Move camera the given distance wihout moving out of map texture
void CameraSystem::moveCamera(vec2 distance, vec2 window_size_in_px) {
	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraComponent = camera.get<CameraComponent>();

	cameraComponent.position += distance;
	preventViewingOutOfBounds(camera, window_size_in_px);
}

// Move camera to view given position wihout moving out of map texture
void CameraSystem::viewPosition(vec2 position, vec2 window_size_in_px) {
	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraComponent = camera.get<CameraComponent>();

	cameraComponent.position = { position.x - window_size_in_px.x / 2, position.y - window_size_in_px.y / 2 };
	preventViewingOutOfBounds(camera, window_size_in_px);
}

// Returns if given position is within view of the camera
bool CameraSystem::isPositionInView(vec2 position, vec2 window_size_in_px) {
	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraComponent = camera.get<CameraComponent>();

	// Position is out of view to the top
	if (position.y < cameraComponent.position.y) {
		return false;
	}
	// Position is out of view to the bottom
	else if (position.y > cameraComponent.position.y + window_size_in_px.y) {
		return false;
	}
	// Position is out of view to the left
	else if (position.x < cameraComponent.position.x) {
		return false;
	}
	// Position is out of view to the right
	else if (position.x > cameraComponent.position.x + window_size_in_px.x) {
		return false;
	}

	return true;
}

// Move camera to view active entity
void CameraSystem::onPlayerChange(const PlayerChangeEvent& event) {
	auto entity = event.newActiveEntity;
	auto& motion = entity.get<Motion>();
	// If camera has a delay component, set the position to move to after the delay
	if (!ECS::registry<CameraDelayedMoveComponent>.entities.empty()) {
		auto camera = ECS::registry<CameraDelayedMoveComponent>.entities[0];
		auto& cameraDelayedMoveComponent = camera.get<CameraDelayedMoveComponent>();
		cameraDelayedMoveComponent.position = motion.position;
	}
	else {
		viewPosition(motion.position, window_size_in_px);
	}
}

// Prevent camera from moving out of map texture
void CameraSystem::preventViewingOutOfBounds(ECS::Entity camera, vec2 window_size_in_px) {
	assert(!ECS::registry<MapComponent>.entities.empty());
	vec2 mapSize = ECS::registry<MapComponent>.entities[0].get<MapComponent>().mapSize;
	auto& cameraComponent = camera.get<CameraComponent>();
	// Prevent moving camera out of top
	if (cameraComponent.position.y <= 0) {
		cameraComponent.position.y = 0;
	}
	// Prevent moving camera out of bottom
	else if (cameraComponent.position.y >= mapSize.y - window_size_in_px.y) {
		cameraComponent.position.y = mapSize.y - window_size_in_px.y;
	}
	// Prevent moving camera out of left
	if (cameraComponent.position.x <= 0) {
		cameraComponent.position.x = 0;
	}
	// Prevent moving camera out of right
	else if (cameraComponent.position.x >= mapSize.x - window_size_in_px.x) {
		cameraComponent.position.x = mapSize.x - window_size_in_px.x;
	}
}