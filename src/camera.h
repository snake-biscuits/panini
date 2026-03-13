// Using C23 Standard
#pragma once

#include "vector.h"


typedef struct Camera_s {
    Vec3  up, right, forward;
    Vec3  position;
} Camera;


// TODO: mat4 type


void init_camera(Camera *camera);
// void update_camera(Vec2 left_stick, Vec2 right_stick, Camera *camera);
void update_matrix(Camera camera, float *matrix[4][4]);
