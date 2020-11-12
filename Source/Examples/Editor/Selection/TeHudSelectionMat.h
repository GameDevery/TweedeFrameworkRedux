#pragma once

#include "TeCorePrerequisites.h"
#include "TeSelectionUtils.h"
#include "Renderer/TeRendererMaterial.h"

namespace te
{
    /** Shader that performs Hud billboards selection. */
    class HudSelectionMat : public RendererMaterial<HudSelectionMat>
    {
        RMAT_DEF(BuiltinShader::HudSelection);

    public:
        using InstanceIter = Vector<EditorUtils::PerHudInstanceData>::iterator;

    public:
        HudSelectionMat();

        /** Set gpu params for camera */
        void BindCamera(const HCamera& camera, SelectionRenderType renderType);

        /** A subset of all Hud elements we want to draw. We can't render more than 32 hud in a raw */
        void BindHud(const InstanceIter& begin, const InstanceIter& end);

    private:
        HTexture _hudMask;

        PerPickSelectFrameParamDef _perFrameParamDef;
        PerHudInstanceParamDef _perInstanceParamDef;

        SPtr<GpuParamBlockBuffer> _perFrameParamBuffer;
        SPtr<GpuParamBlockBuffer> _perInstanceParamBuffer;
    };
}