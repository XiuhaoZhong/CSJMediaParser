#include "Shader.hlsli"

cbuffer FrameParams : register(b0) {
    int IsI420;
    float pad[3]; // help align 16 bytes memory.
};

float3 NV12ToRGB_BT601(float y, float2 uv);
float3 NV12ToRGB_BT709(float y, float2 uv);

float3 I420ToRGB_BT601(float y, float u, float v);
float3 I420ToRGB_BT709(float y, float u, float v);

SamplerState linearSampler {
    Filter = MIN_MAG_MIP_FILTER;
    AddressU = Clamp;
    AddressV = Clamp;
};

float4 main(VertexOut pIn) : SV_Target {
    float3 rgb = float3(0, 0, 0);
    if (IsI420 == 1) {
        float y = texY.Sample(linearSampler, pIn.texCoord);
        float u = texU.Sample(linearSampler, pIn.texCoord);
        float v = texV.Sample(linearSampler, pIn.texCoord);

        rgb = I420ToRGB_BT709(y, u, v);
    }

    return float4(saturate(rgb), 1.0f);
}

float3 NV12ToRGB_BT601(float y, float2 uv) {
    float Y = y;
    float U = uv.x - 0.5f;
    float V = uv.y - 0.5f;

    float R = Y + 1.402f * V;
    float G = Y - 0.34414f * U - 0.71414f * V;
    float B = Y + 1.772f * U;

    return float3(R, G, B);
}

float3 NV12ToRGB_BT709(float y, float2 uv) {
    float Y = y;
    float U = uv.x - 0.5f;
    float V = uv.y - 0.5f;

    float R = Y + 1.5748f * V;
    float G = Y - 0.1873f * U - 0.4681f * V;
    float B = Y + 1.8556f * U;

    return float3(R, G, B);
}

float3 I420ToRGB_BT601(float y, float u, float v) {
    return float3(1.0, 1.0, 1.0);
}

float3 I420ToRGB_BT709(float y, float u, float v) {
    float Y = y;
    float U = u - 0.5f;
    float V = v - 0.5f;

    float R = Y + 1.5748f * V;
    float G = Y - 0.1873f * U - 0.4681f * V;
    float B = Y + 1.8556f * U;

    return float3(R, G, B);
}