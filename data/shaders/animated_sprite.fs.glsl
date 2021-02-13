#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform vec3 fcolor;

// Animation data
uniform float frame;
uniform sampler2DArray array_sampler;
uniform float colourShift;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = texture(array_sampler, vec3(texcoord, frame));

	// Red
	if (colourShift == 1.0)
	{
		color.xyz += 0.8 * vec3(1.0, 0.0, 0.0);
	}
	// Green
	if (colourShift == 2.0)
	{
		color.xyz += 0.8 * vec3(0.0, 1.0, 0.0);
	}
	// Blue
	if (colourShift == 3.0)
	{
		color.xyz += 0.8 * vec3(0.0, 0.0, 1.0);
	}
	// Yellow
	if (colourShift == 4.0)
	{
		color.xyz += 0.8 * vec3(1.0, 1.0, 0.0);
	}
}
