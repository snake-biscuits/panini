#include <stdint.h>
#include <stdio.h>

#include "geometry.h"


int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("usage: %s folder/file.obj\n", argv[0]);
        return 1;
    }

    Vertex    vertices[512];
    uint32_t  indices[512];

    Geometry geo = {
        .num_vertices = 0,
        .max_vertices = sizeof(vertices) / sizeof(Vertex),
        .num_indices = 0,
        .max_indices = sizeof(indices) / sizeof(uint32_t),
        .vertices = vertices,
        .indices = indices};

    if (read_obj(argv[1], &geo) != 0) {
        printf("!!! parse failed !!!\n");
    }

    printf("geo = {\n");
    printf("    .num_vertices=%d\n", geo.num_vertices);
    printf("    .max_vertices=%d\n", geo.max_vertices);
    printf("    .num_indices=%d\n", geo.num_indices);
    printf("    .max_indices=%d\n", geo.max_indices);
    printf("    .vertices=(Vertex*)0x%016lX\n", (long)geo.vertices);
    printf("    .indices=(uint32_t*)0x%016lX\n", (long)geo.indices);
    printf("};\n\n");

    int i;
    for (i = 0; i < geo.num_vertices; i++) {
        printf(
            "geo.vertices[%d] = {{%+.2f, %+.2f, %+.2f}, {%+.2f, %+.2f, %+.2f}, {%+.2f, %+.2f}};\n",
            i,
            geo.vertices[i].position.x, geo.vertices[i].position.y, geo.vertices[i].position.z,
            geo.vertices[i].normal.x, geo.vertices[i].normal.y, geo.vertices[i].normal.z,
            geo.vertices[i].uv.x, geo.vertices[i].uv.y);
    };
    printf("\n");

    for (i = 0; i < geo.num_vertices; i++) {
        printf("geo.indices[%02d] = %d;\n", i, geo.indices[i]);
    };

    return 0;
}
