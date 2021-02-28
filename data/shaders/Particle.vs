#version 330 core

// Input data for verticies,
in vec3 squareVertices;
in vec4 xyzs; // Position of the center of the particule and size of the square
in vec4 color; // Position of the center of the particule and size of the square

// Passed to fragment shader
out vec2 UV;
out vec4 particlecolor;

// Application data
uniform vec3 cameraRightWorldspace; //This is a constant unless we decide to add rotation to our camera
uniform vec3 cameraUpWorldspace; //This is a constant unless we decide to add rotation to our camera

uniform mat3 projection;

void main()
{
	float particleSize = xyzs.w; //Get the size of this particle
	vec3 particleCenterWorldspace = xyzs.xyz;
	
	//Transform the pixels into world location. Not using a passed in transform matrix because that would require calculating one for every singe particle on the cpu and passing that to the GPU. This is easier.
	vec3 vertexPositionWorldspace = particleCenterWorldspace	+ cameraRightWorldspace * squareVertices.x * particleSize	+ cameraUpWorldspace * squareVertices.y * particleSize;

	// Output position of the vertex
	gl_Position = vec4(projection * vertexPositionWorldspace, 1.0f);
	
	// UV TODO pass this in like in the TA tutorial
	UV = squareVertices.xy + vec2(0.5, 0.5);
	particlecolor = color;
}

