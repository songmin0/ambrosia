#include "render.hpp"
#include "render_components.hpp"
#include "text.hpp"

#include "effects/effects.hpp"
#include "entities/tiny_ecs.hpp"
#include "animation/animation_components.hpp"
#include "ai/ai.hpp"
#include "ui/ui_components.hpp"
#include "ui/ui_entities.hpp"
#include "game/camera.hpp"
#include "game/game_state_system.hpp"
#include "maps/map_objects.hpp"

#include <iostream>

void RenderSystem::drawTexturedMesh(ECS::Entity entity, const mat3& projection)
{
	assert(entity.has<Motion>());
	auto& motion = ECS::registry<Motion>.get(entity);
	auto& texmesh = *ECS::registry<ShadedMeshRef>.get(entity).reference_to_cache;
	// Transformation code, see Rendering and Transformation in the template specification for more info
	// Incrementally updates transformation matrix, thus ORDER IS IMPORTANT
	Transform transform;
	if (entity.has<UIComponent>()) {
			transform.translate(motion.renderPosition);
	}
	else {
			auto camera = ECS::registry<CameraComponent>.entities[0];
			auto& cameraComponent = camera.get<CameraComponent>();
			// Multiply camera positon by scroll rate for parallax entities
			if (entity.has<ParallaxComponent>()) {
				auto& parallaxComponent = entity.get<ParallaxComponent>();
				transform.translate(motion.renderPosition - cameraComponent.position * parallaxComponent.scrollRate);
			}
			else {
				transform.translate(motion.renderPosition - cameraComponent.position);
			}
	}
	transform.rotate(motion.renderAngle);

	// Adjust position of map texture so that top left is at { 0.f, 0.f }
	if (entity.has<MapComponent>())
	{
		auto& mapComponent = entity.get<MapComponent>();
		transform.translate(vec2(mapComponent.mapSize.x / 2, mapComponent.mapSize.y / 2));
	}

	// Adjust root position of pathfinding entities
	if (entity.has<PlayerComponent>() || entity.has<AISystem::MobComponent>())
	{
		transform.translate(vec2(0.f, -0.5f));
	}

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc = glGetUniformLocation(texmesh.effect.program, "transform");
	GLint projection_uloc = glGetUniformLocation(texmesh.effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(texmesh.effect.program, "in_texcoord");
	GLint in_color_loc = glGetAttribLocation(texmesh.effect.program, "in_color");

	// Textures
	if (in_texcoord_loc >= 0)
	{
		transform.scale(motion.scale * static_cast<vec2>(texmesh.texture.size));
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(0));
		
		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(sizeof(vec3)));
		
		// Non-animated Layered 2D Texture Arrays
		if ((entity.has<SkillButton>() || entity.has<ToolTip>() || entity.has<UpgradeButton>())
			&& !entity.has<MoveButtonComponent>() && !entity.has<MoveToolTipComponent>())
		{
			// bind texture as 2d array
			GLint arraySamplerLoc = glGetUniformLocation(texmesh.effect.program, "array_sampler");
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D_ARRAY, texmesh.texture.texture_id);
			glUniform1i(arraySamplerLoc, 1);

			// set the layer uniform
			GLint layer_uloc = glGetUniformLocation(texmesh.effect.program, "layer");
			float layer = 0.f; // default layer

			if (entity.has<SkillInfoComponent>())
			{
				layer = playerToFloat(entity.get<SkillInfoComponent>().player);
			}
			glUniform1f(layer_uloc, layer);

			gl_has_errors();
		}
		else // Normal Texture
		{
			// Enabling and binding texture to slot 0
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texmesh.texture.texture_id);
			gl_has_errors();
		}
	}
	// Coloured Meshes
	else if (in_color_loc >= 0)
	{
		transform.scale(texmesh.mesh.original_size * motion.scale);
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), reinterpret_cast<void*>(sizeof(vec3)));
	}
	else
	{
		throw std::runtime_error("This type of entity is not yet supported");
	} 
	gl_has_errors();

	// Set time uniform if it exists
	GLuint time_uloc = glGetUniformLocation(texmesh.effect.program, "time");
	if (time_uloc >= 0)
	{
		glUniform1f(time_uloc, static_cast<float>(glfwGetTime() * 10.0f));
	}

	// Set colour shift if it exists
	if (entity.has<ColourShift>())
	{
		float colour = entity.get<ColourShift>().colour;
		GLuint colourshift_uloc = glGetUniformLocation(texmesh.effect.program, "colourShift");
		glUniform1f(colourshift_uloc, colour);
	}

	if (entity.has<ButtonStateComponent>())
	{
		const auto& component = entity.get<ButtonStateComponent>();
		GLuint isActive_uloc = glGetUniformLocation(texmesh.effect.program, "isActive");
		glUniform1f(isActive_uloc, component.isActive ? 1.f : 0.f);
		GLuint isDisabled_uloc = glGetUniformLocation(texmesh.effect.program, "isDisabled");
		glUniform1f(isDisabled_uloc, component.isDisabled ? 1.f : 0.f);
	}

	if (entity.has<ActiveSkillFX>())
	{
		transform.rotate(glfwGetTime());
	}
	
	// set HP uniform for HP bars
	GLuint percentHP_uloc = glGetUniformLocation(texmesh.effect.program, "percentHP");
	if (percentHP_uloc >= 0)
	{
		if (entity.has<HPBar>()) {
			auto& statsComp = entity.get<HPBar>().statsCompEntity.get<StatsComponent>();

			float maxHP = statsComp.getStatValue(StatType::MAX_HP);
			float hpShield = statsComp.getStatValue(StatType::HP_SHIELD);
			float hp = statsComp.getStatValue(StatType::HP);

			// Using std::max to prevent division by zero
			float maxEffectiveHP = std::max(1.f, maxHP + hpShield);

			float percentHP = hp / maxEffectiveHP;
			float percentShield = hpShield / maxEffectiveHP;

			glUniform1f(percentHP_uloc, percentHP);
			GLuint percentShield_uloc = glGetUniformLocation(texmesh.effect.program, "percentShield");
			glUniform1f(percentShield_uloc, percentShield);

			GLint isMob_uloc = glGetUniformLocation(texmesh.effect.program, "isMob");
			glUniform1i(isMob_uloc, entity.get<HPBar>().isMob);
		}
	}

	if (entity.has<ActiveArrow>())
	{
		GLuint doesBob_uloc = glGetUniformLocation(texmesh.effect.program, "doesBob");
		glUniform1i(doesBob_uloc, true);
	}

	// Uniforms for distendable shader
	GLuint xamplitude_uloc = glGetUniformLocation(texmesh.effect.program, "xamplitude");
	if (xamplitude_uloc >= 0)
	{
		GLuint xfrequency_uloc = glGetUniformLocation(texmesh.effect.program, "xfrequency");
		GLuint yamplitude_uloc = glGetUniformLocation(texmesh.effect.program, "yamplitude");
		GLuint yfrequency_uloc = glGetUniformLocation(texmesh.effect.program, "yfrequency");
		if (entity.has<DistendableComponent>())
		{
			auto& params = entity.get<DistendableComponent>();
			glUniform1f(xamplitude_uloc, params.xamplitude);
			glUniform1f(xfrequency_uloc, params.xfrequency);
			glUniform1f(yamplitude_uloc, params.yamplitude);
			glUniform1f(yfrequency_uloc, params.yfrequency);
		}
		else
		{
			glUniform1f(xamplitude_uloc, 0.f);
			glUniform1f(xfrequency_uloc, 0.f);
			glUniform1f(yamplitude_uloc, 0.f);
			glUniform1f(yfrequency_uloc, 0.f);
		}
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform3fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	//GLsizei num_triangles = num_indices / 3;

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
};


void RenderSystem::drawAnimatedMesh(ECS::Entity entity, const mat3& projection)
{
	assert(entity.has<Motion>() && entity.has<AnimationsComponent>());
	auto& motion = entity.get<Motion>();
	auto& anims = entity.get<AnimationsComponent>();
	auto& texmesh = *anims.referenceToCache;
	Transform transform;
	if (entity.has<UIComponent>()) {
		transform.translate(motion.renderPosition);
	}
	else {
		auto camera = ECS::registry<CameraComponent>.entities[0];
		auto& cameraComponent = camera.get<CameraComponent>();
		// Add skill fx offset to translate
		if (entity.has<SkillFXData>()) {
			auto& fxOffset = entity.get<SkillFXData>().offset;
			transform.translate(motion.renderPosition + fxOffset - cameraComponent.position);
		}
		else {
			transform.translate(motion.renderPosition - cameraComponent.position);
		}
	}
	transform.rotate(motion.renderAngle);
	transform.scale(motion.scale * static_cast<vec2>(texmesh.texture.size));

	// The entity's feet are at the bottom of the texture, so move it upward by half the texture size
	if (entity.has<PlayerComponent>() || entity.has<AISystem::MobComponent>() || entity.has<SkillFXData>())
	{
		transform.translate(vec2(0.f, -0.5f));
	}

	// Setting shaders
	glUseProgram(texmesh.effect.program);
	glBindVertexArray(texmesh.mesh.vao);
	gl_has_errors();

	// Enabling alpha channel for textures
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	gl_has_errors();

	GLint transform_uloc = glGetUniformLocation(texmesh.effect.program, "transform");
	GLint projection_uloc = glGetUniformLocation(texmesh.effect.program, "projection");
	gl_has_errors();

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, texmesh.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, texmesh.mesh.ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(texmesh.effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(texmesh.effect.program, "in_texcoord");
	GLint in_color_loc = glGetAttribLocation(texmesh.effect.program, "in_color");

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(0));

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), reinterpret_cast<void*>(sizeof(vec3))); // note the stride to skip the preceeding vertex position

	// animations have a ShadedMesh with a Texture component that's a 2D Array Texture, not a 2D Texture
	GLint frame_uloc = glGetUniformLocation(texmesh.effect.program, "frame");
	GLint arraySamplerLoc = glGetUniformLocation(texmesh.effect.program, "array_sampler");

	// safety check, although this should never happen 
	// because animation component must be initialized with an animation
	if (anims.anims.empty())
	{
		return;
	}

	// add animation offset
	transform.translate(anims.currAnimData->offset);

	float frame = (float)anims.currAnimData->currFrame;
	glUniform1f(frame_uloc, frame);

	// texture_id stores a 2d array texture instead
	// bind it to slot 1 so we don't confuse with the 2d texture sampler0 (it doesn't reaaally matter though)
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texmesh.texture.texture_id);

	// manually set the sampler...
	glUniform1i(arraySamplerLoc, 1);
	gl_has_errors();

	// Set colour shift if it exists
	if (entity.has<ColourShift>())
	{
		float colour = entity.get<ColourShift>().colour;
		GLuint colourshift_uloc = glGetUniformLocation(texmesh.effect.program, "colourShift");
		glUniform1f(colourshift_uloc, colour);
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(texmesh.effect.program, "fcolor");
	glUniform3fv(color_uloc, 1, (float*)&texmesh.texture.color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float*)&transform.mat);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	glBindVertexArray(0);
};


// Draw the intermediate texture to the screen, with some distortion to simulate water
void RenderSystem::drawToScreen() 
{
	// Setting shaders
	glUseProgram(screen_sprite.effect.program);
	glBindVertexArray(screen_sprite.mesh.vao);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(&window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Disable alpha channel for mapping the screen texture onto the real screen
	glDisable(GL_BLEND); // we have a single texture without transparency. Areas with alpha <1 cab arise around the texture transparency boundary, enabling blending would make them visible.
	glDisable(GL_DEPTH_TEST);

	glBindBuffer(GL_ARRAY_BUFFER, screen_sprite.mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screen_sprite.mesh.ibo); // Note, GL_ELEMENT_ARRAY_BUFFER associates indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();

	// Draw the screen texture on the quad geometry
	gl_has_errors();

	// Draw lights
	int hasLights = GameStateSystem::instance().hasLights() ? 1 : -1;
	GLuint hasLights_uloc = glGetUniformLocation(screen_sprite.effect.program, "hasLights");
	glUniform1i(hasLights_uloc, hasLights);
	
	// Set clock
	GLuint time_uloc       = glGetUniformLocation(screen_sprite.effect.program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(screen_sprite.effect.program, "darken_screen_factor");
	glUniform1f(time_uloc, static_cast<float>(glfwGetTime() * 10.0f));
	auto& screen = ECS::registry<ScreenState>.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();

	// Set the vertex position and vertex texture coordinates (both stored in the same VBO)
	GLint in_position_loc = glGetAttribLocation(screen_sprite.effect.program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)0);
	GLint in_texcoord_loc = glGetAttribLocation(screen_sprite.effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void*)sizeof(vec3)); // note the stride to skip the preceeding vertex position
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, screen_sprite.texture.texture_id);

	// Draw
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr); // two triangles = 6 vertices; nullptr indicates that there is no offset from the bound index buffer
	glBindVertexArray(0);
	gl_has_errors();

	//part->drawParticles(projection_2D);
	
}

struct CompareRenderableEntity
{
	bool operator()(ECS::Entity entity1, ECS::Entity entity2) {
		assert(entity1.has<RenderableComponent>());
		assert(entity2.has<RenderableComponent>());
		auto& renderable1 = entity1.get<RenderableComponent>();
		auto& renderable2 = entity2.get<RenderableComponent>();

		assert(entity1.has<Motion>());
		assert(entity2.has<Motion>());
		auto& motion1 = entity1.get<Motion>();
		auto& motion2 = entity2.get<Motion>();

		// Compare players and mobs by their y-position
		if (renderable1.layer == renderable2.layer && renderable1.layer == RenderLayer::PLAYER_AND_MOB) {
			return motion1.renderPosition.y < motion2.renderPosition.y;
		}

		// Last skill fx applied should render on top
		if (renderable1.layer == renderable2.layer && renderable1.layer == RenderLayer::SKILL) {
			auto& skillFXOrder1 = entity1.get<SkillFXData>().order;
			auto& skillFXOrder2 = entity2.get<SkillFXData>().order;
			return motion1.renderPosition.y + skillFXOrder1 < motion2.renderPosition.y + skillFXOrder2;
		}

		// Skill FXs should render on top of players and mobs
		if ((renderable1.layer == RenderLayer::PLAYER_AND_MOB && renderable2.layer == RenderLayer::SKILL)) {
			return motion1.renderPosition.y - float(RenderLayer::PLAYER_AND_MOB) < motion2.renderPosition.y - float(RenderLayer::SKILL);
		}
		else if ((renderable1.layer == RenderLayer::SKILL && renderable2.layer == RenderLayer::PLAYER_AND_MOB)) {
			return motion1.renderPosition.y - float(RenderLayer::SKILL) < motion2.renderPosition.y - float(RenderLayer::PLAYER_AND_MOB);
		}

		return renderable1.layer > renderable2.layer;
	}
};

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(vec2 window_size_in_game_units)
{
	// Getting size of window
	ivec2 frame_buffer_size; // in pixels
	glfwGetFramebufferSize(&window, &frame_buffer_size.x, &frame_buffer_size.y);

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// Clearing backbuffer
	glViewport(0, 0, frame_buffer_size.x, frame_buffer_size.y);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;
	float right = window_size_in_game_units.x;
	float bottom = window_size_in_game_units.y;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	mat3 projection_2D{ { sx, 0.f, 0.f },{ 0.f, sy, 0.f },{ tx, ty, 1.f } };

	// List of entities to render
	std::vector<ECS::Entity> entities = ECS::registry<RenderableComponent>.entities;
	// Sort the entities depending on their render layer
	std::sort(entities.begin(), entities.end(), CompareRenderableEntity());

	for (ECS::Entity entity : entities)
	{
		if (!entity.has<Motion>())
		{
			continue;
		}
		if (entity.has<VisibilityComponent>())
		{
			if (!entity.get<VisibilityComponent>().isVisible)
			{
				continue;
			}
		}

		// Animated Meshes
		if (entity.has<AnimationsComponent>())
		{ 
			drawAnimatedMesh(entity, projection_2D);
		}
		else // normal textured mesh
		{
			drawTexturedMesh(entity, projection_2D);
		}

		gl_has_errors();
	}

	assert(!ECS::registry<CameraComponent>.entities.empty());
	auto camera = ECS::registry<CameraComponent>.entities[0];
	auto& cameraComponent = camera.get<CameraComponent>();

	// Draw text components to the screen
	// NOTE: for simplicity, text components are drawn in a second pass,
	// on top of all texture mesh components. This should be reasonable
	// for nearly all use cases. If you need text to appear behind meshes,
	// consider using a depth buffer during rendering and adding a
	// Z-component or depth index to all renderable components.
	for (auto entity : ECS::registry<Text>.entities) {
		Text& text = entity.get<Text>();
		// Prevent damage numbers moving with the camera
		if (entity.has<DamageNumberComponent>()) {
			text.offset = -cameraComponent.position;
		}
		drawText(text, window_size_in_game_units);
	}

	particleSystem->drawParticles(projection_2D, cameraComponent.position);
	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(&window);
}

void gl_has_errors()
{
	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return;
	
	const char* error_str = "";
	while (error != GL_NO_ERROR)
	{
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		std::cerr << "OpenGL:" << error_str << std::endl;
		error = glGetError();
	}
	throw std::runtime_error("last OpenGL error:" + std::string(error_str));
}
