#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"
#include "rendering/render_components.hpp"

#include <vector>
#include <string>
#include <memory>


class ParticleEmitter;

// CPU representation of a particle
struct Particle {
		glm::vec3 pos, speed; //position and speed of the particle
		float r, g, b, a; // Color of the particle
		float size; //The size of the particle
		float life; // How much remaining time the particle has. if life < 0 then the particle is dead
};



//followed this tutorial for most of the initial setup:http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
class ParticleSystem 
{
		
public:
		ParticleSystem();
		void prepRender();
		void drawParticles(const mat3& projection);
		void step(float elapsed_ms);
		void initParticles();

		void createParticles(int numParticles);

		static const int MaxParticles = 100;
		//All of the emitters
		std::vector<std::shared_ptr<ParticleEmitter>> emitters;



private:
		// The VBO containing the 4 vertices of the particles.
		GLuint particleVertexBuffer;

		GLuint particlesCenterPositionAndSizeBuffer;
		GLuint particlesColorBuffer;

		GLuint cameraRightWorldspaceID;
		GLuint cameraUpWorldspaceID;
		GLuint projectionMatrixID;

		GLuint VertexArrayID;

		Effect shaderProgram;
		Texture particleTexture;




		static const GLfloat particleVertexBufferData[];
		
		Particle ParticlesContainer[MaxParticles];
		int lastUsedParticle = 0;
		int particlesCount = 0;

		GLfloat particleCenterPositionAndSizeData[MaxParticles * 4];
		GLfloat particleColorData[MaxParticles * 4];

		float secSinceLastParticleSpawn;

		int FindUnusedParticle();

		void updateGPU();

};

//This is a base class for all particle emitters
class ParticleEmitter{
public:
		ParticleEmitter();
		virtual void initEmitter()=0;
		virtual void simulateParticles(float elapsedMs, int numNewParticles)=0;
		virtual void createParticle(int index)=0;
		void step(float elapsedMs);
		void drawParticles(GLuint vertexBuffer, GLuint cameraRightWorldspaceID, GLuint cameraUpWorldspaceID,GLuint projectionMatrixID, const mat3& projection);
protected:
		GLuint particlesCenterPositionAndSizeBuffer;
		GLuint particlesColorBuffer;
		GLuint VertexArrayID;
		Effect shaderProgram;
		Texture particleTexture;
		int particlesPerSecond;
		float secSinceLastParticleSpawn;


		Particle ParticlesContainer[ParticleSystem::MaxParticles];
		int lastUsedParticle = 0;
		int particlesCount = 0;

		GLfloat particleCenterPositionAndSizeData[ParticleSystem::MaxParticles * 4];
		GLfloat particleColorData[ParticleSystem::MaxParticles * 4];

private:
		void updateGPU();
		void prepRender(GLuint vertexBuffer);




};

class BasicEmitter : public ParticleEmitter {
public:
		BasicEmitter(int particlesPerSecond);
	 void initEmitter();
	 void simulateParticles(float elapsedMs, int numNewParticles);
		void createParticle(int index);
};


