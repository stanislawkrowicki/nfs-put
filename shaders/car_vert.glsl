#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 WorldPos;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main()
{
    TexCoords = aTexCoords;
    vec4 worldPosition = M * vec4(aPos, 1.0);
    WorldPos = worldPosition.xyz;

    Normal = mat3(transpose(inverse(M))) * aNormal;
    gl_Position = P * V * worldPosition;
}
