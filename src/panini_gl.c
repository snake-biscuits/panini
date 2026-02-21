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


// default to PSVita display resolution
#define WIDTH  960
#define HEIGHT 544


//////////////////////
// -- STRUCTURES -- //
//////////////////////


typedef struct Clock_s {
    uint64_t accumulator;
    uint64_t delta;
    uint64_t prev_tick;
    uint64_t tick_length;
} Clock;


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
    // -- alway GL_TRIANGLES (triangle soup) GL_UNSIGNED_INT
} Scene;


/////////////////////
// -- FUNCTIONS -- //
/////////////////////


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

// TODO: load shader from files & compile
// -- filesystem navigation
// -- file reads
// -- shader object construction
// -- shader program compilation
// -- surface compile errors
// -- BONUS: cache compiled shader binary (save to file)


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
}


void draw_OpenGL(SDL_Window **window) {  // Scene *scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: update scene according to tick updates
    // -- move the camera

    // TODO: render scene
    // switch to scene shader
    // -- glUseProgram(scene->shader);
    // update camera uniform
    // -- update_camera(&camera, &tick_inputs)
    // -- glUniformMatrix4fv(...);
    // draw scene
    // -- glDrawElements(GL_TRIANGLES, scene->num_indices, GL_UNSIGNED_INT, NULL);

    // TODO: panini reprojection
    // -- 6x camera matrices (or some clever shader trick)
    // -- render each view to a renderbuffer
    // -- renderbuffer -> cube texture
    // -- panini reprojection "scene"

    SDL_GL_SwapWindow(*window);
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
        return 1;
    }

    init_OpenGL();

    // TODO: test triangle
    // Scene scene;
    // init_scene(&scene)
    // - init_buffers(&scene, geometry)
    // - init_shaders(&scene, raw_vertex_shader, raw_fragment_shader)
    // - init_uniforms(&scene)  // camera

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
        // TODO: pass in scene & dt
        draw_OpenGL(&window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
