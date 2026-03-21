#ifndef __CSJMEDIASHADERTYPES_H__
#define __CSJMEDIASHADERTYPES_H__

#include <simd/simd.h>

typedef enum CSJMediaTextureIndex {
    CSJMediaTextureY = 0,
    CSJMediaTextureU = 1,
    CSJMediaTextureV = 2
} CSJMediaTextureIndex;

typedef enum CSJMediaBufferIndex {
    CSJMediaVertexBufferIndex = 0,
} CSJMediaBufferIndex;

typedef enum CSJMediVertexAttribute {
    VertexAttributePosition = 0,
    VertexAttributeTexCoord = 1,
} CSJMediVertexAttribute;

typedef struct CSJMediaVertexData {
    vector_float3 position;
    vector_float2 texCoord;
} CSJMediaVertexData;

typedef struct Vertex {
    float position[3];
    float texCoord[2];
} Vertex;

struct Vertex vertices[] = {
    {-1,  1, 0}, {0, 0},
    { 1,  1, 0}, {1, 0},
    {-1, -1, 0}, {0, 1},
    { 1, -1, 0}, {1, 1},
};

#endif // __CSJMEDIASHADERTYPES_H__