#version 460 core

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 u_offset;

void main() {
    vec3 finalPos = position + u_offset;
    gl_Position = projection * view * vec4(finalPos, 1.0);
}