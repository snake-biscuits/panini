// Using C23 Standard
#pragma once

// GLEW (-lGLEW)
#include <GL/glew.h>

// OpenGL (-lGL)
#include <GL/gl.h>

// SDL2 (`sdl2-config --cflags --libs`)
#include <SDL2/SDL.h>

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
int link_shader(GLuint vertex_shader, GLuint fragment_shader, GLuint *program);
// TODO: BONUS: cache compiled shader binary (save to file)

// draw
void draw_scene(SDL_Window **window, Scene *scene);
