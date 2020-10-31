#pragma once

#include "TeCorePrerequisites.h"
#include "TeWidget.h"
#include "../ImGuiExt/TeImGuiFileBrowser.h"

namespace te
{
    class WidgetMaterials : public Widget
    {
    public:
        WidgetMaterials();
        ~WidgetMaterials();

        virtual void Initialize() override;
        virtual void Update() override;
        virtual void UpdateBackground() override;

    protected:
        bool ShowLoadTexture();

    protected:
        SPtr<Material> _currentMaterial;
        UINT32 _materialCreationCounter;
        ImGuiFileBrowser& _fileBrowser;

        // Data used to set the correct texture while loading
        bool _loadTexture;
        String _loadTextureName;
        bool* _loadTextureUsed;
    };
}