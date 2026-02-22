// Using C23 Standard
#pragma once

// GLEW (-lGLEW)
#include <GL/glew.h>

// OpenGL (-lGL)
#include <GL/gl.h>

#include "geometry.h"


typedef struct Scene_s {
    // data references
    int     num_indices;
    // OpenGL object references
    GLuint  vertex_buffer;
    GLuint  index_buffer;
    GLuint  shader;
    // NOTE: just one big statically buffered object
    // NOTE: hardcoding the specifics of each buffer
    // -- vertex struct and setup can ultimately be whatever
    // -- always GL_TRIANGLES (triangle soup) GL_UNSIGNED_INT
} Scene;


// scene geo
void populate(Scene *scene, Geometry *geo);

// shader construction
int read_glsl(char* path, int glsl_length, const GLchar** glsl);
int compile_glsl(GLuint *shader, GLenum shader_type, int glsl_length, const GLchar* glsl);
void link_shader(GLuint vertex_shader, GLuint fragment_shader, GLuint *program);
// TODO: BONUS: cache compiled shader binary (save to file)


#if 0
Vertex vertices[3] = {
    {.position={-0.75, -0.75, 0}},
    {.position={+0.00, +0.75, 0}},
    {.position={+0.75, -0.75, 0}}};

uint32_t indices[3] = {0, 1, 2};

Geometry geo = {
    sizeof(vertices), sizeof(indices),
    vertices, indices};

Scene scene = {0, 0, 0, 0};
populate(&scene, &geo);

GLchar glsl[4096];  // 4KB limit, not messing with malloc
int glsl_length;

GLuint vertex_shader = 0;
glsl_length = read_glsl("...", sizeof(glsl), &glsl)
compile_glsl(&vertex_shader, GL_VERTEX_SHADER, glsl_length, glsl);

GLuint fragment_shader = 0;
glsl_length = read_glsl("...", sizeof(glsl), &glsl)
compile_glsl(&fragment_shader, GL_FRAGMENT_SHADER, glsl_length, glsl);

link_shader(vertex_shader, fragment_shader, &scene.shader)
#endif
