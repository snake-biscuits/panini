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

    Geometry geo = {0, 0, &vertices[0], &indices[0]};

    if (read_obj(
        argv[1],
        sizeof(vertices) / sizeof(Vertex),
        sizeof(indices) / sizeof(uint32_t),
        &geo) != 0) {
        printf("!!! parse failed !!!\n");
    }

    printf("geo = {\n");
    printf("    .num_vertices=%d\n", geo.num_vertices);
    printf("    .num_indices=%d\n", geo.num_indices);
    printf("    .vertices=0x%016lX\n", (long)geo.vertices);
    printf("    .indices=0x%016lX\n", (long)geo.indices);

    return 0;
}
