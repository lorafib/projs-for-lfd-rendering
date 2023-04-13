#version 330
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_norm;
layout (location = 2) in vec2 in_tc;

uniform mat4 model;
uniform mat4 view_normal;
uniform mat4 view;
uniform mat4 proj;

out vec2 tc;

void main() {
    tc = in_tc;
    gl_Position = proj * view * model * vec4(in_pos, 1.0);
}
