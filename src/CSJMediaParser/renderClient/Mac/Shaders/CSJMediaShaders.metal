#include <metal_stdlib>
#include <simd/simd.h>

#include "CSJMediaShaderTypes.h"

using namespace metal;

typedef struct {
    float3 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
} VertexIn;

typedef struct {
    float4 position [[position]];
    float2 texCoord;
} VertexOut;

vertex VertexOut rgbaVertexShader(VertexIn in [[stage_in]]) {
    VertexOut out;

    out.position = float4(in.position, 1);
    out.texCoord = in.texCoord;

    return out;
}

fragment float4 rgbaFragmentShader(VertexOut in [[stage_in]],
                                   texture2d<float> tex [[texture(0)]]) {
    constexpr sampler sample(address::clamp_to_edge, filter::linear);
    return tex.sample(sample, in.texCoord);
}

// fragment float4 fragmentShader(VertexOut in [[stage_in]],
//                                texture2d<half> textureY [[CSJMediaTextureY]],
//                                texture2d<half> textureU [[CSJMediaTextureU]],
//                                texture2d<half> textureV [[CSJMediaTextureV]]) {

//     constexpr sampler colorSampler(mag_filter::linear, min_filter::linear);

//     // TODO: colorSampler will sampler the input texture, and then computer the rgba value;

//     return float4();
// }