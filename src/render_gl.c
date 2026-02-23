// Using C23 Standard
#include <stdio.h>

// SDL2 (`sdl2-config --cflags --libs`)
#include <SDL2/SDL.h>

#include "render_gl.h"


void populate(Scene *scene, Geometry *geo) {
    // NOTE: core OpenGL profiles must use a VAO
    // -- this restriction does not apply to the compatibility profile
    glGenVertexArrays(1, &scene->vertex_array);
    glBindVertexArray(scene->vertex_array);

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
    fclose(file);

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


int link_shader(GLuint vertex_shader, GLuint fragment_shader, GLuint *program) {
    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);
    glLinkProgram(*program);
    GLint is_linked;
    glGetProgramiv(*program, GL_LINK_STATUS, &is_linked);
    if (is_linked != GL_TRUE) {
        fprintf(stderr, "shader program failed to link\n");
        GLint log_length;
        glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &log_length);
        GLchar log[4096];
        // NOTE: not checking if log_length > sizeof(log)
        glGetProgramInfoLog(*program, sizeof(log), &log_length, log);
        fprintf(stderr, "%s\n", log);
        return 1;
    }

    glDetachShader(*program, vertex_shader);
    glDeleteShader(vertex_shader);
    glDetachShader(*program, fragment_shader);
    glDeleteShader(fragment_shader);

    glValidateProgram(*program);
    GLint is_valid;
    glGetProgramiv(*program, GL_VALIDATE_STATUS, &is_valid);
    if (is_valid != GL_TRUE) {
        fprintf(stderr, "invalid shader program\n");
        GLint log_length;
        glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &log_length);
        GLchar log[4096];
        // NOTE: not checking if log_length > sizeof(log)
        glGetProgramInfoLog(*program, sizeof(log), &log_length, log);
        fprintf(stderr, "%s\n", log);
        return 1;
    }

    return 0;
}


int cache_shader(GLuint *program, char* path) {
    GLsizei bin_size = 0;
    GLenum  bin_type = 0;
    uint8_t bin[8092];

    glGetProgramiv(*program, GL_PROGRAM_BINARY_LENGTH, &bin_size);
    if (bin_size > sizeof(bin)) {
        fprintf(stderr, "compiled shader is too large: %d > %lu\n", bin_size, sizeof(bin));
        return 1;
    }

    glGetProgramBinary(*program, sizeof(bin), &bin_size, &bin_type, &bin);

    printf("bin_size=%d, bin_type=0x%04X\n", bin_size, bin_type);
    FILE *bin_file = fopen("panini.shader", "wb");
    fwrite(bin, sizeof(bin_type), bin_type, bin_file);
    fwrite(bin, 1, bin_size, bin_file);
    fclose(bin_file);

    return 0;
}


void draw_scene(SDL_Window **window, Scene *scene) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(scene->shader);
    // TODO: update shader uniforms (view matrix)
    glDrawElements(GL_TRIANGLES, scene->num_indices, GL_UNSIGNED_INT, NULL);

    // TODO: panini reprojection
    // -- 6x camera matrices (or some clever shader trick)
    // -- render each view to a renderbuffer
    // -- renderbuffer -> cube texture
    // -- panini reprojection "scene"

    SDL_GL_SwapWindow(*window);
}
