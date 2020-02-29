#include "TeToneMappingMat.h"
#include "TeRendererUtility.h"

namespace te
{
    ToneMappingParamDef gToneMappingParamDef;

    ToneMappingMat::ToneMappingMat()
    {
        _paramBuffer = gToneMappingParamDef.CreateBuffer();
        _params->SetParamBlockBuffer("PerFrameBuffer", _paramBuffer);
        _params->SetSamplerState("BilinearSampler", gBuiltinResources().GetBuiltinSampler(BuiltinSampler::Bilinear));
    }

    void ToneMappingMat::Execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& destination, INT32 MSSACount, float gamma, float exposure)
    {
        gToneMappingParamDef.gMSAACount.Set(_paramBuffer, MSSACount, 0);
        gToneMappingParamDef.gGamma.Set(_paramBuffer, gamma, 0);
        gToneMappingParamDef.gExposure.Set(_paramBuffer, exposure, 0);

        if (MSSACount > 1) _params->SetTexture("SourceMapMS", source);
        else _params->SetTexture("SourceMap", source);

        RenderAPI& rapi = RenderAPI::Instance();
        rapi.SetRenderTarget(destination);

        Bind();
        gRendererUtility().DrawScreenQuad();
    }
}