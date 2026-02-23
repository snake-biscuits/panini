#version 450 core
layout (location = 0) in vec3 vertexPosition;
out vec3 position;
void main() {
    position = vertexPosition;
    gl_Position = vec4(vertexPosition, 1.0);
}
