#include "Shader.hlsli"

VertexOut main(VertexIn vIn) {
    VertexOut vOut;
    vOut.posH = float4(vIn.pos, 1.0f);
    vOut.texCoord = vIn.texCoord;

    return vOut;
}