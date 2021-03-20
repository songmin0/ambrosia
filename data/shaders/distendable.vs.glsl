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
uniform float orientation;
uniform float xamplitude;
uniform float xfrequency;
uniform float yamplitude;
uniform float yfrequency;

void main()
{
	texcoord = in_texcoord;
	mat3 distention;

	float variationx = 1 + cos(time*xfrequency) * xamplitude;
	float variationy = 1 + cos(time*yfrequency) * yamplitude;
	
	distention[0] = vec3(variationx, 0.f, 0.f);
	distention[1] = vec3(0.f, variationy, 0.f);
	distention[2] = vec3(0.f, 0.f, 1.f);

	vec3 pos = projection * transform * distention * vec3(in_position.xy, 1.0);

	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}