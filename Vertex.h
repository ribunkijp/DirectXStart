#ifndef VERTEX_H
#define VERTEX_H

#include <cstdint>

struct Vertex {
    float x, y;
    float u, v;
    uint32_t color;
};

#endif