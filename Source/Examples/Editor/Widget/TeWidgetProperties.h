#pragma once

#include "TeCorePrerequisites.h"
#include "TeWidget.h"
#include "../TeEditor.h"
#include "Scene/TeTransform.h"

namespace te
{
    class WidgetProperties : public Widget
    {
    public:
        WidgetProperties();
        ~WidgetProperties();

        virtual void Initialize() override;
        virtual void Update() override;
        virtual void UpdateBackground() override;

    protected:
        /** Shows element properties, returns true if something has been modified */
        bool ShowSceneObjectProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCCameraProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCCameraFlyerProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCCameraUIProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCLightProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCRenderableProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCScriptProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCSkyboxProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCAnimationProperties();

        /**  @copydoc WidgetProperties::ShowSceneObjectProperties */
        bool ShowCBoneProperties();

        /**  Show transform and mobility options */
        bool ShowTransform(Transform& transform, ObjectMobility& mobility, bool disableTransform = false);

        /**  Show information about a GameObject (both Component and SceneObject) */
        bool ShowGameObjectInformation(SPtr<GameObject>& gameObject);

        /** Show information about a camera */
        bool ShowCamera(SPtr<CCamera> camera);

        /** Show information about a cameraFlyer */
        bool ShowCameraFlyer(SPtr<CCameraFlyer> cameraFlyer);

        /** Show information about a cameraUI */
        bool ShowCamerUI(SPtr<CCameraUI> cameraUI);

        /** Show information about a light */
        bool ShowLight(SPtr<Light> light);

        /** Show information about renderable properties */
        bool ShowRenderableProperties(SPtr<Renderable> renderable);

        /** Show information about renderable properties */
        bool ShowRenderableSubMeshes(SPtr<Renderable> renderable);

        /** Show information about a script */
        bool ShowScript(SPtr<CScript> script);

        /** Show information about an animation */
        bool ShowAnimation(SPtr<CAnimation> animation);

        /** Show information about animation clips */
        bool ShowAnimationClips(SPtr<CAnimation> animation);

        /** Show information about a skybox */
        bool ShowSkybox(SPtr<Skybox> skybox);

        /** Show FileBrowser in order to load a new mesh */
        bool ShowLoadMesh();

        /** Show FileBrowser in order to load a new cubemap texture */
        bool ShowLoadSkybox();

    protected:
        Editor::SelectionData& _selections;
        ImGuiFileBrowser& _fileBrowser;

        bool _loadMesh = false;
        bool _loadSkybox = false;
    };
}
