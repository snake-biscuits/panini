// Using C23 Standard
#pragma once


typedef struct Vec2_s {
    float x;
    float y;
} Vec2;


typedef struct Vec3_s {
    float x;
    float y;
    float z;
} Vec3;


Vec3 cross(Vec3 a, Vec3 b);
float dot(Vec3 a, Vec3 b);

float Vec2_sqrmagnitude(Vec2 v);
float Vec2_magnitude(Vec2 v);
void Vec2_normalise(Vec2 *v);

float Vec3_sqrmagnitude(Vec3 v);
float Vec3_magnitude(Vec3 v);
void Vec3_normalise(Vec3 *v);

int rotate(Vec3 *v, int axis, float degrees);
// TODO: matrix multiplication
