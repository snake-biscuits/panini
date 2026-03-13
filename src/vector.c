// Using C23 Standard
// Math (-lm)
#include <math.h>

#include "vector.h"


Vec3 cross(Vec3 lhs, Vec3 rhs) {
    Vec3 out = {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x};
    return out;
}


float dot(Vec3 lhs, Vec3 rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}


float Vec2_sqrmagnitude(Vec2 v) {
    return v.x * v.x + v.y * v.y;
}


float Vec2_magnitude(Vec2 v) {
    return sqrtf(Vec2_sqrmagnitude(v));
}


void Vec2_normalise(Vec2 *v) {
    float magnitude = Vec2_magnitude(*v);
    v->x = v->x / magnitude;
    v->y = v->y / magnitude;
}


float Vec3_sqrmagnitude(Vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}


float Vec3_magnitude(Vec3 v) {
    return sqrtf(Vec3_sqrmagnitude(v));
}


void normalise_Vec3(Vec3 *v) {
    float magnitude = Vec3_magnitude(*v);
    v->x = v->x / magnitude;
    v->y = v->y / magnitude;
    v->z = v->z / magnitude;
}


int rotate(Vec3 *v, int axis, float degrees) {
    const float pi = 3.1415926535;
    float radians = degrees * pi / 180;
    float cos_ = cosf(radians);
    float sin_ = sinf(radians);
    // NOTE: no NaN checks
    switch (axis) {
        case 0:
            float y = v->y;
            v->y = y * cos_ - v->z * sin_;
            v->z = v->z * cos_ + y * sin_;
            break;
        case 1:
            float z = v->z;
            v->x = v->x * cos_ + z * sin_;
            v->z = z * cos_ + v->x * sin_;
            break;
        case 2:
            float x = v->x;
            v->x = x * cos_ - v->y * sin_;
            v->y = v->y * cos_ + x * sin_;
            break;
        default:
            return 1;  // invalid axis
    }
    return 0;
}
