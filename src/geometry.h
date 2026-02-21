// Using C23 Standard
#pragma once

#include <stdint.h>


typedef struct Vec3_s {
    float x;
    float y;
    float z;
} Vec3;


typedef struct Vertex_s {
    Vec3  position;
    // Vec3  normal;
    // Vec2  uv0;
} Vertex;


typedef struct Geometry_s {
    int       num_vertices;
    int       num_indices;
    Vertex   *vertices;
    uint32_t *indices;
} Geometry;
