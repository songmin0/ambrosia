#include "particle_system.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <iostream>

#include <fstream>



const GLfloat particle_system::particleVertexBufferData[] = {
				-0.5f, -0.5f, 0.0f,
				0.5f, -0.5f, 0.0f,
				-0.5f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.0f,
};

particle_system::particle_system()
{

		for (int i = 0; i < MaxParticles; i++) {
				ParticlesContainer[i].life = -1.0f;
				//ParticlesContainer[i].cameradistance = -1.0f;
		}
		//TODO make particleContainer a heap variable so that we aren't limited to stack size (use a shared or unique pointer for this)

}


//http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
void particle_system::drawParticles(const mat3& projection)
{

		// Use the particle shader
		glUseProgram(programID);

		// Get the uniform ID's
		cameraRightWorldspaceID = glGetUniformLocation(programID, "cameraRightWorldspace");
		cameraUpWorldspaceID = glGetUniformLocation(programID, "cameraUpWorldspace");
		projectionMatrixID = glGetUniformLocation(programID, "projection");

		// Hardcoded the cameraRight and up direction because we are a 2D game and don't allow camera rotation
		glUniform3f(cameraRightWorldspaceID, 1.0f, 0.0f, 0.0f);
		glUniform3f(cameraUpWorldspaceID, 0.0f, 1.0f, 0.0f);

		glUniformMatrix3fv(projectionMatrixID, 1, GL_FALSE, (float*)&projection);

		prepRender();
		//This line make sure that every instance of a particle uses the same 4 verticies
		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		//This make sure that every instance of a particle uses a new center position
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
		//This make sure that every instance of a particle uses a new colour
		glVertexAttribDivisor(2, 1); // color : one per quad -> 1
		// Draw the particules
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, particlesCount);

		//Disable the vertex attribute arrays used for the particle instances
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		glBindVertexArray(0);
}

void particle_system::prepRender() {
		glBindVertexArray(VertexArrayID);
		updateGPU();

		//If we don't explicitly declare these here we already have them from the animated meshes that are rendered before particle in the render function right now but explicitly declaring here as well so it isn't enabled by "accident"
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



		GLenum error = glGetError();
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);
		glVertexAttribPointer(
				0, // attribute. No particular reason for 0, but must match the layout in the shader. Also must match the value used in glVertexAttribDivisor
				3, // size because each triangle has 3 vertices
				GL_FLOAT, // type
				GL_FALSE, // normalized?
				0, // stride. this is default value for this function
				0 // array buffer offset. this is default value for this function
		);
		error = glGetError();
		assert(error == 0);

		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particlesCenterPositionAndSizeBuffer);
		glVertexAttribPointer(
				1, // attribute. No particular reason for 1, but must match the layout in the shader. Also must match the value used in glVertexAttribDivisor
				4, // size: each triangle has an x,y,z location and a size which is a size of 4
				GL_FLOAT, // type
				GL_FALSE, // normalized?
				0, // stride. this is default value for this function
				0 // array buffer offset. this is default value for this function
		);
		error = glGetError();
		assert(error == 0);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer);
		glVertexAttribPointer(
				2, // attribute. No particular reason for 1, but must match the layout in the shader. Also must match the value used in glVertexAttribDivisor
				4, // size: Each triangle has a colour which has values for each rgba which is 4 values.
				GL_UNSIGNED_BYTE, // type
				GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader *** TODO understand why this normalization allows us to use them as floats
				0, // stride. this is default value for this function
				0 // array buffer offset. this is default value for this function
		);
		error = glGetError();
		assert(error == 0);
}


// from http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
void particle_system::updateGPU() {
		//TODO look into the more sophisticated method of updating the GPU buffers
		// Update the buffers that OpenGL uses for rendering.
		// There are much more sophisticated means to stream data from the CPU to the GPU
		// http://www.opengl.org/wiki/Buffer_Object_Streaming

		//These two blocks of code reallocate the buffers to stop the program from having to stop and wait for the previous draw calls to be done with the previous data in the buffer. This allows us to now add our new data to the buffers without waiting.
		glBindBuffer(GL_ARRAY_BUFFER, particlesCenterPositionAndSizeBuffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning
		//This allows us to use the already allocated buffer from the previous line rather than reallocating a new buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLfloat) * 4, particleCenterPositionAndSizeData);

		glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning
		//This allows us to use the already allocated buffer from the previous line rather than reallocating a new buffer
		glBufferSubData(GL_ARRAY_BUFFER, 0, particlesCount * sizeof(GLubyte) * 4, particleColorData);
		assert(glGetError() == 0);
}

void particle_system::step(float elapsed_ms)
{
		//Reference http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
		float elapsed_time_sec = elapsed_ms / 1000.0f;
		int newParticles = (int)(elapsed_time_sec * 80.0f);
		createParticles(newParticles);

		// Simulate all particles
		particlesCount = 0;
		for (int i = 0; i < MaxParticles; i++) {

				//Get a reference to the current particle to work with
				Particle& p = ParticlesContainer[i];

				if (p.life > 0.0f) {

						// Decrease life
						p.life -= elapsed_ms;
						if (p.life > 0.0f) {
								
								p.pos += p.speed * ((float)elapsed_time_sec);

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
						//TODO implement sorting of particles
						//else {
								// Particles that just died will be put at the end of the buffer in SortParticles();
								//p.cameradistance = -1.0f;
						//}

						particlesCount++;

				}
		}

		//TODO sort particles if needed once we add textures to them and they look bad. for now they look fine unsorted but that may change with textures.
}

void particle_system::initParticles()
{
		// Create and compile our GLSL program from the shaders
		programID = LoadShaders("data/shaders/Particle.vs", "data/shaders/Particle.fs");



		//Generate the VAO for this particle system
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//TODO fill in the particles container with default particles Look at the tutorial code
		//memset(ParticlesContainer, 0, sizeof(ParticlesContainer));

		GLenum error = glGetError();
		//billboard_vertex_buffer;
		glGenBuffers(1, &particleVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, particleVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particleVertexBufferData), particleVertexBufferData, GL_STATIC_DRAW);

		error = glGetError();
		// The VBO containing the positions and sizes of the particles
		glGenBuffers(1, &particlesCenterPositionAndSizeBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, particlesCenterPositionAndSizeBuffer);
		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
		// The VBO containing the colors of the particles
		//particles_color_buffer;
		glGenBuffers(1, &particlesColorBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer);
		// Initialize with empty (NULL) buffer : it will be updated later, each frame.
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);




}


void particle_system::createParticles(int numParticles) {


		for (int i = 0; i < numParticles; i++) {
				int particleIndex = FindUnusedParticle();
				ParticlesContainer[particleIndex].life = rand() % 5 *1000 + 10000;   // This particle will live 5 seconds.
				ParticlesContainer[particleIndex].pos = glm::vec3(-640.0f, rand()%200 - 412, 0.0f);
				
				//The main velocity of every particle
				glm::vec3 mainVelocity = glm::vec3(100.0f, 0.0f, 0.0f);
				//Genertate a random velocity so not all particles follow the same direction
				glm::vec3 randomVelocity = glm::vec3(
						rand() % 25,
						rand() % 5,
						0.0f
				);

				ParticlesContainer[particleIndex].speed = mainVelocity + randomVelocity;

				//TODO we might not need this in the final particle system as we will be using textures
				// Generate a random colour for each particle
				ParticlesContainer[particleIndex].r = rand() % 256;
				ParticlesContainer[particleIndex].g = rand() % 256;
				ParticlesContainer[particleIndex].b = rand() % 256;
				ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

				//Generate a random size for each particle
				ParticlesContainer[particleIndex].size = (rand() % 20)  + 10.0f;
		}
}

// Finds a Particle in ParticlesContainer which isn't used yet or has "died".
int particle_system::FindUnusedParticle()
{
		// from http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
		for (int i = lastUsedParticle; i < MaxParticles; i++) {
				if (ParticlesContainer[i].life < 0) {
						lastUsedParticle = i;
						return i;
				}
		}

		for (int i = 0; i < lastUsedParticle; i++) {
				if (ParticlesContainer[i].life < 0) {
						lastUsedParticle = i;
						return i;
				}
		}
		return 0;
}







//TODO we probably already have a function for this in the template code so find and use that instead
//This is from http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
GLuint particle_system::LoadShaders(const char* vertex_file_path, const char* fragment_file_path) {

		// Create the shaders
		GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

		// Read the Vertex Shader code from the file
		std::string VertexShaderCode;
		std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
		if (VertexShaderStream.is_open()) {
				std::stringstream sstr;
				sstr << VertexShaderStream.rdbuf();
				VertexShaderCode = sstr.str();
				VertexShaderStream.close();
		}
		else {
				printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
				getchar();
				return 0;
		}

		// Read the Fragment Shader code from the file
		std::string FragmentShaderCode;
		std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
		if (FragmentShaderStream.is_open()) {
				std::stringstream sstr;
				sstr << FragmentShaderStream.rdbuf();
				FragmentShaderCode = sstr.str();
				FragmentShaderStream.close();
		}

		GLint Result = GL_FALSE;
		int InfoLogLength;

		// Compile Vertex Shader
		printf("Compiling shader : %s\n", vertex_file_path);
		char const* VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
		glCompileShader(VertexShaderID);

		// Check Vertex Shader
		glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
				std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
				printf("%s\n", &VertexShaderErrorMessage[0]);
		}

		// Compile Fragment Shader
		printf("Compiling shader : %s\n", fragment_file_path);
		char const* FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
		glCompileShader(FragmentShaderID);

		// Check Fragment Shader
		glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
		glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
				std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
				glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
				printf("%s\n", &FragmentShaderErrorMessage[0]);
		}

		// Link the program
		printf("Linking program\n");
		GLuint ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);

		// Check the program
		glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
		glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		if (InfoLogLength > 0) {
				std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
				glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
				printf("%s\n", &ProgramErrorMessage[0]);
		}

		glDetachShader(ProgramID, VertexShaderID);
		glDetachShader(ProgramID, FragmentShaderID);

		glDeleteShader(VertexShaderID);
		glDeleteShader(FragmentShaderID);

		return ProgramID;
}