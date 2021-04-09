#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float time;

// Output color
layout(location = 0) out  vec4 color;

/// Ref: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
//	Classic Perlin 2D Noise 
//	by Stefan Gustavson
vec2 fade(vec2 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
float cnoise(vec2 P){
	vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
	vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
	Pi = mod(Pi, 289.0); // To avoid truncation effects in permutation
	vec4 ix = Pi.xzxz;
	vec4 iy = Pi.yyww;
	vec4 fx = Pf.xzxz;
	vec4 fy = Pf.yyww;
	vec4 i = permute(permute(ix) + iy);
	vec4 gx = 2.0 * fract(i * 0.0243902439) - 1.0; // 1/41 = 0.024...
	vec4 gy = abs(gx) - 0.5;
	vec4 tx = floor(gx + 0.5);
	gx = gx - tx;
	vec2 g00 = vec2(gx.x,gy.x);
	vec2 g10 = vec2(gx.y,gy.y);
	vec2 g01 = vec2(gx.z,gy.z);
	vec2 g11 = vec2(gx.w,gy.w);
	vec4 norm = 1.79284291400159 - 0.85373472095314 * 
	vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11));
	g00 *= norm.x;
	g01 *= norm.y;
	g10 *= norm.z;
	g11 *= norm.w;
	float n00 = dot(g00, vec2(fx.x, fy.x));
	float n10 = dot(g10, vec2(fx.y, fy.y));
	float n01 = dot(g01, vec2(fx.z, fy.z));
	float n11 = dot(g11, vec2(fx.w, fy.w));
	vec2 fade_xy = fade(Pf.xy);
	vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
	float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
	return 2.3 * n_xy;
}

void main()
{
	// randomize noise with time  
    float x = texcoord.x * 8.0;
    float y = texcoord.y * 5.0 + time;
    
	// ref https://petewerner.blogspot.com/2015/02/intro-to-curl-noise.html
    // curl
    float eps =	0.5f; // soft factor
	float n1, n2, a, b;
    
    // curl derivative for x
    n1 = cnoise(vec2(x, y	+ eps));
    n2 = cnoise(vec2(x, y - eps));	
    a =	(n1-n2)/(2.f * eps);	
    
    // curl derivative for y
    n1 = cnoise(vec2(x + eps, y));
    n2 = cnoise(vec2(x - eps, y));
    b = (n1 - n2) / (2.f * eps);
    
    // the final curl
    vec2 curl = vec2(a, -b);

	// original texel
	vec4 texel = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));

	// adapt texel
	float red = texel.x + max(0.0, curl.x * 0.6);
	float green = min(red, texel.y + curl.y * 0.3);
	vec3 fire_texel = vec3(red, green, texel.z);

	// adapt alpha
	float alpha = (curl.y * 0.8 + texel.w) * texel.w;

	// final output
	color = vec4(fire_texel.x, fire_texel.y, fire_texel.z, alpha);
}
