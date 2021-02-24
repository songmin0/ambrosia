#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 squareVertices;
layout(location = 1) in vec4 xyzs; // Position of the center of the particule and size of the square
layout(location = 2) in vec4 color; // Position of the center of the particule and size of the square

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec4 particlecolor;

// Values that stay constant for the whole mesh.
uniform vec3 cameraRightWorldspace;
uniform vec3 cameraUpWorldspace;
//uniform mat4 VP; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)

uniform mat3 projection;

void main()
{
	float particleSize = xyzs.w; // because we encoded it this way.
	vec3 particleCenter_wordspace = xyzs.xyz;
	//vec3 particleCenter_wordspace = vec3(500,500,0);
	
	vec3 vertexPosition_worldspace = 
		particleCenter_wordspace
		+ cameraRightWorldspace * squareVertices.x * particleSize
		+ cameraUpWorldspace * squareVertices.y * particleSize;

	//vec3 vertexPosition_worldspace = 
		//particleCenter_wordspace
		//+ (CameraRight_worldspace * squareVertices.x * 50.0f)
		//+ (CameraUp_worldspace * squareVertices.y * 50.0f);



				//row 1	[0.0015625 0 -1]	



	// Output position of the vertex
	//gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);
	gl_Position = vec4(projection * vertexPosition_worldspace, 1.0f);
	
	//TODO check if in -1 to 1 range
	if(gl_Position.y < 1.0f){
		//gl_Position = vec4(squareVertices,1.0f);
	}


	if(projection[0][0] == 0.0f){
		//gl_Position = vec4(squareVertices,1.0f);
	}

	//if(CameraRight_worldspace.x == 1.0f){
		//gl_Position = vec4(squareVertices,1.0f);
	//}

	//gl_Position = vec4(squareVertices,1.0f);
 //gl_Position.w = 1.0;

	// UV of the vertex. No special space for this one.
	UV = squareVertices.xy + vec2(0.5, 0.5);
	particlecolor = color;
}

