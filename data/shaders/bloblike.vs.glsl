#version 330 

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 projection;
uniform float time;

void main()
{
	texcoord = in_texcoord;

	float amplitude = 0.5;
	float frequency = 0.5;
	float variation = 1.0 + cos(time*frequency) * amplitude;
	mat3 distention;
	distention[0] = vec3(variation, 0.f, 0.f);
	distention[1] = vec3(0.f, variation, 0.f);
	distention[2] = vec3(0.f, 0.f, 1.f);
	//{ { variation, 0.f, 0.f },{ 0.f, variation, 0.f },{ 0.f, 0.f, 1.f } };

	vec3 pos = projection * transform * distention * vec3(in_position.xy, 1.0);

	//vec3 pos = projection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}