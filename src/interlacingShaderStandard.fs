#version 330 core
in vec2 texCoords; 

//Inspired by Looking Glass SDK: HoloPlayShaders.h
out vec4 fragColor; 
 
// Calibration values 
uniform float pitch; 
uniform float tilt; 
uniform float center; 
uniform int invView; 
uniform float subp; 
 
// Quilt settings 
uniform vec3 tile; 
uniform sampler2D quilt; 
 
//Read the color from the selected quilt view index
vec2 readColor(vec2 tc, float index) 
{ 
	float x = (mod(index, tile.x) + tc.x) / tile.x; 
	float y = (floor(index / tile.x) + tc.y) / tile.y; 
	
	return vec2(x, y); 
} 
 
void main() 
{ 
	float invert = 1.0; 
	if (invView == 1) invert = -1.0; 
	vec4 rgb[3]; float index;
	for (int i=0; i < 3; i++) 
	{ 
		index = (texCoords.x + i * subp + texCoords.y * tilt) * pitch - center; 
		index = mod(index + ceil(abs(index)), 1.0); 
		index *= invert; 
		index *= tile.z; 
		vec4 colB = texture(quilt, readColor(texCoords, floor(index))); 
		vec4 colT = texture(quilt, readColor(texCoords, ceil(index))); 
		rgb[i] = mix(colB, colT, index - floor(index)); 
	} 
	fragColor = vec4(rgb[0].r, rgb[1].g, rgb[2].b, 1.0); 
	
} 