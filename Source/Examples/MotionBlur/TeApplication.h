#pragma once

#include "TeCorePrerequisites.h"
#include "TeCoreApplication.h"

namespace te
{
    /**
     * Represents the primary entry point for the core systems. Handles start-up, shutdown, primary loop and allows you to
     * load and unload plugins.
     */
    class Application : public CoreApplication
    {
    public:
        Application(START_UP_DESC desc) : CoreApplication(desc) {}
        virtual ~Application() = default;

        TE_MODULE_STATIC_HEADER_MEMBER(Application)

        /** Starts the framework. If using a custom Application system, provide it as a template parameter. */
        template<class T = Application>
        static void StartUp(const START_UP_DESC& desc)
        {
            CoreApplication::StartUp<T>(desc);
        }

    protected:
        /** @copydoc CoreApplication::PostStartUp */
        void PostStartUp() override;

        /** @copydoc CoreApplication::PreShutDown */
        void PreShutDown() override;

        /** @copydoc CoreApplication::PreUpdate */
        void PreUpdate() override;

        /** @copydoc CoreApplication::PreUpdate */
        void PostUpdate() override;

    protected:
#if TE_PLATFORM == TE_PLATFORM_WIN32
        HCamera _sceneCamera;
        HCameraFlyer _sceneCameraFlyer;

        HMesh _loadedMeshCube;
        HMesh _loadedMeshMill;
        HTexture _loadedTextureCube;
        HMaterial _materialCube;
        HMaterial _materialMill;

        HSceneObject _sceneCameraSO;
        HSceneObject _sceneCubeSO;
        HSceneObject _sceneMillSO;
        HSceneObject _sceneLightSO;

        HRenderable _renderableCube;
        HRenderable _renderableMill;
        HLight _light;
        HSkybox _skybox;

        enum class CubeDirection
        {
            GoLeft, GoRight
        };

        CubeDirection _direction = CubeDirection::GoLeft;
#endif
    };
}
