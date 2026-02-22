// Using C23 Standard
#include <stdio.h>

#include "render_gl.h"


void populate(Scene *scene, Geometry *geo) {
    // vertex buffer
    glGenBuffers(1, &scene->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, scene->vertex_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(Vertex) * geo->num_vertices, geo->vertices,
        GL_STATIC_DRAW);

    // vertex attribs
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex), (void*)offsetof(Vertex, position));

    // index buffer
    glGenBuffers(1, &scene->index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene->index_buffer);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(uint32_t) * geo->num_indices, geo->indices,
        GL_STATIC_DRAW);
    scene->num_indices = geo->num_indices;
}


int read_glsl(char* path, int glsl_length, const GLchar** glsl) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {  // most likely file not found
        fprintf(stderr, "failed to open shader file: %s\n", path);
        return 0;
    }

    int file_length = fseek(file, 0, SEEK_END);
    if (file_length > glsl_length) {
        fprintf(stderr, "shader file is too long: %s (%d > %d)\n", path, file_length, glsl_length);
        return 0;
    }

    rewind(file);
    int bytes_read = fread(glsl, 1, file_length, file);
    if (bytes_read != file_length) {
        fprintf(stderr, "failed to read shader: %s\n", path);
        return 0;
    }

    return file_length;
}


int compile_glsl(GLuint *shader, GLenum shader_type, int glsl_length, const GLchar* glsl) {
    *shader = glCreateShader(shader_type);
    glShaderSource(*shader, 1, &glsl, &glsl_length);
    glCompileShader(*shader);

    GLint compiled;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == GL_FALSE) {
        fprintf(stderr, "shader failed to compile: %d\n", shader_type);

        GLint log_length;
        glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &log_length);
        GLchar log[4096];
        // NOTE: not checking if log_length > sizeof(log)
        glGetShaderInfoLog(*shader, sizeof(log), &log_length, log);
        fprintf(stderr, "%s\n", log);

        return 1;
    }

    return 0;
}


void link_shader(GLuint vertex_shader, GLuint fragment_shader, GLuint *program) {
    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);
    glLinkProgram(*program);
    glDetachShader(*program, vertex_shader);
    glDetachShader(*program, fragment_shader);
}
