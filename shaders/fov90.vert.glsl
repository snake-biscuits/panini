#version 450 core
layout (location = 0) in vec3 vertexPosition;
out vec3 position;
void main() {
    position = vertexPosition;

    mat4 view_matrix;
    float near = 0.1;
    float far = 1024.0;
    float r = far - near;
    float a = -far / r;
    float b = -far * near / r;
    // NOTE: 90deg fov on both axes, best for square viewport (cube texture)
    view_matrix[0] = vec4(1, 0, 0,  0);
    view_matrix[1] = vec4(0, 1, 0,  0);
    view_matrix[2] = vec4(0, 0, a, -1);
    view_matrix[3] = vec4(0, 0, b,  0);
    // TODO: rotate from -Y up +Z forward to +Z up +Y forward

    gl_Position = view_matrix * vec4(vertexPosition, 1.0);
}
