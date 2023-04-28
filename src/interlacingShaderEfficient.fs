#version 330 core
in vec2 texCoords; 

out vec4 fragColor; 
 
//Calibration values 
uniform float pitch; 
uniform float tilt; 
uniform float center; 
uniform int invView; 
uniform float subp; 
uniform float aspect_ratio;

//Quilt settings 
uniform vec3 tile; 
uniform sampler2D quilt;

//Parameters of the adapted projective mapping
uniform float tilt_angle;
uniform float pitch_d;
uniform float fovFactor;
uniform float aspect_ratio_n;
uniform int pr_tl;
uniform int pr_total;
 
//Transform the texture coordinates and read the original color from the selected quilt view index
vec2 readColor(vec2 tc, float index) 
{ 
	vec2 tc_new;
	//Transform local x texture coordinate - formulas stem from inverted matrices
	if(tilt_angle<0) {
		tc_new.x = (cos(-tilt_angle) * aspect_ratio * (tc.x + tan(-tilt_angle)/aspect_ratio)  -  sin(-tilt_angle) * tc.y)
						/ (aspect_ratio_n * fovFactor);
	}
	else{
		tc_new.x = (cos(-tilt_angle) * aspect_ratio * tc.x  -  sin(-tilt_angle) * tc.y)
						/ (aspect_ratio_n * fovFactor);
	}
	//Reset projection matrix skew parameter
	float si_reverse;                                
	if (invView != 1){
		si_reverse = -(tile.z-index) / tile.z + 0.5;                             
	}
	else{
		si_reverse = index / tile.z + 0.5;
	}
	tc_new.x += (si_reverse + (pr_tl - pr_total/2) / tile.z)  *  (1.0f / pitch_d);


	//Transform local y texture coordinate - formulas stem from inverted matrices
	if(tilt_angle<0) {
		tc_new.y = (cos(-tilt_angle) * tc.y  +  sin(-tilt_angle) * tc.x * aspect_ratio) 
						/ fovFactor;
	}
	else{
		tc_new.y = (cos(-tilt_angle) * tc.y  +  sin(-tilt_angle) * tc.x * aspect_ratio  +  sin(tilt_angle) * aspect_ratio) 
						/ fovFactor;
	}
	
	//Transform local texture coordinates to the position of the desired view in the quilt
	float x = (mod(index, tile.x) + tc_new.x) / tile.x ; 
	float y = (floor(index / tile.x) + tc_new.y) / tile.y; 
	
	return vec2(x,y);	
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