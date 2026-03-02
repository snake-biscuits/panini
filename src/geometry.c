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
            int index = (e - s <= 1) ? atoi(&token[s]) : 0;
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

    // parse
    Vec3 vs[1024] = {{0, 0, 0}};
    int  num_vs = 1;
    int  max_vs = sizeof(vs) / sizeof(Vec3);
    Vec3 vns[1024] = {{0, 0, 0}};
    int  num_vns = 1;
    int  max_vns = sizeof(vns) / sizeof(Vec3);
    Vec2 vts[1024] = {{0, 0}};
    int  num_vts = 1;
    int  max_vts = sizeof(vts) / sizeof(Vec2);
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
                if (read_float_token(file, &c, &vs[num_vs].x) != 0
                 || read_float_token(file, &c, &vs[num_vs].y) != 0
                 || read_float_token(file, &c, &vs[num_vs].z) != 0)
                    failed = true;
                printf(" %03d | v %f %f %f\n",
                    line_number, vs[num_vs].x, vs[num_vs].y, vs[num_vs].z);  // DEBUG
                num_vs++;
                if (num_vs >= max_vs)
                    failed = true;
                break;
            case 't':  // 'vt'
                if (read_float_token(file, &c, &vts[num_vts].x) != 0
                 || read_float_token(file, &c, &vts[num_vts].y) != 0)
                    failed = true;
                printf(" %03d | vt %f %f\n",
                    line_number, vts[num_vts].x, vts[num_vts].y);  // DEBUG
                num_vts++;
                if (num_vts >= max_vts)
                    failed = true;
                break;
            case 'n':  // 'vn'
                if (read_float_token(file, &c, &vns[num_vns].x) != 0
                 || read_float_token(file, &c, &vns[num_vns].y) != 0
                 || read_float_token(file, &c, &vns[num_vns].z) != 0)
                    failed = true;
                printf(" %03d | vn %f %f %f\n",
                    line_number, vns[num_vns].x, vns[num_vns].y, vns[num_vns].z);  // DEBUG
                num_vns++;
                if (num_vns >= max_vns)
                    failed = true;
                break;
            case 'f':
                // NOTE: this will be complex, use a function
                // int read_face_tokens(FILE* file, char* c,
                //   int *num_vs, int max_vs, Vec3* vs,
                //   int *num_vts, int max_vts, Vec2* vts,
                //   int *num_vns, int max_vns, Vec3* vns,
                //   int max_vertices, int max_indices, Geo *geo);
                // NOTE: that's a lot of state, use a struct?
                // int append_face(FILE *file, char* c, Obj *obj, Geo *geo);

                // TODO:
                // -- token -> v_index, vn_index, vt_index
                // -- negative index handling
                // NOTE: we can use 0 for unspecified fields (indices start from 1)
                // -- the user shouldn't use that index though
                // -- also, v_index should never be 0

                // vertices_index = geo->num_vertices
                // int ngon_verts = 0;
                // while (c != '\n') {
                //     read_vertex_token(file, &c, &v_index, &vt_index, &vn_index);
                //     // TODO: append to geo->vertices (num_vertices++ w/ bounds check)
                //     // NOTE: no checking for duplicate vertices
                //     ngon_verts++;

                // #define append(p, i, v)  p[i] = v; i++;  // TODO: bounds check
                // #undef append
                
                // TODO: polygon -> triangle fan indices

                printf(" %03d | f ???\n", line_number);
                consume_line(file, &c);  // skip, for now
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
