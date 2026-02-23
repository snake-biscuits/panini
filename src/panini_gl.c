// Using C23 Standard
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GLEW (-lGLEW)
#include <GL/glew.h>

// OpenGL (-lGL)
#include <GL/gl.h>

// SDL2 (`sdl2-config --cflags --libs`)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "geometry.h"
#include "render_gl.h"


// default to PSVita display resolution
#define WIDTH  960
#define HEIGHT 544


typedef struct Clock_s {
    uint64_t accumulator;
    uint64_t delta;
    uint64_t prev_tick;
    uint64_t tick_length;
} Clock;


void print_usage(char* argv_0) {
    printf("%s [WIDTH HEIGHT]\n", argv_0);
    printf("SDL2 + OpenGL Panini Projection Test\n");
    printf("    WIDTH    viewport width\n");
    printf("    HEIGHT   viewport height\n");
}


int init_window(int width, int height, SDL_Window **window) {
    SDL_Init(SDL_INIT_VIDEO);
    *window = SDL_CreateWindow(
        "Panini Projection Test (SDL2 + OpenGL)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);

    if (*window == NULL) {
        fprintf(stderr, "Couldn't make a window: %s\n", SDL_GetError());
        return 1;
    }

    return 0;
}


// TODO: poll available OpenGL versions & extensions
int init_context(int major, int minor, SDL_Window **window, SDL_GLContext *context) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    *context = SDL_GL_CreateContext(*window);

    if (*context == NULL) {
        fprintf(stderr, "Couldn't initialise GL context: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetSwapInterval(0);

    return 0;
}


void init_OpenGL() {
    glewInit();  // load OpenGL functions
    // glClearColor(0.1, 0.4, 0.5, 1.0);
    glClearColor(0.0, 0.0, 0.0, 1.0);
}


// isn't drawing anything!?
int init_scene(Scene *scene) {
    Vertex vertices[] = {
        {.position={-0.8f, -0.8f, +0.0f}},
        {.position={+0.8f, -0.8f, +0.0f}},
        {.position={+0.8f, +0.8f, +0.0f}},
        {.position={-0.8f, +0.8f, +0.0f}}};

    uint32_t indices[] = {
        0, 1, 2,
        0, 2, 3};

    Geometry geo = {
        sizeof(vertices) / sizeof(Vertex),
        sizeof(indices) / sizeof(uint32_t),
        vertices, indices};

    populate(scene, &geo);

    // GLchar glsl[4096];  // 4KB limit, not messing with malloc
    // int glsl_length = read_glsl("...", sizeof(glsl), &glsl);
    // compile_glsl(&vertex_shader, GL_VERTEX_SHADER, glsl_length, glsl);
    // glsl_length = read_glsl("...", sizeof(glsl), &glsl);
    // compile_glsl(&fragment_shader, GL_FRAGMENT_SHADER, glsl_length, glsl);

    const GLchar* vertex_glsl =
        "#version 450 core\n"
        "layout (location = 0) in vec3 vertexPosition;\n"
        "out vec3 position;\n"
        "void main() {\n"
        "    position = vertexPosition;\n"
        "    gl_Position = vec4(vertexPosition, 1.0);\n"
        "}\n";

    GLuint vertex_shader = 0;
    if (compile_glsl(&vertex_shader, GL_VERTEX_SHADER, strlen(vertex_glsl), vertex_glsl) != 0) {
        fprintf(stderr, "vertex_shader failed to compile\n");
        return 1;
    }

    const GLchar* fragment_glsl =
        "#version 450 core\n"
        "layout (location = 0) out vec4 outColour;\n"
        "in vec3 position;\n"
        "void main() {\n"
        "    vec3 xyz = (position + 1) / 2;\n"
        "    float b = 1 - xyz.r;\n"
        "    outColour = vec4(xyz.rg, b, 1.0);\n"
        "}\n";

    GLuint fragment_shader = 0;
    if (compile_glsl(&fragment_shader, GL_FRAGMENT_SHADER, strlen(fragment_glsl), fragment_glsl) != 0) {
        fprintf(stderr, "fragment_shader failed to compile\n");
        return 1;
    }

    if (link_shader(vertex_shader, fragment_shader, &scene->shader) != 0) {
        fprintf(stderr, "link_shader failed\n");
        return 1;
    }

    return 0;
}


int main(int argc, char* argv[]) {
    if (argc <= 0 || argc == 2 || argc > 3) {
        print_usage(argv[0]);
    }

    int width  = WIDTH;
    int height = HEIGHT;
    if (argc == 3) {
        width  = atoi(argv[1]);
        height = atoi(argv[2]);
    }

    SDL_Window *window = NULL;
    if (init_window(width, height, &window) != 0) {
        fprintf(stderr, "init_window failed\n");
        return 1;
    }

    // TODO: capture mouse when window is active
    // -- need to disable when focus changes
    // SDL_SetRelativeMouseMode(SDL_TRUE);
    // SDL_CaptureMouse(SDL_TRUE);

    SDL_GLContext context = NULL;  // void*
    if (init_context(4, 5, &window, &context) != 0) {
        fprintf(stderr, "init_context failed\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    init_OpenGL();

    Scene scene = {0, 0, 0, 0};
    if (init_scene(&scene) != 0) {
        fprintf(stderr, "init_scene failed\n");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Clock clock = {
        .accumulator = 0,
        .delta = 0,
        .prev_tick = SDL_GetTicks64(),
        .tick_length = 15  // 15ms per tick (~66.67 ticks per second)
    };

    bool running = true;
    while (running) {
        // handle input events
        // TODO: break out into a function
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                    break;
                default: break;
            }
        }

        // simulate tick(s)
        // TODO: break out into a function
        clock.delta = (SDL_GetTicks64() - clock.prev_tick) + clock.accumulator;
        while (clock.delta >= clock.tick_length) {
            // TODO: simulate one tick
            // -- input -> state
            clock.delta -= clock.tick_length;
        }
        clock.accumulator = clock.delta;
        clock.prev_tick = SDL_GetTicks64();

        // draw
        draw_scene(&window, &scene);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
