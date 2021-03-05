#include "camera.hpp"

ECS::Entity Camera::createCamera(json configValues) {
	return createCamera(vec2(configValues.at("position")[0], configValues.at("position")[1]));
}

ECS::Entity Camera::createCamera(vec2 position) {
	auto entity = ECS::Entity();
	CameraComponent& cameraComponent = entity.emplace<CameraComponent>();
	cameraComponent.position = position;
	return entity;
}