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
    uint   gUseEmissiveMap;
    uint   gUseNormalMap;
    uint   gUseSpecularMap;
    uint   gUseBumpMap;
    uint   gUseParallaxMap;
    uint   gUseTransparencyMap;
    uint   gUseReflectionMap;
    float  gSpecularPower;
    float  gTransparency;
    float  gIndexOfRefraction;
    float  gAbsorbance;
    float  gBumpScale;
    float  gAlphaThreshold;
};

SamplerState AnisotropicSampler : register(s0);

Texture2D DiffuseMap : register(t0);
Texture2D EmissiveMap : register(t1);
Texture2D NormalMap : register(t2);
Texture2D SpecularMap : register(t3);
Texture2D BumpMap : register(t4);
Texture2D ParallaxMap : register(t5);
Texture2D TransparencyMap : register(t6);
Texture2D ReflectionMap : register(t7);

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
    float alpha = gTransparency;

    if(gUseTransparencyMap == 1)
    {
        alpha = TransparencyMap.Sample( AnisotropicSampler, IN.Texture ).r;
    }

    if(alpha <= gAlphaThreshold)
    {
        discard;
    }

    if(gUseDiffuseMap == 1)
    {
        ambient = ambient * DiffuseMap.Sample(AnisotropicSampler, IN.Texture).rgb;
        diffuse = DiffuseMap.Sample(AnisotropicSampler, IN.Texture).rgb;
    }

    if(gUseEmissiveMap == 1)
    {
        emissive = emissive * EmissiveMap.Sample( AnisotropicSampler, IN.Texture ).rgb;
    }

    if(gUseNormalMap == 1)
    {
        float3x3 TBN = float3x3(IN.Tangent.xyz, IN.BiTangent.xyz, IN.Normal.xyz);
        normal = DoNormalMapping(TBN, NormalMap, AnisotropicSampler, IN.Texture);
    }

    if(gUseSpecularMap == 1)
    {
        specular.rgb = SpecularMap.Sample(AnisotropicSampler, IN.Texture).xyz;
    }

    if(gUseBumpMap == 1)
    {
        float3x3 TBN = float3x3(IN.Tangent.xyz, IN.BiTangent.xyz, IN.Normal.xyz);
        normal = DoBumpMapping(TBN, BumpMap, AnisotropicSampler, IN.Texture, 1.0f);
    }

    if(gUseParallaxMap == 1)
    {
        // TODO
    }

    if(gUseReflectionMap == 1)
    {
        // TODO
    }

    // Diffuse
    float  diff = max(dot(lightDirection, normal), 0.0);
    diffuse = (diff * diffuse.rgb);

    // specular
    float3 refVector = normalize(reflect(lightDirection, normal));
    float3 specFactor = pow(max(dot(IN.ViewDirection, refVector), 0.0), gSpecularPower);
    specular = (specFactor * specular.rgb);

    outColor.rgb = ambient + diffuse + specular + emissive;
    outColor.a = alpha;
    return outColor;
}
