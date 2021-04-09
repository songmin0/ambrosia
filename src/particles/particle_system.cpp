#include "particle_system.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

#include <fstream>



const GLfloat ParticleSystem::particleVertexBufferData[] = {
				-0.5f, -0.5f, 0.0f,
				0.5f, -0.5f, 0.0f,
				-0.5f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.0f,
};
//TODO delete the emitter logic from the particle system so that the only way to have particles is through an emitter.
ParticleSystem::ParticleSystem()
{
		//Initialize all the member variables to stop the compiler from being upset. They are all properly initalized in the initParticles function which is called after all the GL dependencies are done being called.
		particleVertexBuffer = 0;
		particlesCenterPositionAndSizeBuffer = 0;
		particlesColorBuffer = 0;
		cameraRightWorldspaceID = 0;
		cameraUpWorldspaceID = 0;
		projectionMatrixID = 0;
		VertexArrayID = 0;
		//Initialize the arrays to 0 to stop the compiler from being upset.
		memset(particleCenterPositionAndSizeData, 0, sizeof(GLfloat) * MaxParticles * 4);
		memset(particleColorData, 0, sizeof(GLfloat) * MaxParticles * 4);

		secSinceLastParticleSpawn = 0.0f;

		for (int i = 0; i < MaxParticles; i++) {
				ParticlesContainer[i].life = -1.0f;
		}
		//TODO make particleContainer a heap variable so that we aren't limited to stack size (use a shared or unique pointer for this)

		addEmitterListener = EventSystem<AddEmitterEvent>::instance().registerListener(
			std::bind(&ParticleSystem::onAddedEmitterEvent, this, std::placeholders::_1));

		deleteEmitterListener = EventSystem<DeleteEmitterEvent>::instance().registerListener(
			std::bind(&ParticleSystem::onDeleteEmitterEvent, this, std::placeholders::_1));

		deleteAllEmittersListener = EventSystem<DeleteAllEmittersEvent>::instance().registerListener(
			std::bind(&ParticleSystem::onDeleteAllEmitterEvent, this, std::placeholders::_1));

}


//http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
void ParticleSystem::drawParticles(const mat3& projection, const vec2& cameraPos)
{
		for (std::map<std::string, std::shared_ptr<ParticleEmitter>>::iterator it = newEmitters.begin(); it != newEmitters.end(); ++it){
			it->second->drawParticles(particleVertexBuffer, cameraRightWorldspaceID, cameraUpWorldspaceID, projectionMatrixID, projection, cameraPos);
		}
}

void ParticleSystem::onAddedEmitterEvent(const AddEmitterEvent& event)
{
	newEmitters.emplace(event.label, event.emitter);
	event.emitter->initEmitter();
}

void ParticleSystem::onDeleteEmitterEvent(const DeleteEmitterEvent& event) {
	newEmitters.erase(event.label);
}

void ParticleSystem::onDeleteAllEmitterEvent(const DeleteAllEmittersEvent& event) {
	newEmitters.erase(newEmitters.begin(), newEmitters.end());
}

void ParticleSystem::step(float elapsed_ms)
{
	//Call the step function for each emitter in the world NOTE this does nothing right now because emitters haven't been fully built
	for (std::map<std::string, std::shared_ptr<ParticleEmitter>>::iterator it = newEmitters.begin(); it != newEmitters.end(); ++it) {
		it->second->step(elapsed_ms);
	}
	
}

void ParticleSystem::initParticles()
{
	
		GLenum error = glGetError();
		//generate the vertex buffer for the particles. This should be used for all particle emitters
		glGenBuffers(1, &particleVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertexBufferData), particleVertexBufferData, GL_STATIC_DRAW);
}

// ---------- ParticleEmitter base class -----------
ParticleEmitter::ParticleEmitter()
{
		for (int i = 0; i < ParticleSystem::MaxParticles; i++) {
				ParticlesContainer[i].life = -1.0f;
		}
}

void ParticleEmitter::step(float elapsedMs)
{
	int newParticles;
	if (burst) {
		newParticles = ParticleSystem::MaxParticles;
		burst = false;
	}
	else {
		float elapsed_time_sec = elapsedMs / 1000.0f;
		secSinceLastParticleSpawn += elapsed_time_sec;
		newParticles = (int)(secSinceLastParticleSpawn * particlesPerSecond);
		if (newParticles != 0) {
			secSinceLastParticleSpawn = 0.0f;
		}
	}

		this->simulateParticles(elapsedMs, newParticles);
}

void ParticleEmitter::drawParticles(GLuint vertexBuffer, GLuint cameraRightWorldspaceID, GLuint cameraUpWorldspaceID, GLuint projectionMatrixID, const mat3& projection, const vec2& cameraPos)
{

		// Use the particle shader
		glUseProgram(shaderProgram.program);

		// Get the uniform ID's
		cameraRightWorldspaceID = glGetUniformLocation(shaderProgram.program, "cameraRightWorldspace");
		cameraUpWorldspaceID = glGetUniformLocation(shaderProgram.program, "cameraUpWorldspace");
		projectionMatrixID = glGetUniformLocation(shaderProgram.program, "projection");
		cameraPosID = glGetUniformLocation(shaderProgram.program, "cameraPos");

		// Hardcoded the cameraRight and up direction because we are a 2D game and don't allow camera rotation
		glUniform3f(cameraRightWorldspaceID, 1.0f, 0.0f, 0.0f);
		glUniform3f(cameraUpWorldspaceID, 0.0f, 1.0f, 0.0f);

		glUniformMatrix3fv(projectionMatrixID, 1, GL_FALSE, (float*)&projection);
		glUniform2f(cameraPosID, cameraPos.x, cameraPos.y);

		prepRender(vertexBuffer);
		//This line make sure that every instance of a particle uses the same 4 verticies
		glVertexAttribDivisor(0, 0); // particles vertices : All particles use the same 4 verticies.  The first digit must match the vertex index in the prepRender function
		//This make sure that every instance of a particle uses a new center position
		glVertexAttribDivisor(1, 1); // positions : one per particle. The first digit must match the position index in the prepRender function
		//This make sure that every instance of a particle uses a new colour
		glVertexAttribDivisor(2, 1); // color : one per particle.  The first digit must match the color index in the prepRender function

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, particleTexture.texture_id);

		// Draw the particules
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particlesCount);

		//Disable the vertex attribute arrays used for the particle instances
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glBindVertexArray(0);
}

void ParticleEmitter::updateGPU()
{
		auto error = glGetError();
		//These two blocks of code reallocate the buffers to stop the program from having to stop and wait for the previous draw calls to be done with the previous data in the buffer. This allows us to now add our new data to the buffers without waiting.
		glBindBuffer(GL_ARRAY_BUFFER, particlesCenterPositionAndSizeBuffer);
		glBufferData(GL_ARRAY_BUFFER, ParticleSystem::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning
		//This allows us to use the already allocated buffer from the previous line rather than reallocating a new buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLfloat) * 4, particleCenterPositionAndSizeData);
		error = glGetError();
		glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer);
		glBufferData(GL_ARRAY_BUFFER, ParticleSystem::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning
		//This allows us to use the already allocated buffer from the previous line rather than reallocating a new buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLfloat) * 4, particleColorData);
		error = glGetError();
		assert(error == 0);
}

void ParticleEmitter::prepRender(GLuint vertexBuffer)
{
		glBindVertexArray(VertexArrayID);
		updateGPU();

		//If we don't explicitly declare these here we already have them from the animated meshes that are rendered before particle in the render function right now but explicitly declaring here as well so it isn't enabled by "accident"
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



		GLenum error = glGetError();
		//particle verticies
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
				0, // index. Must match the shader in variable number, also must match the value used in glVertexAttribDivisor
				3, // size because each triangle has 3 vertices
				GL_FLOAT, // type
				GL_FALSE, // normalized
				0, // stride. this is default value for this function
				0 // array buffer offset. this is default value for this function
		);
		error = glGetError();
		assert(error == 0);

		// particles centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particlesCenterPositionAndSizeBuffer);
		glVertexAttribPointer(
				1, // index. Must match the shader in variable number, also must match the value used in glVertexAttribDivisor
				4, // size: each triangle has an x,y,z location and a size which is a size of 4
				GL_FLOAT, // type
				GL_FALSE, // normalized
				0, // stride. this is default value for this function
				0 // array buffer offset. this is default value for this function
		);
		error = glGetError();
		assert(error == 0);

		// particle colours
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer);
		glVertexAttribPointer(
				2, // index. Must match the shader in variable number, also must match the value used in glVertexAttribDivisor
				4, // size: Each triangle has a colour which has values for each rgba which is 4 values.
				GL_FLOAT, // type
				GL_FALSE, // normalized
				0, // stride. this is default value for this function
				0 // array buffer offset. this is default value for this function
		);
		error = glGetError();
		assert(error == 0);
}





//Everything below here is for emitters

BasicEmitter::BasicEmitter(int particlesPerSecond){
		this->particlesPerSecond = particlesPerSecond;
		secSinceLastParticleSpawn = 0;
}


void BasicEmitter::simulateParticles(float elapsedMs, int numNewParticles)
{
		float elapsedTimeSec = elapsedMs / 1000.0f;
		particlesCount = 0;
		for (int i = 0; i < ParticleSystem::MaxParticles; i++) {

				//Get a reference to the current particle to work with
				Particle& p = ParticlesContainer[i];

				if (p.life > 0.0f) {
						// Decrease life
						p.life -= elapsedMs;
						if (p.life > 0.0f) {

								p.pos += p.speed * ((float)elapsedTimeSec);

								// Fill the GPU buffer
								particleCenterPositionAndSizeData[4 * particlesCount + 0] = p.pos.x;
								particleCenterPositionAndSizeData[4 * particlesCount + 1] = p.pos.y;
								particleCenterPositionAndSizeData[4 * particlesCount + 2] = p.pos.z;

								particleCenterPositionAndSizeData[4 * particlesCount + 3] = p.size;

								particleColorData[4 * particlesCount + 0] = p.r;
								particleColorData[4 * particlesCount + 1] = p.g;
								particleColorData[4 * particlesCount + 2] = p.b;
								particleColorData[4 * particlesCount + 3] = p.a;

						}
						
						particlesCount++;
				}
				//Spawn new particles if there is an empty index and we need to spawn a new particle
				else if (numNewParticles > 0) {
						createParticle(i);
						numNewParticles--;
				}
		}
}

void BasicEmitter::createParticle(int index)
{
		ParticlesContainer[index].life = rand() % 10 * 1000 + 1200000;   // This particle will live at least 120 seconds.
		ParticlesContainer[index].pos = glm::vec3(rand() % 3840, -512.0f, 0.0f);

		glm::vec3 mainVelocity = glm::vec3(0.5f, 10.0f, 0.0f);
		//Genertate a random velocity so not all particles follow the same direction
		glm::vec3 randomVelocity = glm::vec3(
				rand() % 25,
				rand() % 5,
				0.0f
		);

		ParticlesContainer[index].speed = mainVelocity + randomVelocity;

		ParticlesContainer[index].r = 1.0;
		ParticlesContainer[index].g = 1.0;
		ParticlesContainer[index].b = 1.0;
		ParticlesContainer[index].a = 1.0;

		//Generate a random size for each particle
		ParticlesContainer[index].size = (rand() % 20) + 20.0f;

}

void BasicEmitter::initEmitter()
{
		// Create and compile our GLSL program from the shaders
		shaderProgram.loadFromFile("data/shaders/Particle.vs.glsl", "data/shaders/Particle.fs.glsl");
		particleTexture.loadFromFile(objectsPath("candy-fluff-pink.png"));

		//Generate the VAO for this particle system
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);


		GLenum error = glGetError();

		error = glGetError();
		// The VBO containing the positions and sizes of the particles
		glGenBuffers(1, &particlesCenterPositionAndSizeBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, particlesCenterPositionAndSizeBuffer);
		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBufferData(GL_ARRAY_BUFFER, ParticleSystem::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
		// The VBO containing the colors of the particles
		//particles_color_buffer;
		glGenBuffers(1, &particlesColorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer);
		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBufferData(GL_ARRAY_BUFFER, ParticleSystem::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
}




BlueCottonCandyEmitter::BlueCottonCandyEmitter(int particlesPerSecond) {
	this->particlesPerSecond = particlesPerSecond;
	secSinceLastParticleSpawn = 0;
}


void BlueCottonCandyEmitter::simulateParticles(float elapsedMs, int numNewParticles)
{
	float elapsedTimeSec = elapsedMs / 1000.0f;
	particlesCount = 0;
	for (int i = 0; i < ParticleSystem::MaxParticles; i++) {

		//Get a reference to the current particle to work with
		Particle& p = ParticlesContainer[i];

		if (p.life > 0.0f) {
			// Decrease life
			p.life -= elapsedMs;
			if (p.life > 0.0f) {

				p.pos += p.speed * ((float)elapsedTimeSec);

				// Fill the GPU buffer
				particleCenterPositionAndSizeData[4 * particlesCount + 0] = p.pos.x;
				particleCenterPositionAndSizeData[4 * particlesCount + 1] = p.pos.y;
				particleCenterPositionAndSizeData[4 * particlesCount + 2] = p.pos.z;

				particleCenterPositionAndSizeData[4 * particlesCount + 3] = p.size;

				particleColorData[4 * particlesCount + 0] = p.r;
				particleColorData[4 * particlesCount + 1] = p.g;
				particleColorData[4 * particlesCount + 2] = p.b;
				particleColorData[4 * particlesCount + 3] = p.a;

			}

			particlesCount++;
		}
		//Spawn new particles if there is an empty index and we need to spawn a new particle
		else if (numNewParticles > 0) {
			createParticle(i);
			numNewParticles--;
		}
	}
}

void BlueCottonCandyEmitter::createParticle(int index)
{
	ParticlesContainer[index].life = rand() % 10 * 1000 + 1200000;   // This particle will live at least 120 seconds.
	ParticlesContainer[index].pos = glm::vec3(rand() % 3840 , -512.0f, 0.0f);

	glm::vec3 mainVelocity = glm::vec3(0.5f, 10.0f, 0.0f);
	//Genertate a random velocity so not all particles follow the same direction
	glm::vec3 randomVelocity = glm::vec3(
		rand() % 25,
		rand() % 5,
		0.0f
	);

	ParticlesContainer[index].speed = mainVelocity + randomVelocity;

	ParticlesContainer[index].r = 1.0;
	ParticlesContainer[index].g = 1.0;
	ParticlesContainer[index].b = 1.0;
	ParticlesContainer[index].a = 1.0;

	//Generate a random size for each particle
	ParticlesContainer[index].size = (rand() % 20) + 20.0f;

}

void BlueCottonCandyEmitter::initEmitter()
{
	// Create and compile our GLSL program from the shaders
	shaderProgram.loadFromFile("data/shaders/Particle.vs.glsl", "data/shaders/Particle.fs.glsl");
	particleTexture.loadFromFile(objectsPath("candy-fluff-blue.png"));

	//Generate the VAO for this particle system
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);


	GLenum error = glGetError();

	error = glGetError();
	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particlesCenterPositionAndSizeBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, particlesCenterPositionAndSizeBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, ParticleSystem::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	// The VBO containing the colors of the particles
	//particles_color_buffer;
	glGenBuffers(1, &particlesColorBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, ParticleSystem::MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
}