#version 410 core

layout (location = 0) in vec3 vp;

uniform vec3 u_offset; // движение объекта

void main() {
    gl_Position = vec4(vp + u_offset, 1.0);
}