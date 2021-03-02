#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float percentHP;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	// make available HP green
	if (texcoord.x <= percentHP)
	{
		color.xyz -= vec3(1.0, 0.0, 1.0); // remove red and blue
		color.xyz += vec3(color.x * 0.0, color. y * 3.0,  color.z * 0.0); // augment green
	}
}
