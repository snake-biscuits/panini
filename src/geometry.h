// Using C23 Standard
#pragma once

#include <stdio.h>
// #include <stdlib.h>  // atof & atoi
#include <stdint.h>


typedef struct Vec2_s {
    float x;
    float y;
} Vec2;


typedef struct Vec3_s {
    float x;
    float y;
    float z;
} Vec3;


typedef struct Vertex_s {
    Vec3  position;
    Vec3  normal;
    Vec2  uv;
} Vertex;


typedef struct Geometry_s {
    int       num_vertices;
    int       num_indices;
    Vertex   *vertices;
    uint32_t *indices;
} Geometry;


// .obj file parser
int read_obj(char* path, int max_vertices, int max_indices, Geometry *geo);
// general parser tools
int consume_line(FILE *file, char *c);
int consume_whitespace(FILE *file, char *c);
// get line start
int read_opcode(FILE *file, char *c, char *opcode);
// NOTE: consumes leading whitespace
int read_token(FILE *file, char *c, int len_token, char* token);
int read_float_token(FILE *file, char *c, float *dest);
int read_vertex_token(FILE *file, char *c, int *vi, int *vni, int *vti);
