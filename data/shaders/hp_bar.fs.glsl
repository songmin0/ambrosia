#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float percentHP;
uniform float percentShield;
uniform int isMob;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	// make available, unshielded HP green
	if (texcoord.x <= percentHP)
	{
		if (isMob == 1)
		{
			// red
			color.xyz -= vec3(0.0, 1.0, 1.0);
			color.xyz += vec3(color.x * 3.0, color. y * 0.0,  color.z * 0.0);
		}
		else
		{
			color.xyz -= vec3(1.0, 0.0, 1.0); // remove red and blue
			color.xyz += vec3(color.x * 0.0, color. y * 3.0,  color.z * 0.0); // augment green
		}
	}
	// make shielded HP blue
	else if (texcoord.x <= percentHP + percentShield)
	{
		color.xyz -= vec3(0.3, 0.0, 0.0);
		color.xyz += vec3(color.x * 0.0, color. y * 0.5,  color.z * 3.0);
	}
}
