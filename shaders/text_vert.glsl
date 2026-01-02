#version 450 core
layout(location = 0) in vec2 aPos;

uniform mat4 u_ortho;

void main() {
    gl_Position = u_ortho * vec4(aPos, 0.0, 1.0);
}