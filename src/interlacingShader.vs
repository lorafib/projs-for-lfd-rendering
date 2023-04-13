#version 330 core
layout (location = 0) in vec3 in_pos;

out vec2 texCoords;

void main() {
    vec4 pos= vec4(vec3(2.0)*in_pos - vec3(1.0), 1.0);
    pos.z = -0.8;
    gl_Position = pos;
    texCoords = in_pos.xy;
}
