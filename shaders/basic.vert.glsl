#version 450 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 0) in vec3 vertexNormal;
layout (location = 0) in vec2 vertexUv;

out vec3 position;
out vec3 normal;
out vec2 uv;


void main() {
    position = vertexPosition;
    normal = vertexNormal;
    uv = vertexUv;

    gl_Position = vec4(vertexPosition, 1.0);
}
