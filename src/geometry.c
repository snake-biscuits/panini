// Using C23 Standard
#include <stdio.h>
#include <stdlib.h>

#include "geometry.h"


int read_opcode(FILE *file, char* c, char *opcode) {
    if (!fread(opcode, 1, 1, file))
        return 1;  // EOF
    if (*opcode == '\n')
        return 0;  // empty line
    if (!fread(c, 1, 1, file))
        return 2;  // EOF
    if (*c == ' ' || *c == '\t') {
        return 0;
    } else if (*opcode == 'v') {
        switch (*c) {
            case 't':
            case 'n':
                *opcode = *c;
                if (!fread(c, 1, 1, file))
                    return 3;  // EOF
                if (!(*c == ' ' || *c == '\t'))
                    return 4;  // invalid "v[nt]" opcode
                return 0;
                break;
            default:
                return 5;  // unknown opcode
        }
    }
    // advance to whitespace after opcode
    while (*c != ' ' && *c != '\t') {
        if (!fread(c, 1, 1, file))
            return 4;  // EOF
        // NOTE: will consume newlines; "#\n" could cause bugs
    }
    return 0;
}


int consume_whitespace(FILE *file, char *c) {
    while (*c == ' ' || *c == '\t') {
        if (*c == '\n')
            return 0;  // end of line
        if (!fread(c, 1, 1, file))
            return 1;  // end of file
    }
    return 0;
}


int consume_line(FILE *file, char *c) {
    while (*c != '\n') {
        if (!fread(c, 1, 1, file))
            return 1;  // end of file
    }
    return 0;
}


int read_token(FILE *file, char *c, int len_token, char* token) {
    if (len_token == 0)
        return 1;  // 0 length token
    // skip leading whitespace
    if (*c == ' ' || *c == '\t') {
        if (consume_whitespace(file, c) != 0)
            return 2;  // hit eof before reaching token
        if (*c == '\n')  // TODO: just expose the consume_whitespace error code
            return 3;  // hit newline before reaching token
    }
    // collect non-whitespace characters into token
    int p = 0;
    while (*c != ' ' && *c != '\t' && *c != '\n') {
        token[p] = *c;
        p++;
        if (p >= len_token)
            return 4;  // undersized token
        if (!fread(c, 1, 1, file))
            return 5;  // end of file
    }
    token[p] = '\0';
    return 0;
}


int read_float_token(FILE *file, char *c, float *dest) {
    char token[32];
    if (read_token(file, c, sizeof(token), token) != 0)
        return 1;
    *dest = atof(token);
    // NOTE: atof has no error checking
    return 0;
}


// NOTE: assumes token is NULL terminated, we trust read_token
int read_vertex_token(FILE *file, char *c, int *vi, int *vti, int *vni) {
    // get full vertex token
    char token[32];
    if (read_token(file, c, sizeof(token), token) != 0)
        return 1;  // unexpected EOF
    // defaults
    *vti = 0;
    *vni = 0;
    int i = 0;  // index index (v, vn, vt)
    int s = 0;  // start of subtoken
    int e = 0;  // end of subtoken
    bool finished = false;
    while (!finished) {
        // separate & parse subtoken(s)
        if (token[e] == '/' || token[e] == '\0') {
            if (token[e] == '\0')
                finished = true;  // end of token
            // subtoken -> int
            token[e] = '\0';
            // read subtoken. or skip if empty ("//")
            int index = (e > s) ? atoi(&token[s]): 0;
            // NOTE: atoi is not checked for errors
            s = e + 1;
            // "yield" index
            switch (i) {
                case 0: *vi  = index; break;
                case 1: *vti = index; break;
                case 2: *vni = index; break;
                default: return 2;  // unreachable
            }
            i++;
        }
        e++;
    }
    if (i == 0)
        return 3;  // empty token; *vi was not set
    return 0;
}


int read_face(FILE *file, char* c, ObjFile *obj, Geometry *geo) {
    int first_vertex = geo->num_vertices;
    int num_vertices = 0;
    while (*c != '\n') {
        if (geo->num_vertices + 1 >= geo->max_vertices)
            return 1;  // out of memory (geo.vertices)
        int vi, vti, vni;
        if (read_vertex_token(file, c, &vi, &vti, &vni) != 0)
            return 2;  // failed to parse token
        // NOTE: haven't checked if the user tried to use 0 as an index
        if (vi == 0)  // must've been set to 0 by user
            return 3;  // indices start from 1 (0 is reserved for unset)
        // remap negative indices
        vi  = (vi  < 0) ? vi  + obj->num_positions : vi;
        vti = (vti < 0) ? vti + obj->num_uvs       : vti;
        vni = (vni < 0) ? vni + obj->num_normals   : vni;
        // bounds check indices
        if (vi  < 0 || vi  >= obj->num_positions
         || vti < 0 || vti >= obj->num_uvs
         || vni < 0 || vni >= obj->num_normals)
            return 4;  // index out of bounds
        // append vertex to geo
        Vertex vertex = {
            .position = obj->positions[vi],
            .normal = obj->normals[vni],
            .uv = obj->uvs[vti]};
        geo->vertices[geo->num_vertices] = vertex;
        geo->num_vertices++;
        num_vertices++;
    }

    if (num_vertices < 3)
        return 5;  // invalid polygon

    // triangle fan polygon
    for (int i = 2; i < num_vertices; i++) {
        if (geo->num_indices + 3 >= geo->max_indices)
            return 6;  // out of memory (geo.indices)
        geo->indices[geo->num_indices + 0] = first_vertex + 0;
        geo->indices[geo->num_indices + 1] = first_vertex + i - 1;
        geo->indices[geo->num_indices + 2] = first_vertex + i;
        geo->num_indices += 3;
    }

    return 0;
}


int read_obj(char* path, int max_vertices, int max_indices, Geometry *geo) {
    // NOTE: roughly copied from render_gl.c:read_glsl
    // TODO: src/file_io.{c,h}
    // -- int open(char* path, FILE *file, long *filesize);  // error handler
    // -- int read(FILE *file, int length, void *dest);  // EOF checker
    FILE *file = fopen(path, "r");
    if (file == NULL) {  // most likely file not found
        fprintf(stderr, "failed to open file: %s\n", path);
        return 1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "seek failed: %s\n", path);
        fclose(file);
        return 1;
    }
    long file_length = ftell(file);
    if (file_length == 0) {
        fprintf(stderr, "file is empty: %s", path);
        fclose(file);
        return 1;
    }
    rewind(file);

    // obj state
    Vec3 positions[1024] = {{0, 0, 0}};
    Vec3 normals[1024] = {{0, 0, 0}};
    Vec2 uvs[1024] = {{0, 0}};

    ObjFile obj = {
        .num_positions = 1,
        .num_normals = 1,
        .num_uvs = 1,
        .max_positions = sizeof(positions) / sizeof(Vec3),
        .max_normals = sizeof(normals) / sizeof(Vec3),
        .max_uvs = sizeof(uvs) / sizeof(Vec2),
        .positions = positions,
        .normals = normals,
        .uvs = uvs,
    };

    // parse
    char c = '\n';  // last char read
    char opcode = '\0';
    int  line_number = 1;
    bool eof_reached = false;
    bool failed = false;
    while (!failed && !eof_reached) {  // loop over lines
        if (c != '\n'
         || read_opcode(file, &c, &opcode) != 0) {
            failed = true;
            break;
        }
        // parse line
        switch (opcode) {
            case 'v':
                if (read_float_token(file, &c, &obj.positions[obj.num_positions].x) != 0
                 || read_float_token(file, &c, &obj.positions[obj.num_positions].y) != 0
                 || read_float_token(file, &c, &obj.positions[obj.num_positions].z) != 0)
                    failed = true;
                obj.num_positions++;
                if (obj.num_positions >= obj.max_positions)
                    failed = true;
                break;
            case 't':  // 'vt'
                if (read_float_token(file, &c, &obj.uvs[obj.num_uvs].x) != 0
                 || read_float_token(file, &c, &obj.uvs[obj.num_uvs].y) != 0)
                    failed = true;
                obj.num_uvs++;
                if (obj.num_uvs >= obj.max_uvs)
                    failed = true;
                break;
            case 'n':  // 'vn'
                if (read_float_token(file, &c, &obj.normals[obj.num_normals].x) != 0
                 || read_float_token(file, &c, &obj.normals[obj.num_normals].y) != 0
                 || read_float_token(file, &c, &obj.normals[obj.num_normals].z) != 0)
                    failed = true;
                obj.num_normals++;
                if (obj.num_normals >= obj.max_normals)
                    failed = true;
                break;
            case 'f':
                if (read_face(file, &c, &obj, geo) != 0)
                    failed = true;
                break;
            case '\n':  // empty line
                break;
            default:  // skip to end of line
                consume_line(file, &c);
                break;
        }

        // trailing whitespace
        // NOTE: no terminating newline is ok
        if (c != '\n') {
            if (consume_whitespace(file, &c) != 0)
                eof_reached = true;
        }

        line_number++;

        if (ftell(file) == file_length)
            eof_reached = true;
    }

    fclose(file);

    if (failed) {
        fprintf(stderr, "failed to parse line %d\n", line_number - 1);
        return 1;
    }

    return 0;
}
