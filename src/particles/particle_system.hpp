#pragma once
#include "common.hpp"
#include "entities/tiny_ecs.hpp"

// CPU representation of a particle
struct Particle {
		glm::vec3 pos, speed;
		unsigned char r, g, b, a; // Color
		float size;
		float life; // Remaining life of the particle. if < 0 : dead and unused.
};

//followed this tutorial for most of the initial setup:http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
class particle_system 
{
		
public:
		particle_system();
		void prepRender();
		void drawParticles(const mat3& projection);
		void step(float elapsed_ms);
		void initParticles();

		void createParticles(int numParticles);



private:
		// The VBO containing the 4 vertices of the particles.

		GLuint particleVertexBuffer;
		GLuint particlesCenterPositionAndSizeBuffer;
		GLuint particlesColorBuffer;

		GLuint cameraRightWorldspaceID;
		GLuint cameraUpWorldspaceID;
		GLuint projectionMatrixID;

		GLuint VertexArrayID;

		GLuint programID;

		GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);


		static const GLfloat particleVertexBufferData[];
		static const int MaxParticles = 10000;
		Particle ParticlesContainer[MaxParticles];
		int lastUsedParticle = 0;
		int particlesCount = 0;

		GLfloat particleCenterPositionAndSizeData[MaxParticles * 4];
		GLubyte particleColorData[MaxParticles * 4];

		int FindUnusedParticle();

		void updateGPU();

};


