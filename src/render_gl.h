// Using C23 Standard
#pragma once

// GLEW (-lGLEW)
#include <GL/glew.h>

// OpenGL (-lGL)
#include <GL/gl.h>

// SDL2 (`sdl2-config --cflags --libs`)
#include <SDL2/SDL.h>

#include "geometry.h"


// bucket of opengl state for rendering
typedef struct Scene_s {
    // data references
    int     num_indices;
    // NOTE: assuming GL_TRIANGLE & GL_UNSIGNED_INT for draw calls
    // OpenGL object references
    GLuint  vertex_array;
    GLuint  vertex_buffer;
    GLuint  index_buffer;
    GLuint  shader;
} Scene;


// scene geo
void populate(Scene *scene, Geometry *geo);

// shader construction
int read_glsl(char* path, int glsl_length, const GLchar** glsl);
int compile_glsl(GLuint *shader, GLenum shader_type, int glsl_length, const GLchar* glsl);
int link_shader(GLuint vertex_shader, GLuint fragment_shader, GLuint *program);
int cache_shader(GLuint *program, char* path);
// int load_shader(GLuint *program, char* path);
// -- glProgramBinary(*program, *bin_format, bin, sizeof(bin));

// draw
void draw_scene(SDL_Window **window, Scene *scene);
