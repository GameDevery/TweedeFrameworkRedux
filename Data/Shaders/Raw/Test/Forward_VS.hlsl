#include "Include/ForwardBase.hlsli"

cbuffer PerCameraBuffer : register(b0)
{
    float3 gViewDir;
    float3 gViewOrigin;
    matrix gMatViewProj;
    matrix gMatView;
    matrix gMatProj;
    matrix gMatPrevViewProj;
    matrix gNDCToPrevNDC;
}

cbuffer PerMaterialBuffer : register(b1)
{
    float4 gAmbient;
    float4 gDiffuse;
    float4 gEmissive;
    float4 gSpecular;
    uint   gUseDiffuseMap;
    uint   gUseEmissiveMap;
    uint   gUseNormalMap;
    uint   gUseSpecularMap;
    uint   gUseBumpMap;
    uint   gUseParallaxMap;
    uint   gUseTransparencyMap;
    uint   gUseReflectionMap;
    uint   gUseOcclusionMap;
    float  gSpecularPower;
    float  gTransparency;
    float  gIndexOfRefraction;
    float  gReflection;
    float  gAbsorbance;
    float  gBumpScale;
    float  gAlphaThreshold;
};

cbuffer PerInstanceBuffer : register(b2)
{
    PerInstanceData gInstanceData[STANDARD_FORWARD_MAX_INSTANCED_BLOCK];
}

cbuffer PerObjectBuffer : register(b3)
{
    matrix gMatWorld;
    matrix gMatInvWorld;
    matrix gMatWorldNoScale;
    matrix gMatInvWorldNoScale;
    matrix gMatPrevWorld;
    uint   gLayer;
}

cbuffer PerFrameBuffer : register(b4)
{
    float gTime;
}

cbuffer PerCallBuffer : register(b5)
{
    matrix gMatWorldViewProj;
}

VS_OUTPUT main( VS_INPUT IN )
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;

    float SpecularPower = gSpecularPower;

    if(IN.Instanceid == 0)
    {
        OUT.Position.xyz = IN.Position;
        OUT.Position.w = 1.0f;
        OUT.Position = mul(OUT.Position, gMatWorld);
        OUT.Position = mul(OUT.Position, gMatViewProj);

        OUT.Color = IN.Color;
        OUT.Normal = normalize(mul(float4(IN.Normal, 0.0f), gMatWorld)).xyz;
        OUT.Tangent = normalize(mul(float4(IN.Tangent.xyz, 0.0f), gMatWorld)).xyz;
        OUT.BiTangent = normalize(mul(float4(IN.BiTangent.xyz, 0.0f), gMatWorld)).xyz;
        OUT.Texture = FlipUV(IN.Texture);

        OUT.WorldPosition.xyz = IN.Position;
        OUT.WorldPosition.w = 1.0f;
        OUT.WorldPosition = mul(OUT.WorldPosition, gMatWorld);

        OUT.ViewDirection = normalize(OUT.WorldPosition.xyz - gViewOrigin);

        OUT.WorldViewDistance = mul(OUT.WorldPosition, gMatView);
    }
    else
    {
        OUT.Position.xyz = IN.Position;
        OUT.Position.w = 1.0f;
        OUT.Position = mul(OUT.Position, gInstanceData[IN.Instanceid].gMatWorld);
        OUT.Position = mul(OUT.Position, gMatViewProj);

        OUT.Color = IN.Color;
        OUT.Normal = normalize(mul(float4(IN.Normal, 0.0f), gInstanceData[IN.Instanceid].gMatWorld)).xyz;
        OUT.Tangent = normalize(mul(float4(IN.Tangent.xyz, 0.0f), gInstanceData[IN.Instanceid].gMatWorld)).xyz;
        OUT.BiTangent = normalize(mul(float4(IN.BiTangent.xyz, 0.0f), gInstanceData[IN.Instanceid].gMatWorld)).xyz;
        OUT.Texture = FlipUV(IN.Texture);

        OUT.WorldPosition.xyz = IN.Position;
        OUT.WorldPosition.w = 1.0f;
        OUT.WorldPosition = mul(OUT.WorldPosition, gInstanceData[IN.Instanceid].gMatWorld);

        OUT.ViewDirection = normalize(OUT.WorldPosition.xyz - gViewOrigin);

        OUT.WorldViewDistance = mul(OUT.WorldPosition, gMatView);
    }

    return OUT;
}
