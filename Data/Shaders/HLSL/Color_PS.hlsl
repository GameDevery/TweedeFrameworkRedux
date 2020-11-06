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
    // xy - (Viewport size in pixels / 2) / Target size in pixels
    // zw - (Viewport offset in pixels + (Viewport size in pixels / 2) + Optional pixel center offset) / Target size in pixels
    float4 	 gClipToUVScaleOffset;
    float4 	 gUVToClipScaleOffset;	
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
    uint   gUseEnvironmentMap;
    float  gSpecularPower;
    float  gSpecularStrength;
    float  gTransparency;
    float  gIndexOfRefraction;
    float  gRefraction;
    float  gReflection;
    float  gAbsorbance;
    float  gBumpScale;
    float  gAlphaThreshold;
};

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

    // Diffuse
    float  diff = max(dot(lightDirection, normal), 0.0);
    diffuse = (diff * diffuse.rgb);

    // specular
    float3 refVector = normalize(reflect(lightDirection, normal));
    float3 specFactor = pow(max(dot(IN.ViewDirection, refVector), 0.0), gSpecularPower) * gSpecularStrength;
    specular = (specFactor * specular.rgb);  

    outColor.rgb = ambient + diffuse + specular + emissive;
    return outColor;
}