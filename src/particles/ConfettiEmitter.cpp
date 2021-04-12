#include "particle_system.hpp"

ConfettiEmitter::ConfettiEmitter() {
	this->burst = true;
	secSinceLastParticleSpawn = 0;
}


void ConfettiEmitter::simulateParticles(float elapsedMs, int numNewParticles)
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
				//Apply "Gravity"
				p.speed += vec3(0.0f,50.0f,0.0f) * ((float)elapsedTimeSec);
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

void ConfettiEmitter::createParticle(int index)
{
	ParticlesContainer[index].life = rand() % 10 * 1000 + 60000;   // This particle will live at least 60 seconds.
	if (rand() % 2 == 1) {
		ParticlesContainer[index].pos = glm::vec3(rand() % 200 - 500.0f, 512.0f, 0.0f);
	}
	else {
		ParticlesContainer[index].pos = glm::vec3(rand() % 200 + 300.0f, 512.0f, 0.0f);
	}

	glm::vec3 mainVelocity = glm::vec3(0.0f, -250.0f, 0.0f);

	//Genertate a random velocity so not all particles follow the same direction
	glm::vec3 randomVelocity = glm::vec3(
		rand() % 120 - 60,
		rand() % 250,
		0.0f
	);

	ParticlesContainer[index].speed = mainVelocity + randomVelocity;
	vec4 color;

	switch(rand() % 5){
	case 0: 
	 color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
		break;
	case 1:
		color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		break;
	case 2:
		color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
		break;
	case 3:
		color = vec4(1.0f, 1.0f, 0.0f, 1.0f);
		break;
	case 4:
		color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
		break;
	default:
		color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	ParticlesContainer[index].r = color.r;
	ParticlesContainer[index].g = color.g;
	ParticlesContainer[index].b = color.b;
	ParticlesContainer[index].a = color.a;

	//Generate a random size for each particle
	ParticlesContainer[index].size = (rand() % 15) + 10.0f;
}

void ConfettiEmitter::initEmitter()
{
	// Create and compile our GLSL program from the shaders
	shaderProgram.loadFromFile("data/shaders/Particle.vs.glsl", "data/shaders/Particle.fs.glsl");
	particleTexture.loadFromFile(objectsPath("confetti.png"));

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