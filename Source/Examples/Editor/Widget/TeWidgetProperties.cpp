#include "TeWidgetProperties.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "../TeEditorResManager.h"
#include "../TeEditorUtils.h"
#include "Resources/TeResourceManager.h"
#include "Components/TeCRenderable.h"
#include "Components/TeCCameraFlyer.h"
#include "Components/TeCCameraUI.h"
#include "Components/TeCCamera.h"
#include "Components/TeCSkybox.h"
#include "Components/TeCScript.h"
#include "Components/TeCLight.h"
#include "Components/TeCAnimation.h"
#include "Components/TeCBone.h"
#include "Scene/TeSceneManager.h"
#include "Scene/TeSceneObject.h"
#include "RenderAPI/TeSubMesh.h"
#include "Mesh/TeMesh.h"
#include "Importer/TeMeshImportOptions.h"
#include "Importer/TeTextureImportOptions.h"
#include "Resources/TeBuiltinResources.h"
#include "String/TeUnicode.h"
#include "Utility/TeFileSystem.h"
#include "Scripting/TeScriptManager.h"

namespace te
{
    WidgetProperties::WidgetProperties()
        : Widget(WidgetType::Properties)
        , _selections(gEditor().GetSelectionData())
        , _fileBrowser(gEditor().GetFileBrowser())
    { 
        _title = PROPERTIES_TITLE;
        _flags |= ImGuiWindowFlags_HorizontalScrollbar;
    }

    WidgetProperties::~WidgetProperties()
    { }

    void WidgetProperties::Initialize()
    { }

    void WidgetProperties::Update()
    {
        bool hasChanged = false;

        if (_selections.ClickedComponent)
        {
            SPtr<GameObject> gameObject = std::static_pointer_cast<GameObject>(_selections.ClickedComponent);
            if (ShowGameObjectInformation(gameObject))
                hasChanged = true;

            switch (_selections.ClickedComponent->GetCoreType())
            {
            case TID_CCamera:
            {
                if (ShowCCameraProperties())
                    hasChanged = true;
            }
            break;

            case TID_CRenderable:
            {
                if (ShowCRenderableProperties())
                    hasChanged = true;
            }
            break;

            case TID_CLight:
            {
                if (ShowCLightProperties())
                    hasChanged = true;
            }
            break;

            case TID_CSkybox:
            {
                if (ShowCSkyboxProperties())
                    hasChanged = true;
            }
            break;

            case TID_CCameraFlyer:
            {
                if (ShowCCameraFlyerProperties())
                    hasChanged = true;
            }
            break;

            case TID_CCameraUI:
            {
                if (ShowCCameraUIProperties())
                    hasChanged = true;
            }
            break;

            case TID_CScript:
            {
                if (ShowCScriptProperties())
                    hasChanged = true;
            }
            break;

            case TID_CAnimation:
            {
                if (ShowCAnimationProperties())
                    hasChanged = true;
            }
            break;

            case TID_CBone:
            {
                if (ShowCBoneProperties())
                    hasChanged = true;
            }
            break;

            default:
                break;
            }
        }
        else if (_selections.ClickedSceneObject)
        {
            if (ShowSceneObjectProperties())
                hasChanged = true;
        }

        if (hasChanged)
        {
            gEditor().NeedsRedraw();
            gEditor().GetSettings().State = Editor::EditorState::Modified;
        }
    }

    void WidgetProperties::UpdateBackground()
    { }

    bool WidgetProperties::ShowSceneObjectProperties()
    {
        bool hasChanged = false;
        ObjectMobility mobility = _selections.ClickedSceneObject->GetMobility();
        Transform transform = _selections.ClickedSceneObject->GetLocalTransform();
        SPtr<GameObject> gameObject = std::static_pointer_cast<GameObject>(_selections.ClickedSceneObject);

        if (ShowGameObjectInformation(gameObject))
            hasChanged = true;

        bool active = _selections.ClickedSceneObject->GetActive(true);
        if (ImGuiExt::RenderOptionBool(active, "##scene_object_information_active_option", "Active"))
        {
            hasChanged = true;
            _selections.ClickedSceneObject->SetActive(active);
        }

        if (ShowTransform(transform, mobility))
        {
            _selections.ClickedSceneObject->SetLocalTransform(transform);
            _selections.ClickedSceneObject->SetMobility(mobility);
            hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCCameraProperties()
    {
        bool hasChanged = false;
        SPtr<CCamera> camera = std::static_pointer_cast<CCamera>(_selections.ClickedComponent);
        ObjectMobility mobility = camera->_getCamera()->GetMobility();
        Transform transform = camera->GetSceneObject()->GetTransform();

        if (ShowTransform(transform, mobility))
        {
            camera->_getCamera()->SetMobility(mobility);
            camera->GetSceneObject()->SetLocalTransform(transform);
            hasChanged = true;
        }

        if (ImGui::CollapsingHeader("Rendering Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowCamera(camera))
                hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCCameraFlyerProperties()
    {
        bool hasChanged = false;
        SPtr<CCameraFlyer> camera = std::static_pointer_cast<CCameraFlyer>(_selections.ClickedComponent);

        if (ImGui::CollapsingHeader("Flying Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowCameraFlyer(camera))
                hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCCameraUIProperties()
    {
        bool hasChanged = false;
        SPtr<CCameraUI> camera = std::static_pointer_cast<CCameraUI>(_selections.ClickedComponent);

        if (ImGui::CollapsingHeader("Orbital Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowCamerUI(camera))
                hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCLightProperties()
    {
        bool hasChanged = false;
        SPtr<CLight> light = std::static_pointer_cast<CLight>(_selections.ClickedComponent);
        ObjectMobility mobility = light->_getLight()->GetMobility();
        Transform transform = light->GetSceneObject()->GetTransform();

        if (ShowTransform(transform, mobility))
        {
            light->_getLight()->SetMobility(mobility);
            light->GetSceneObject()->SetLocalTransform(transform);
            hasChanged = true;
        }

        if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowLight(light->_getLight()))
                hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCRenderableProperties()
    {
        bool hasChanged = false;
        SPtr<CRenderable> renderable = std::static_pointer_cast<CRenderable>(_selections.ClickedComponent);
        ObjectMobility mobility = renderable->_getInternal()->GetMobility();
        Transform transform = renderable->GetSceneObject()->GetTransform();

        if (ShowTransform(transform, mobility))
        {
            renderable->_getInternal()->SetMobility(mobility);
            renderable->GetSceneObject()->SetLocalTransform(transform);
            hasChanged = true;
        }

        if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowRenderableProperties(renderable->_getInternal()))
                hasChanged = true;
        }

        if (renderable->GetMesh())
        {
            if (ImGui::CollapsingHeader("SubMeshes", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ShowRenderableSubMeshes(renderable->_getInternal()))
                    hasChanged = true;
            }
        }
        
        return hasChanged;
    }

    bool WidgetProperties::ShowCScriptProperties()
    {
        bool hasChanged = false;
        SPtr<CScript> script = std::static_pointer_cast<CScript>(_selections.ClickedComponent);

        if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowScript(script))
                hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCAnimationProperties()
    {
        bool hasChanged = false;
        SPtr<CAnimation> animation = std::static_pointer_cast<CAnimation>(_selections.ClickedComponent);

        if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowAnimation(animation))
                hasChanged = true;
        }

        if (animation->GetNumClips() > 0)
        {
            if (ShowAnimationClips(animation))
                hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCBoneProperties()
    {
        bool hasChanged = false;
        SPtr<CBone> bone = std::static_pointer_cast<CBone>(_selections.ClickedComponent);
        ObjectMobility mobility = bone->GetSceneObject()->GetMobility();
        Transform transform = bone->GetSceneObject()->GetTransform();
        const float width = ImGui::GetWindowContentRegionWidth() - 120.0f;

        if (ShowTransform(transform, mobility))
        {
            bone->GetSceneObject()->SetMobility(mobility);
            bone->GetSceneObject()->SetLocalTransform(transform);
            hasChanged = true;
        }

        if (ImGui::CollapsingHeader("Bone", ImGuiTreeNodeFlags_DefaultOpen))
        {
            HSceneObject currentSO = bone->GetSceneObject();
            HRenderable renderable;
            HAnimation animation;

            while (!currentSO.Empty())
            {
                renderable = static_object_cast<CRenderable>(currentSO->GetComponent<CRenderable>());
                animation = static_object_cast<CAnimation>(currentSO->GetComponent<CAnimation>());

                if (!renderable.Empty() && !animation.Empty()) // If there is an animation and a renderable in the hierarchy
                    break;

                if (currentSO == gEditor().GetSceneRoot())
                    break;

                currentSO = currentSO->GetParent();           
            }

            if (!renderable.Empty() && !animation.Empty())
            {
                if (renderable->GetMesh() && renderable->GetMesh()->GetSkeleton()) // if the renderable has a skeleton
                {
                    ImGuiExt::ComboOptions<UINT32> bonesOptions;
                    SPtr<Skeleton> skeleton = renderable->GetMesh()->GetSkeleton();
                    UINT32 boneIndex = -1;

                    bonesOptions.AddOption(-1, "None");

                    for (UINT32 i = 0; i < skeleton->GetNumBones(); i++)
                    {
                        SkeletonBoneInfo boneInfo = skeleton->GetBoneInfo(i);
                        bonesOptions.AddOption(i, boneInfo.Name);

                        if (boneInfo.Name == bone->GetBoneName())
                            boneIndex = i;
                    }

                    if (ImGuiExt::RenderOptionCombo<UINT32>(&boneIndex, "##bone_name_option", "Mimicked bone", bonesOptions, width))
                    {
                        if (boneIndex != -1)
                            bone->SetBoneName(skeleton->GetBoneInfo(boneIndex).Name);
                        else
                            bone->SetBoneName("");

                        hasChanged = true;
                    }
                }
            }
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCSkyboxProperties()
    {
        bool hasChanged = false;
        SPtr<CSkybox> skybox = std::static_pointer_cast<CSkybox>(_selections.ClickedComponent);
        ObjectMobility mobility = skybox->_getSkybox()->GetMobility();
        Transform transform = skybox->_getSkybox()->GetTransform();

        if (ImGui::CollapsingHeader("Skybox", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ShowSkybox(skybox->_getSkybox()))
                hasChanged = true;
        }

        if (ShowTransform(transform, mobility, true))
        {
            skybox->_getSkybox()->SetMobility(mobility);
            hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowTransform(Transform& transform, ObjectMobility& mobility, bool disableTransform)
    {
        bool hasChanged = false;
        const float widgetWidth = ImGui::GetWindowContentRegionWidth() - 100.0f;

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static ImGuiExt::ComboOptions<int> mobilityOptions;

            if (mobilityOptions.Options.size() == 0)
            {
                mobilityOptions.AddOption((int)ObjectMobility::Movable, "Movable");
                mobilityOptions.AddOption((int)ObjectMobility::Immovable, "Immovable");
                mobilityOptions.AddOption((int)ObjectMobility::Static, "Static");
            }

            if (ImGuiExt::RenderOptionCombo<int>((int*)(&mobility), "##transform_mobility_option", "Mobility", mobilityOptions, widgetWidth, disableTransform))
                hasChanged = true;

            ImGui::Separator();

            if (ImGuiExt::RenderTransform(transform, "transform_option", disableTransform))
                hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowGameObjectInformation(SPtr<GameObject>& gameObject)
    {
        char inputName[128];
        char inputUUID[64];
        bool hasChanged = false;
        String name = gameObject->GetName();
        String uuid = gameObject->GetUUID().ToString();
        const float widgetWidth = ImGui::GetWindowContentRegionWidth() - 100.0f;

        strcpy(inputName, name.c_str());
        strcpy(inputUUID, uuid.c_str());

        if (ImGui::CollapsingHeader("Identification", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::PushItemWidth(widgetWidth);
            if (ImGui::InputText("Name", inputName, IM_ARRAYSIZE(inputName)))
            {
                gameObject->SetName(inputName);
                hasChanged = true;
            }
            ImGui::Separator();

            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::InputText("UUID", inputUUID, IM_ARRAYSIZE(inputUUID));
            ImGui::PopItemFlag();
            ImGui::PopItemWidth();
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowCamera(SPtr<CCamera> camera)
    {
        bool hasChanged = false;
        HCamera cameraHandle;
        cameraHandle._setHandleData(camera);
        auto& cameraSettings = camera->GetRenderSettings();
        const float width = ImGui::GetWindowContentRegionWidth() - 100.0f;

        if (ImGui::CollapsingHeader("Graphics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGuiExt::RenderCameraGraphics(cameraHandle, cameraSettings, width))
                hasChanged = true;
        }

        if (ImGui::CollapsingHeader("Post processing", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGuiExt::RenderCameraPostProcessing(cameraHandle, cameraSettings, width))
                hasChanged = true;
        }

        if (hasChanged)
            camera->SetRenderSettings(cameraSettings);

        return hasChanged;
    }

    bool WidgetProperties::ShowCameraFlyer(SPtr<CCameraFlyer> cameraFlyer)
    {
        bool hasChanged = false;
        Degree pitch = cameraFlyer->GetPitch();
        Degree yaw = cameraFlyer->GetYaw();
        const float width = ImGui::GetWindowContentRegionWidth() - 100.0f;

        float pitchAngle = pitch.ValueDegrees();
        float yawAngle = yaw.ValueDegrees();
        ImGuiExt::RenderOptionFloat(pitchAngle, "##camera_fliyer_pitch_option", "Current pitch", -180.0f, 180.0f, width, true);
        ImGuiExt::RenderOptionFloat(yawAngle, "##camera_fliyer_yaw_option", "Current yaw", -180.0f, 180.0f, width, true);

        return hasChanged;
    }

    bool WidgetProperties::ShowCamerUI(SPtr<CCameraUI> cameraUI)
    {
        bool hasChanged = false;
        Vector3 target = cameraUI->GetTarget();

        if (ImGuiExt::RenderVector3(target, "##camera_ui_target_option", " Target", 20.0f, false))
        {
            cameraUI->SetTarget(target);
            hasChanged = true;
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowLight(SPtr<Light> light)
    {
        bool hasChanged = false;
        int lightType = (int)light->GetType();
        bool castsShadows = light->GetCastsShadow();
        float attenuationRadius = light->GetAttenuationRadius();
        float linearAttenuation = light->GetLinearAttenuation();
        float quadraticAttenuation = light->GetQuadraticAttenuation();
        float intensity = light->GetIntensity();
        Degree spotAngle = light->GetSpotAngle();
        float shadowBias = light->GetShadowBias();
        const float width = ImGui::GetWindowContentRegionWidth() - 100.0f;

        // Color
        {
            Vector4 color = light->GetColor().GetAsVector4();
            if (ImGuiExt::RenderColorRGBA(color, "##light_color_option", "Color", width))
            {
                hasChanged = true;
                light->SetColor(Color(color));
            }
        }
        ImGui::Separator();

        // Cast shadows
        {
            if (ImGuiExt::RenderOptionBool(castsShadows, "##light_cast_shadows_option", "Cast shadows"))
            {
                hasChanged = true;
                light->SetCastsShadow(castsShadows);
            }
        }
        ImGui::Separator();

        // Light Type
        {
            static ImGuiExt::ComboOptions<int> lightTypeOptions;
            if (lightTypeOptions.Options.size() == 0)
            {
                lightTypeOptions.AddOption((int)LightType::Directional, "Directional");
                lightTypeOptions.AddOption((int)LightType::Radial, "Radial");
                lightTypeOptions.AddOption((int)LightType::Spot, "Spot");
            }

            if (ImGuiExt::RenderOptionCombo<int>((int*)(&lightType), "##light_type_option", "Type", lightTypeOptions, width, true))
            {
                light->SetType((LightType)lightType);
                hasChanged = true;
            }
        }
        ImGui::Separator();

        // Attenuation radius
        {
            if (ImGuiExt::RenderOptionFloat(attenuationRadius, "##light_attenuation_radius_option", "Att. Radius", 0.0f, 5.0f, width))
            {
                hasChanged = true;
                light->SetAttenuationRadius(attenuationRadius);
            }
        }
        ImGui::Separator();

        // Linear attenuation
        {
            if (ImGuiExt::RenderOptionFloat(linearAttenuation, "##light_linear_attenuation_option", "Linear att.", 0.0f, 1.0f, width))
            {
                hasChanged = true;
                light->SetLinearAttenuation(linearAttenuation);
            }
        }
        ImGui::Separator();

        // quadratic attenuation
        {
            if (ImGuiExt::RenderOptionFloat(quadraticAttenuation, "##light_quadratic_attenuation_option", "Quadratic att.", 0.0f, 1.0f, width))
            {
                hasChanged = true;
                light->SetQuadraticAttenuation(quadraticAttenuation);
            }
        }
        ImGui::Separator();

        // intensity
        {
            if (ImGuiExt::RenderOptionFloat(intensity, "##light_intensity_option", "Intensity", 0.0f, 16.0f, width))
            {
                hasChanged = true;
                light->SetIntensity(intensity);
            }
        }
        ImGui::Separator();

        // Spot angle
        if (light->GetType() == LightType::Spot)
        {
            {
                float angle = spotAngle.ValueDegrees();
                if (ImGuiExt::RenderOptionFloat(angle, "##light_spot_angle_option", "Spot angle", 0.0f, 179.0f, width))
                {
                    hasChanged = true;
                    light->SetSpotAngle(Degree(angle));
                }
            }
            ImGui::Separator();
        }

        // Shadow bias
        {
            if (ImGuiExt::RenderOptionFloat(shadowBias, "##light_shadow_bias_option", "Shadow bias", 0.0f, 5.0f, width))
            {
                hasChanged = true;
                light->SetShadowBias(shadowBias);
            }
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowRenderableProperties(SPtr<Renderable> renderable)
    {
        bool hasChanged = false;
        const RenderableProperties& properties = renderable->GetProperties();
        const SPtr<Mesh> mesh = renderable->GetMesh();
        const float width = ImGui::GetWindowContentRegionWidth() - 100.0f;

        ImGuiExt::ComboOptions<UUID> meshesOptions;
        UUID emptyMesh = UUID(50, 0, 0, 0);
        UUID loadMesh = UUID::EMPTY;
        UUID meshUUID = (mesh) ? mesh->GetUUID() : emptyMesh;
        EditorResManager::ResourcesContainer& container = EditorResManager::Instance().Get<Mesh>();

        // current mesh to use
        {
            for (auto& resource : container.Res)
                meshesOptions.AddOption(resource.second->GetUUID(), resource.second->GetName());

            meshesOptions.AddOption(emptyMesh, ICON_FA_TIMES_CIRCLE " No mesh");
            meshesOptions.AddOption(UUID::EMPTY, ICON_FA_FOLDER_OPEN " Load");

            if (ImGuiExt::RenderOptionCombo<UUID>(&meshUUID, "##meshes_option", "Mesh", meshesOptions, width))
            {
                if (meshUUID == loadMesh)
                {
                    _loadMesh = true;
                }
                else if (meshUUID == emptyMesh)
                {
                    renderable->SetMesh(nullptr);
                    renderable->ClearAllMaterials();
                    hasChanged = true;
                }
                else if (meshUUID != ((mesh) ? mesh->GetUUID() : emptyMesh))
                {
                    renderable->SetMesh(gResourceManager().Load<Mesh>(meshUUID).GetInternalPtr());
                    renderable->ClearAllMaterials();
                    hasChanged = true;

                    // We check if a material exists on each subMesh. In this case, we apply the material on the renderable
                    for (UINT32 i = 0; i < renderable->GetMesh()->GetProperties().GetNumSubMeshes(); i++)
                    {
                        SubMesh& subMesh = renderable->GetMesh()->GetProperties().GetSubMesh(i);
                        if (subMesh.Mat.GetHandleData())
                            renderable->SetMaterial(i, subMesh.Mat.GetInternalPtr());
                    }
                }
            }

            if (ShowLoadMesh())
                hasChanged = true;
        }
        ImGui::Separator();

        // velocity
        {
            bool writeVelocity = properties.WriteVelocity;
            if (ImGuiExt::RenderOptionBool(writeVelocity, "##renderable_properties_velocity_option", "Write velocity"))
            {
                hasChanged = true;
                renderable->SetWriteVelocity(writeVelocity);
            }
        }
        ImGui::Separator();

        // instancing
        {
            bool instancing = properties.Instancing;
            if (ImGuiExt::RenderOptionBool(instancing, "##renderable_properties_instancing_option", "Instancing"))
            {
                hasChanged = true;
                renderable->SetInstancing(instancing);
            }
        }
        ImGui::Separator();

        // can be merged
        {
            bool canBeMerged = properties.CanBeMerged;
            if (ImGuiExt::RenderOptionBool(canBeMerged, "##renderable_properties_can_be_merged_option", "Merging"))
            {
                hasChanged = true;
                renderable->SetCanBeMerged(canBeMerged);
            }
        }
        ImGui::Separator();

        // cast shadows
        {
            bool castShadow = properties.CastShadow;
            if (ImGuiExt::RenderOptionBool(castShadow, "##renderable_properties_cast_shadow_option", "Cast shadows"))
            {
                hasChanged = true;
                renderable->SetCastShadow(castShadow);
            }
        }
        ImGui::Separator();

        // cast lights
        {
            bool castLight = properties.CastLight;
            if (ImGuiExt::RenderOptionBool(castLight, "##renderable_properties_cast_light_option", "Cast lights"))
            {
                hasChanged = true;
                renderable->SetCastLight(castLight);
            }
        }
        ImGui::Separator();

        // use for dynamic env mapping
        {
            bool useForDynamicEnvMapping = properties.UseForDynamicEnvMapping;
            if (ImGuiExt::RenderOptionBool(useForDynamicEnvMapping, "##renderable_properties_dynamic_env_mapping_option", "Dynamic env mapping"))
            {
                hasChanged = true;
                renderable->SetUseForDynamicEnvMapping(useForDynamicEnvMapping);
            }
        }
        ImGui::Separator();

        // cull distance factor
        {
            float cullDistanceFactor = properties.CullDistanceFactor;
            if (ImGuiExt::RenderOptionFloat(cullDistanceFactor, "##renderable_properties_dynamic_env_mapping_option", "Cull dist. factor", 0.0f, 1.0f, width - 20.0f))
            {
                hasChanged = true;
                renderable->SetCullDistanceFactor(cullDistanceFactor);
            }
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowRenderableSubMeshes(SPtr<Renderable> renderable)
    {
        bool hasChanged = false;
        SPtr<Mesh> mesh = renderable->GetMesh();
        ImGuiExt::ComboOptions<UUID> materialsOptions;
        UUID emptyMaterial = UUID(50, 0, 0, 0);
        MeshProperties& meshProperties = mesh->GetProperties();
        const float width = ImGui::GetWindowContentRegionWidth() - 120.0f;
        EditorResManager::ResourcesContainer& container = EditorResManager::Instance().Get<Material>();

        for (auto& resource : container.Res)
            materialsOptions.AddOption(resource.second->GetUUID(), resource.second->GetName());

        materialsOptions.AddOption(emptyMaterial, ICON_FA_TIMES_CIRCLE " No material");

        for (UINT32 i = 0; i < meshProperties.GetNumSubMeshes(); i++)
        {
            SPtr<Material> material = renderable->GetMaterial(i);
            UUID materialUUID = (material) ? material->GetUUID() : emptyMaterial;
            SubMesh& subMesh = meshProperties.GetSubMesh(i);
            String title = subMesh.MaterialName;
            String id = "##" + subMesh.MaterialName + ToString(i);

            // current material to use
            {
                if (ImGuiExt::RenderOptionCombo<UUID>(&materialUUID, id.c_str(), title.c_str(), materialsOptions, width))
                {
                    if (materialUUID == emptyMaterial)
                    {
                        renderable->SetMaterial(i, nullptr);
                        hasChanged = true;
                    }
                    else
                    {
                        renderable->SetMaterial(i, gResourceManager().Load<Material>(materialUUID).GetInternalPtr());
                        hasChanged = true;
                    }
                }
            }  
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowScript(SPtr<CScript> script)
    {
        bool hasChanged = false;
        Vector<String> files;
        Vector<String> directories;
        ImGuiExt::ComboOptions<String> scriptsOptions;
        const float width = ImGui::GetWindowContentRegionWidth() - 50.0f;

        FileSystem::GetChildren(RAW_APP_ROOT + ScriptManager::LIBRARIES_PATH, files, directories, true);

        // current script to use
        {
            for (auto& file : files)
            {
                if (RegexMatch(file, "^(.*)(\\.cpp)$"))
                {
                    String fileName = ReplaceAll(file, ".cpp", "");
                    scriptsOptions.AddOption(fileName, fileName);
                }
            }

            scriptsOptions.AddOption(String(), ICON_FA_TIMES_CIRCLE " No script");

            String currentScript = script->GetNativeScriptName();
            String previousScript = script->GetNativeScriptName();
            if (ImGuiExt::RenderOptionCombo<String>(&currentScript, "##scripts_options", "Script", scriptsOptions, width))
            {
                if (currentScript != previousScript)
                {
                    script->SetNativeScript(currentScript);
                    hasChanged = true;
                }
            }
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowAnimation(SPtr<CAnimation> animation)
    {
        bool hasChanged = false;
        const float width = ImGui::GetWindowContentRegionWidth() - 100.0f;

        // preview mode
        /*{
            bool previewMode = animation->GetPreviewMode();
            bool oldPreviewMode = previewMode;
            if (ImGuiExt::RenderOptionBool(previewMode, "##animation_preview_mode_option", "Preview"))
            {
                if (previewMode != oldPreviewMode)
                {
                    hasChanged = true;
                    animation->_togglePreviewMode(true);
                }
            }
        }*/

        // cull
        {
            bool cull = animation->GetEnableCull();
            if (ImGuiExt::RenderOptionBool(cull, "##animation_cull_option", "Culling"))
            {
                hasChanged = true;
                animation->SetEnableCull(cull);
            }
        }
        ImGui::Separator();

        // speed
        {
            float speed = animation->GetSpeed();
            if (ImGuiExt::RenderOptionFloat(speed, "##animation_speed_option", "Speed", 0.1f, 10.0f, width))
            {
                hasChanged = true;
                animation->SetSpeed(speed);
            }
        }
        ImGui::Separator();

        // wrap mode
        {
            static ImGuiExt::ComboOptions<AnimWrapMode> wrapModeOptions;
            if (wrapModeOptions.Options.size() == 0)
            {
                wrapModeOptions.AddOption(AnimWrapMode::Loop, "Loop");
                wrapModeOptions.AddOption(AnimWrapMode::Clamp, "Clamp");
            }

            AnimWrapMode wrapMode = animation->GetWrapMode();
            if (ImGuiExt::RenderOptionCombo<AnimWrapMode>(&wrapMode, "##animation_wrap_mode_option", "Wrap mode", wrapModeOptions, width))
            {
                hasChanged = true;
                animation->SetWrapMode(wrapMode);
            }
        }
        ImGui::Separator();

        // bounds
        {
            AABox bounds = animation->GetBounds();
            Vector3 minBound = bounds.GetMin();
            Vector3 maxBound = bounds.GetMax();

            if (ImGuiExt::RenderVector3(minBound, "##animation_min_bound_option", " Min bound", 40.0f))
            {
                bounds.SetMin(minBound);
                animation->SetBounds(bounds);
                hasChanged = true;
            }

            if (ImGuiExt::RenderVector3(maxBound, "##animation_max_bound_option", " Max bound", 40.0f))
            {
                bounds.SetMax(maxBound);
                animation->SetBounds(bounds);
                hasChanged = true;
            }
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowAnimationClips(SPtr<CAnimation> animation)
    {
        bool hasChanged = false;
        char inputName[128];
        UINT32 numClip = animation->GetNumClips();
        const float widgetWidth = ImGui::GetWindowContentRegionWidth() - 100.0f;

        for (UINT32 i = 0; i < numClip; i++)
        {
            HAnimationClip clip = animation->GetClip(i);
            String name = clip->GetName();
            
            ImGui::PushID((int)clip->GetInternalID());
            if (ImGui::CollapsingHeader(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                memset(&inputName, 0, 128);
                strcpy(inputName, name.c_str());
   
                ImGui::PushItemWidth(widgetWidth);
                if (ImGui::InputText("Name", inputName, IM_ARRAYSIZE(inputName)))
                {
                    clip->SetName(inputName);
                    hasChanged = true;
                }
                ImGui::PopItemWidth();
            }
            ImGui::PopID();
        }

        return hasChanged;
    }

    bool WidgetProperties::ShowSkybox(SPtr<Skybox> skybox)
    {
        bool hasChanged = false;
        float brigtness = skybox->GetBrightness();
        const float width = ImGui::GetWindowContentRegionWidth() - 100.0f;

        ImGuiExt::ComboOptions<UUID> texturesOptions;
        UUID emptyTexture = UUID(50, 0, 0, 0);
        UUID loadTexture = UUID::EMPTY;
        UUID textureUUID = (skybox->GetTexture()) ? skybox->GetTexture()->GetUUID() : emptyTexture;
        EditorResManager::ResourcesContainer& container = EditorResManager::Instance().Get<Texture>();

        // current texture to use
        {
            for (auto& resource : container.Res)
            {
                SPtr<Texture> texture = std::static_pointer_cast<Texture>(resource.second.GetInternalPtr());
                if(texture->GetProperties().GetTextureType() == TextureType::TEX_TYPE_CUBE_MAP)
                    texturesOptions.AddOption(resource.second->GetUUID(), resource.second->GetName());
            }

            texturesOptions.AddOption(emptyTexture, ICON_FA_TIMES_CIRCLE " No texture");
            texturesOptions.AddOption(UUID::EMPTY, ICON_FA_FOLDER_OPEN " Load");

            if (ImGuiExt::RenderOptionCombo<UUID>(&textureUUID, "##skybox_texture_option", "Texture", texturesOptions, width))
            {
                if (textureUUID == loadTexture)
                {
                    _loadSkybox = true;
                }
                else if (textureUUID == emptyTexture)
                {
                    skybox->SetTexture(nullptr);
                    hasChanged = true;
                }
                else
                {
                    skybox->SetTexture(gResourceManager().Load<Texture>(textureUUID).GetInternalPtr());
                    hasChanged = true;
                }
            }
        }
        ImGui::Separator();

        if (ImGuiExt::RenderOptionFloat(brigtness, "##skybox_option", "Brightness", 0.0f, 3.0f, width))
        {
            skybox->SetBrightness(brigtness);
            hasChanged = true;
        }

        if (ShowLoadSkybox())
            hasChanged = true;

        return hasChanged;
    }

    bool WidgetProperties::ShowLoadMesh()
    {
        bool meshLoaded = false;

        if (_loadMesh)
            ImGui::OpenPopup("Load Mesh");

        if (_fileBrowser.ShowFileDialog("Load Mesh", ImGuiFileBrowser::DialogMode::OPEN, ImVec2(800, 450), true, ".obj,.dae,.fbx"))
        {
            auto meshImportOptions = MeshImportOptions::Create();
            meshImportOptions->ImportNormals = _fileBrowser.Data.MeshParam.ImportNormals;
            meshImportOptions->ImportTangents = _fileBrowser.Data.MeshParam.ImportTangents;
            meshImportOptions->ImportSkin = _fileBrowser.Data.MeshParam.ImportSkin;
            meshImportOptions->ImportBlendShapes = _fileBrowser.Data.MeshParam.ImportBlendShapes;
            meshImportOptions->ImportAnimation = _fileBrowser.Data.MeshParam.ImportAnimation;
            meshImportOptions->ReduceKeyFrames = _fileBrowser.Data.MeshParam.ReduceKeyFrames;
            meshImportOptions->ImportMaterials = _fileBrowser.Data.MeshParam.ImportMaterials;
            meshImportOptions->CpuCached = false;

            SPtr<MultiResource> resources = EditorResManager::Instance().LoadAll(_fileBrowser.Data.SelectedPath, meshImportOptions);
            if (!resources->Empty())
            {
                for (auto& subRes : resources->Entries)
                {
                    if (subRes.Name == "primary")
                    {
                        HMesh mesh = static_resource_cast<Mesh>(subRes.Res);
                        if (mesh.GetHandleData())
                        {
                            mesh->SetName(UTF8::FromANSI(_fileBrowser.Data.SelectedFileName));
                            EditorResManager::Instance().Add<Mesh>(mesh);
                            SPtr<CRenderable> renderable = std::static_pointer_cast<CRenderable>(_selections.ClickedComponent);

                            // We will try to set the material attach to this mesh (in fact one material per submesh), and create it before if not exist
                            renderable->SetMesh(mesh.GetInternalPtr());
                            meshLoaded = true;

                            if (meshImportOptions->ImportMaterials)
                            {
                                EditorUtils::ImportMeshMaterials(mesh);

                                for (UINT32 i = 0; i < mesh->GetProperties().GetNumSubMeshes(); i++)
                                {
                                    SubMesh& subMesh = mesh->GetProperties().GetSubMesh(i);
                                    if (subMesh.Mat.GetHandleData())
                                        renderable->SetMaterial(i, subMesh.Mat.GetInternalPtr());
                                }
                            }
                        }

                        _loadMesh = false;
                    }
                    else
                    {
                        subRes.Res->SetPath(UTF8::FromANSI(_fileBrowser.Data.SelectedFileName));
                    }
                }
            }
        }
        else
        {
            if (_fileBrowser.Data.IsCancelled)
                _loadMesh = false;
        }

        return meshLoaded;
    }

    bool WidgetProperties::ShowLoadSkybox()
    {
        bool textureLoaded = false;

        if (_loadSkybox)
            ImGui::OpenPopup("Load Skybox Texture");

        if (_fileBrowser.ShowFileDialog("Load Skybox Texture", ImGuiFileBrowser::DialogMode::OPEN, ImVec2(800, 450), false, ".png,.jpeg,.jpg"))
        {
            auto textureSkyboxImportOptions = TextureImportOptions::Create();
            textureSkyboxImportOptions->CpuCached = false;
            textureSkyboxImportOptions->CubemapType = CubemapSourceType::Faces;
            textureSkyboxImportOptions->IsCubemap = true;
            textureSkyboxImportOptions->Format = Util::IsBigEndian() ? PF_RGBA8 : PF_BGRA8;

            HTexture texture = EditorResManager::Instance().Load<Texture>(_fileBrowser.Data.SelectedPath, textureSkyboxImportOptions);
            if (texture.GetHandleData())
            {
                texture->SetName(UTF8::FromANSI(_fileBrowser.Data.SelectedFileName));
                EditorResManager::Instance().Add<Texture>(texture);
                SPtr<CSkybox> skybox = std::static_pointer_cast<CSkybox>(_selections.ClickedComponent);
                skybox->SetTexture(texture.GetInternalPtr());
                textureLoaded = true;
            }

            _loadSkybox = false;
        }
        else
        {
            if (_fileBrowser.Data.IsCancelled)
                _loadSkybox = false;
        }

        return textureLoaded;
    }
}
