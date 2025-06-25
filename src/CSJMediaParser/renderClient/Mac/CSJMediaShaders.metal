#include <metal_stdlib>
#include <simd/simd.h>

#include "CSJMediaShaderTypes.h"

using namespace metal;

typedef struct {
    float3 position [[VertexAttributePosition]];
    float2 texCoord [[VertexAttributeTexCoord]];
} Vertex;

typedef struct {
    float4 position [[position]];
    float2 texCoord;
} VertexOut;

vertex VertexOut vertexShader(Vertex in [[stage_in]]) {
    VertexOut out;

    return out;
}

fragment float4 fragmentShader(VertexOut in [[stage_in]],
                               texture2d<half> textureY [[CSJMediaTextureY]],
                               texture2d<half> textureU [[CSJMediaTextureU]],
                               texture2d<half> textureV [[CSJMediaTextureV]]) {

    constexpr sampler colorSampler(mag_filter::linear, min_filter::linear);
ß
    // TODO: colorSampler will sampler the input texture, and then computer the rgba value;

    return float4();
}