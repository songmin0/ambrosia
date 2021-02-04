#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform vec3 fcolor;

// Animation data
uniform float frame;
uniform sampler2DArray array_sampler;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = texture(array_sampler, vec3(texcoord, frame));
}
