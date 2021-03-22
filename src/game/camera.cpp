#include "camera.hpp"

ECS::Entity Camera::createCamera(json configValues) {
	return createCamera(vec2(configValues.at("position")[0], configValues.at("position")[1]));
}

ECS::Entity Camera::createCamera(vec2 position) {
	// There should only ever be one of this type of entity
	while(ECS::registry<CameraComponent>.size() > 0)
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<CameraComponent>.entities.back());
	}

	auto entity = ECS::Entity();
	entity.emplace<CameraComponent>().position = position;
	return entity;
}