#version 450 core
layout (location = 0) out vec4 outColour;
in vec3 position;
void main() {
    vec3 albedo = vec3(.6, .6, .6);
    vec3 fog = vec3(.1, .4, .5);
    float depth = min(-position.z / 10, 1);
    vec3 ambient = vec3(.02, .02, .02);
    vec3 colour = mix(albedo, fog, depth) + ambient;
    outColour = vec4(colour, 1.0);
}
