// Using C23 Standard
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    glClearColor(0.1, 0.4, 0.5, 1.0);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}


int init_scene(Scene *scene) {
    // TODO: load geo from .obj file
    Vertex vertices[] = {
        {.position={-0.8f, -0.8f,  -1.0f}},
        {.position={-0.8f, -0.8f, -20.0f}},
        {.position={-0.8f, +0.8f, -20.0f}},
        {.position={-0.8f, +0.8f,  -1.0f}},
        {.position={+0.8f, -0.8f,  -1.0f}},
        {.position={+0.8f, -0.8f, -20.0f}},
        {.position={+0.8f, +0.8f, -20.0f}},
        {.position={+0.8f, +0.8f,  -1.0f}},
    };

    uint32_t indices[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
    };

    Geometry geo = {
        sizeof(vertices) / sizeof(Vertex),
        sizeof(indices) / sizeof(uint32_t),
        vertices, indices};

    populate(scene, &geo);

    const GLchar glsl[4096] = "\0";
    int glsl_length = 0;

    GLuint vertex_shader = 0;
    glsl_length = read_glsl("shaders/fov90.vert.glsl", sizeof(glsl), (const GLchar**)&glsl);
    // NOTE: compile_glsl will fail automatically if the file fails to load (EOF)
    if (compile_glsl(&vertex_shader, GL_VERTEX_SHADER, glsl_length, glsl) != 0) {
        fprintf(stderr, "vertex_shader failed to compile\n");
        return 1;
    }

    GLuint fragment_shader = 0;
    glsl_length = read_glsl("shaders/clay.frag.glsl", sizeof(glsl), (const GLchar**)&glsl);
    if (compile_glsl(&fragment_shader, GL_FRAGMENT_SHADER, glsl_length, glsl) != 0) {
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
