#include "debug.hpp"

#include "entities/tiny_ecs.hpp"
#include "rendering/render.hpp"
#include "rendering/render_components.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 2
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
namespace DebugSystem 
{
	void createLine(vec2 position, vec2 scale) {
		auto entity = ECS::Entity();

		std::string key = "thick_line";
		ShadedMesh& resource = cacheResource(key);
		if (resource.effect.program.resource == 0) {
			// create a procedural circle
			constexpr float z = -0.1f;
			vec3 red = { 0.8,0.1,0.1 };

			// Corner points
			ColoredVertex v;
			v.position = {-0.5,-0.5,z};
			v.color = red;
			resource.mesh.vertices.push_back(v);
			v.position = { -0.5,0.5,z };
			v.color = red;
			resource.mesh.vertices.push_back(v);
			v.position = { 0.5,0.5,z };
			v.color = red;
			resource.mesh.vertices.push_back(v);
			v.position = { 0.5,-0.5,z };
			v.color = red;
			resource.mesh.vertices.push_back(v);

			// Two triangles
			resource.mesh.vertex_indices.push_back(0);
			resource.mesh.vertex_indices.push_back(1);
			resource.mesh.vertex_indices.push_back(3);
			resource.mesh.vertex_indices.push_back(1);
			resource.mesh.vertex_indices.push_back(2);
			resource.mesh.vertex_indices.push_back(3);

			RenderSystem::createColoredMesh(resource, "colored_mesh");
		}

		// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
		ECS::registry<ShadedMeshRef>.emplace(entity, resource);
		entity.emplace<RenderableComponent>(RenderLayer::DEBUG);

		// Create motion
		auto& motion = ECS::registry<Motion>.emplace(entity);
		motion.angle = 0.f;
		motion.velocity = { 0, 0 };
		motion.position = position;
		motion.scale = scale;

		ECS::registry<DebugComponent>.emplace(entity);
	}

	void createBox(vec2 position, vec2 size)
	{
		constexpr float LINE_THICKNESS = 5.f;

		const vec2 line_scale_horizontal(size.x, LINE_THICKNESS);
		const vec2 line_scale_vertical(LINE_THICKNESS, size.y);

		const float size_half_x = size.x / 2.f;
		const float size_half_y = size.y / 2.f;

		vec2 left_line_pos = position + vec2(-size_half_x, 0.f);
		createLine(left_line_pos, line_scale_vertical);

		vec2 right_line_pos = position + vec2(size_half_x, 0.f);
		createLine(right_line_pos, line_scale_vertical);

		vec2 top_line_pos = position + vec2(0.f, -size_half_y);
		createLine(top_line_pos, line_scale_horizontal);

		vec2 bottom_line_pos = position + vec2(0.f, size_half_y);
		createLine(bottom_line_pos, line_scale_horizontal);
	}

	void createDottedLine(vec2 position1, vec2 position2)
	{
		const vec2 SIZE(5.f, 5.f);
		constexpr int DOT_SPACING = 20;

		vec2 pos1_to_pos2 = position2 - position1;

		vec2 direction = normalize(pos1_to_pos2);
		float distance = length(pos1_to_pos2);

		int num_dots = (int)floor((distance / DOT_SPACING) + 1.f);

		for (int i = 0; i < num_dots; i++)
		{
			float distance_to_dot = DOT_SPACING * i;
			vec2 dot_position = position1 + (direction * distance_to_dot);

			createLine(dot_position, SIZE);
		}
	}

	void createPath(std::stack<vec2> path)
	{
		if (path.empty())
		{
			return;
		}

		vec2 prevPoint = path.top();
		path.pop();

		while (!path.empty())
		{
			vec2 currPoint = path.top();
			path.pop();

			createDottedLine(prevPoint, currPoint);

			prevPoint = currPoint;
		}
	}

	void clearDebugComponents() {
		// Clear old debugging visualizations
		while (!ECS::registry<DebugComponent>.entities.empty()) {
			ECS::ContainerInterface::removeAllComponentsOf(ECS::registry<DebugComponent>.entities.back());
		}
	}

	bool in_debug_mode = false;
}
