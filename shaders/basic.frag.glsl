#version 450 core

layout (location = 0) out vec4 outColour;

in vec3 position;
in vec3 normal;
in vec2 uv;


void main() {
    vec3 xyz = (position + 1) / 2;
    float b = 1 - xyz.r;
    outColour = vec4(xyz.rg, b, 1.0);
}
