#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2DArray array_sampler;
uniform vec3 fcolor;
uniform float isDisabled;
uniform float layer;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = texture(array_sampler, vec3(texcoord, layer));

	// procedural dark fade
	if (isDisabled == 1.0)
	{
		color.xyz -= 0.8 * vec3(1.0, 1.0, 1.0);
	}
}
