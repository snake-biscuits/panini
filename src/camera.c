// Using C23 Standard
#include "camera.h"


void init_camera(Camera *camera) {
    Vec3 right = {.x = 1, .y = 0, .z = 0};
    Vec3 up = {.x = 0, .y = 0, .z = 1};
    Vec3 forward = {.x = 0, .y = 1, .z = 0};
    Vec3 position = {.x = 0, .y = 0, .z = 0};
    camera->right = right;
    camera->up = up;
    camera->forward = forward;
    camera->position = position;
}


// TODO: perspective view matrix (to multiply against camera)
/*
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
*/


// TODO: SDL2 events -> InputState {left_stick, right_stick};
// void update_camera(Vec2 left_stick, Vec2 right_stick, Camera *camera);
// -- camera.forward + left_stick -> wish


void update_matrix(Camera camera, float *matrix[4][4]) {
    *matrix[0][0] = camera.right.x;
    *matrix[0][1] = camera.right.y;
    *matrix[0][2] = camera.right.z;
    *matrix[0][3] = 0;

    *matrix[1][0] = camera.up.x;
    *matrix[1][1] = camera.up.y;
    *matrix[1][2] = camera.up.z;
    *matrix[1][3] = 0;

    *matrix[2][0] = camera.forward.x;
    *matrix[2][1] = camera.forward.y;
    *matrix[2][2] = camera.forward.z;
    *matrix[2][3] = 0;

    *matrix[3][0] = camera.position.x;
    *matrix[3][1] = camera.position.y;
    *matrix[3][2] = camera.position.z;
    *matrix[3][3] = 1;
}
