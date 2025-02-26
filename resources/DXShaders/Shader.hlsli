Texture2D texY : register(t0);
Texture2D texU : register(t1);
Texture2D texV : register(t2);
Texture2D texRgb : register(t3);

SamplerState gSampler : register(s0);

struct VertexIn {
    float3 pos : SV_POSITION;
    float2 texCoord: TEXCOORD;
};

struct VertexOut {
    float4 posH : SV_POSITION;
    float2 texCoord : TEXCOORD;
};