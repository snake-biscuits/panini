#version 450 core

layout (location = 0) out vec4 outColour;

in vec3 position;
in vec3 normal;
in vec2 uv;


void main() {
    vec3 albedo = vec3(1, 1, 1);
    vec3 ambient = vec3(.35, .35, .35);

    vec3 light = vec3(.3, .2, .5);
    vec3 diffuse = albedo * max(dot(normal, light), 0.15);

    // NOTE: depth is tied to world position, not camera position
    float depth = min(-position.z / 10, 1);
    vec3 fog = vec3(.1, .4, .5);
    vec3 colour = mix(diffuse + ambient, fog, depth);

    outColour = vec4(colour, 1.0);
}
