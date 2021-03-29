#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float darken_screen_factor;
uniform int hasLights;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec2 distort(vec2 uv) 
{
	vec2 coord = uv.xy;
    return coord;
}

vec4 color_shift(vec4 in_color) 
{
	vec4 color = in_color;

	if (hasLights == 1)
	{
		color -= vec4(0.25, 0.3, 0.1, 0);
	}

	return color;
}

vec4 fade_color(vec4 in_color) 
{
	vec4 color = in_color;
	if (darken_screen_factor > 0)
	{
		color -= darken_screen_factor * vec4(1.0, 1.0, 1.0, 0);
	}

	return color;
}

vec4 add_lights(vec4 in_color, vec2 uv)
{
	// all calculations are in uv coords
	vec4 color = in_color;

	if (hasLights != 1)
	{
		return color;
	}

	float radius = 0.1; // the width/spread of the spotlight

	// the x position of the spotlights at the top of the screen
	//float lightposX = 0.5;
	float lightsposX[3] = float[](0.75, 0.2, 0.5);

	// calculate each light
	for(int i = 0 ; i < lightsposX.length() ; ++i)
	{	
		// generate pseudo-random offsets over time based on index
		float amplitude = 0.02 + 0.02 * float(i);
		float frequency = 0.05 + 0.05 * float(i);
		float offset = sin(time * frequency) * amplitude;

		// the spread increases as we go down the screen
		float threshold = radius * (1.0 - uv.y) + 0.1;
		threshold += offset;

		// offset light direction based on index for pseudo-random effect
		if (mod(i, 2) != 0)
		{
			offset *= -1.0;
		}

  		float lightposX = lightsposX[i] + offset;

		// light up coords within the threshold
		if (uv.x > (lightposX - threshold) && uv.x < (lightposX + threshold))
		{
			// apply soft edges based on how far the coord is from the light
			float softenFactor = abs(uv.x - lightposX);

			// normalize that factor from 0 to 1 - the maximum softening is when the coord is at threshold
			softenFactor = softenFactor / threshold;

			float brightnessOffset = cos(time * frequency) * amplitude;

			// soft light
			color += (1.0 - softenFactor) * (1 + brightnessOffset) * vec4(0.2, 0.2, 0.0, 0);
			
		}
	}

	return color;
}

void main()
{
	vec2 coord = distort(texcoord);

    vec4 in_color = texture(screen_texture, coord);
    color = color_shift(in_color);
	color = add_lights(color, coord);
	color = fade_color(color);
}