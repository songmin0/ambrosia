#include "camera.hpp"

ECS::Entity Camera::createCamera(json configValues) {
	return createCamera(vec2(configValues.at("position")[0], configValues.at("position")[1]));
}

ECS::Entity Camera::createCamera(vec2 position) {
	// There should only ever be one of this type of entity
	while (!ECS::ComponentContainer<Camera>().entities.empty())
	{
		ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<Camera>.entities.back());
	}

	auto entity = ECS::Entity();
	entity.emplace<CameraComponent>().position = position;
	return entity;
}