#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float time;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	float frequency = 0.15;
	float alpha = abs(sin(time * frequency));
	color = vec4(fcolor, alpha) * texture(sampler0, vec2(texcoord.x, texcoord.y));
}
