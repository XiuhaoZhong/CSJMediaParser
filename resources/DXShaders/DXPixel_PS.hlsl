#include "Shader.hlsli"

float4 main(VertexOut pIn) : SV_Target {
    float4 color = float4(1.0f, 0.0f, 1.0f, 1.0f);//texRgb.Sample(gSampler, pIn.texCoord);//float4(1.0f, 0.0f, 1.0f, 1.0f);
    return color;
}