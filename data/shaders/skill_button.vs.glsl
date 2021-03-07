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
uniform float isActive;
uniform float isDisabled;

void main()
{
	// default state
	texcoord = in_texcoord;
	vec3 pos = projection * transform * vec3(in_position.xy, 1.0);

	// add active effects if active and not disabled
	if (isActive == 1.0 && isDisabled != 1.0)
	{
		float amplitude = 0.05;
		float frequency = 0.4; 
		mat3 distention;

		float variation = 1 + cos(time*frequency) * amplitude;
		
		distention[0] = vec3(variation, 0.f, 0.f);
		distention[1] = vec3(0.f, variation, 0.f);
		distention[2] = vec3(0.f, 0.f, 1.f);

		pos = projection * transform * distention * vec3(in_position.xy, 1.0);
	}

	// output
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}