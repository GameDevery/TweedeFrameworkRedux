#include "Include/Common.hlsli"

cbuffer PerCameraBuffer : register(b0)
{
    float3 gViewDir;
    float3 gViewOrigin;
    matrix gMatViewProj;
    matrix gMatView;
    matrix gMatProj;
}

cbuffer PerMaterialBuffer : register(b1)
{
    float4 gAmbient;
    float4 gDiffuse;
    float4 gEmissive;
    float4 gSpecular;
    uint   gUseDiffuseMap;
    uint   gUseNormalMap;
    uint   gUseSpecularMap;
    float  gSpecularPower;
    float  gTransparency;
    float  gAbsorbance;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 BiTangent : BINORMAL;
    float4 Color : COLOR0;
    float2 Texture : TEXCOORD0;
    float3 ViewDirection: POSITION1;
};

SamplerState AnisotropicSampler : register(s0);
Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);

static const float4 LightColor = float4(1.0f, 0.9f, 0.8f, 0.6f);
static const float3 LightDirection = float3(0.75f, -2.0f, -2.0f);

float4 main( PS_INPUT IN ) : SV_Target
{
    float4 outColor  = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float3 lightDirection = normalize(-LightDirection);

    float3 ambient   = gAmbient.rgb * gAmbient.a * (LightColor.rgb * LightColor.a);
    float3 diffuse   = gDiffuse.rgb * gDiffuse.a;
    float3 emissive  = gEmissive.rgb * gEmissive.a;
    float3 specular  = gSpecular.rgb * gSpecular.a;

    float3 normal = IN.Normal;

    if(gUseDiffuseMap == 1)
    {
        ambient = ambient * DiffuseMap.Sample(AnisotropicSampler, IN.Texture).rgb;
        diffuse = DiffuseMap.Sample(AnisotropicSampler, IN.Texture).rgb;
    }

    if(gUseNormalMap == 1)
    {
        float3 bump = NormalMap.Sample(AnisotropicSampler, IN.Texture).xyz;
        bump = normalize(bump * 2.0 - 1.0);
        normal =  (bump.x * IN.Tangent) + (bump.y * IN.BiTangent) + (bump.z * IN.Normal);
        normal = normalize(normal);
    }

    if(gUseSpecularMap == 1)
    {
        specular.rgb = SpecularMap.Sample(AnisotropicSampler, IN.Texture).xyz;
    }

    // Diffuse
    float  diff = max(dot(lightDirection, normal), 0.0);
    diffuse = (diff * diffuse.rgb);

    // specular
    float3 refVector = normalize(reflect(lightDirection, normal));
    float3 specFactor = pow(max(dot(IN.ViewDirection, refVector), 0.0), gSpecularPower);
    specular = (specFactor * specular.rgb);  

    outColor.rgb = ambient + diffuse + specular + emissive;
    return outColor;
}
