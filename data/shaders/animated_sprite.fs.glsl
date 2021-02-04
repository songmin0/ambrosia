#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform vec3 fcolor;

// Animation data
uniform float frame;

// test 2d array sampler
//uniform sampler2D sampler0;
uniform sampler2DArray array_sampler;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	// calculate texcoord.x and texcoord.y here...?
	// texture(sampler2d, vec2 [texture coordinates of sample]) should return gvec4
	// texture coordinates go from 0 to 1 in both x and y...

	// testing getting only half of the texture
	
	//gvec4 sprite = texture(sampler0, vec2(0.5, 0.5));
	//color = vec4(fcolor, 1.0) * sprite;

	// feeding it a float doesn't work... we have to divide it by texcoords... ugh...
	// Raoul is 8 rows x 8 columns, with only 6 sprites on the last (8th) row

	//float xoff = texcoord.x / 8.0;
	//float yoff = texcoord.y / 8.0;

	// this gives the first 1x1 frame...
	//vec2 sprite_coords = vec2((texcoord.x / 8.0), (texcoord.y / 8.0));

	// this gives the first 2x2 frame...
	//vec2 sprite_coords = vec2((texcoord.x / 8.0) * 2.0, (texcoord.y / 8.0)*2.0);

	// this gives the first row, 2 columns, but with a wrong widths...
	//vec2 sprite_coords = vec2((texcoord.x / 8.0) * 2.0, (texcoord.y / 8.0));

	vec2 sprite_coords = texcoord;

	//color = vec4(fcolor, 1.0) * texture(sampler0, sprite_coords);
	//color = texture(sampler0, texcoord);
	// frame has to be a float, even if it's technically treated as an int, since we're making a vec3 with texcoord
	color = texture(array_sampler, vec3(texcoord, frame));

	// f we gotta fking USE the uniform for it to show up sometimes
	// by USE you REALLY have to USE IT, like it's gotta contribute to the output
	// so copying it like we did here isn't enough, the compiler still garbages it
	//vec4 blah = texture(array_sampler, vec3(sprite_coords, frame));
	//int copy = frame;


	//color = vec4(fcolor, 1.0) * texture(array_sampler, vec3(sprite_coords, frame));
	

	//color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	//im so done, draw a white square for every texel
	// why the fk does this even fail to compile legit
	//color = vec4(1.0, 1.0, 1.0, 1.0);
}
