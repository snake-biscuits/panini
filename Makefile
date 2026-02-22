CC := gcc
CFLAGS := -Wall --std=c23 -ggdb
# SDL2 + OpenGL
GLFLAGS := -lGLEW -lGL
SDL2FLAGS := `sdl2-config --cflags --libs`
# TODO: SDL3 + Vulkan

DUMMY != mkdir -p build

.PHONY: all run debug
# TODO: clean

# TODO: panini_vulkan.exe

all: build/panini_gl.exe

run: build/panini_gl.exe
	build/panini_gl.exe

debug: build/panini_gl.exe
	gdb -ex run -ex bt --args build/panini_gl.exe

build/panini_gl.exe: src/panini_gl.c src/render_gl.c
	$(CC) $(CFLAGS) $(GLFLAGS) $^ -o $@ $(SDL2FLAGS)
