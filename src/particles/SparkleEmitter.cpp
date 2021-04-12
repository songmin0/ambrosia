#include "particle_system.hpp"

SparkleEmitter::SparkleEmitter(int particlesPerSecond) {
	this->particlesPerSecond = particlesPerSecond;
	secSinceLastParticleSpawn = 0;
}


void SparkleEmitter::simulateParticles(float elapsedMs, int numNewParticles)
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

void SparkleEmitter::createParticle(int index)
{
	ParticlesContainer[index].life = rand() % 2 * 1000 + 3000;   // This particle will live at least 3 seconds.
	ParticlesContainer[index].pos = glm::vec3(rand() % 1366 - 683.0f, rand() % 1024 - 512.0f, 0.0f);

	glm::vec3 mainVelocity = glm::vec3(0.5f, 0.5f, 0.0f);

	//Generate a random velocity so not all particles follow the same direction
	glm::vec3 randomVelocity = glm::vec3(
		rand() % 5,
		rand() % 10,
		0.0f
	);

	ParticlesContainer[index].speed = mainVelocity + randomVelocity;
	
	ParticlesContainer[index].r = 1.0;
	ParticlesContainer[index].g = 1.0;
	ParticlesContainer[index].b = 1.0;
	ParticlesContainer[index].a = (rand() % 6) / 10.f + 0.5f; // 0.5-1.0

	//Generate a random size for each particle
	ParticlesContainer[index].size = (rand() % 20) + 15.0f;
}

void SparkleEmitter::initEmitter()
{
	// Create and compile our GLSL program from the shaders
	shaderProgram.loadFromFile("data/shaders/Particle.vs.glsl", "data/shaders/Particle.fs.glsl");
	particleTexture.loadFromFile(objectsPath("sparkle.png"));

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