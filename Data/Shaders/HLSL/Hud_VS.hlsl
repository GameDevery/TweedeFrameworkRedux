#include "Include/HudBase.hlsli"

cbuffer PerCameraBuffer : register(b0)
{
    matrix gMatViewProj;
    float3 gViewOrigin;
}

cbuffer PerInstanceBuffer : register(b1)
{
    PerInstanceData gInstanceData[STANDARD_MAX_INSTANCED_BLOCK];
}

VS_OUTPUT main( VS_INPUT IN, uint Instanceid : SV_InstanceID )
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;

    OUT.Position.xyz = IN.Position;
    OUT.Position.w = (float)Instanceid;

    return OUT;
}