#version 330

in vec2 tc;

layout (location = 0) out vec4 out_col;

uniform sampler2D diffuse;

void main() {  
    vec3 col = texture(diffuse,tc).rgb;
    out_col = vec4(col,1.0);   
}
