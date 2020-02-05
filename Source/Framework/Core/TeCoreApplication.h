#pragma once

#include "TeCorePrerequisites.h"
#include "RenderAPI/TeRenderWindow.h"
#include "Utility/TeModule.h"
#include "Renderer/TeParamBlocks.h"

namespace te
{
    /**	Structure containing parameters for starting the application. */
    struct START_UP_DESC
    {
        String RenderAPI; /** Name of the render system plugin to use. */
        String Renderer; /** Name of the renderer plugin to use. */
        String Physics; /** Name of the physics plugin to use. */
        String Audio; /** Name of the audio plugin to use. */

        RENDER_WINDOW_DESC WindowDesc; /** Describes the window to create during start-up. */

        Vector<String> Importers; /** A list of importer plugins to load. */
    };

    TE_PARAM_BLOCK_BEGIN(FrameConstantBuffer)
        TE_PARAM_BLOCK_ENTRY(Vector3, gViewDir)
        TE_PARAM_BLOCK_ENTRY(Vector3, gViewOrigin)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatView)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatProj)
    TE_PARAM_BLOCK_END

    TE_PARAM_BLOCK_BEGIN(ObjectConstantBuffer)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatWorld)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatPrevWorld)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatWorldNoScale)
    TE_PARAM_BLOCK_END

    /**
     * Represents the primary entry point for the core systems. Handles start-up, shutdown, primary loop and allows you to
     * load and unload plugins.
     *
     * @note	Sim thread only.
     */
    class TE_CORE_EXPORT CoreApplication : public Module<CoreApplication>
    {
    public:
        CoreApplication(START_UP_DESC desc);
        virtual ~CoreApplication();

        TE_MODULE_STATIC_HEADER_MEMBER(CoreApplication)

        /**
         * Executes the main loop. This will update your components and modules, queue objects for rendering and run
         * the simulation. Usually called immediately after startUp().
         *
         * This will run infinitely until stopMainLoop is called (usually from another thread or internally).
         */
        void RunMainLoop();

        /**	Stops the (infinite) main loop from running. The loop will complete its current cycle before stopping. */
        void StopMainLoop();

        /**	Pause (or not) the infinite loop */
        void Pause(bool pause);

        /**	Return if the application is on pause or not */
        bool GetPaused();

        /** Changes the maximum FPS the application is allowed to run in. Zero means unlimited. */
        void SetFPSLimit(UINT32 limit);

        /** Check if the number of FPS has already been reached. */
        void CheckFPSLimit();

        /** Display frame rate on window titlebar */
        void DisplayFrameRate();

        /** Issues a request for the application to close. Application may choose to ignore the request */
        virtual void OnStopRequested();

        /** Issues a request for the application to pause. Application may choose to ignore the request */
        virtual void OnPauseRequested();

        /**	Returns the main window that was created on application start-up. */
        SPtr<RenderWindow> GetWindow() const { return _window; }

        /**	Returns startup desc. */
        const START_UP_DESC& GetStartUpDesc() const { return _startUpDesc; }

        /**
         * Loads a plugin.
         *
         * @param[in]	pluginName	Name of the plugin to load, without extension.
         * @param[out]	library		Specify as not null to receive a reference to the loaded library.
         * @param[in]	passThrough	Optional parameter that will be passed to the loadPlugin function.
         * @return					Value returned from the plugin start-up method.
         */
        void* LoadPlugin(const String& pluginName, DynLib** library = nullptr, void* passThrough = nullptr);

        /**	Unloads a previously loaded plugin. */
        void UnloadPlugin(DynLib* library);

    protected:
        /** @copydoc Module::OnStartUp */
        void OnStartUp() override;

        /** @copydoc Module::OnShutDown */
        void OnShutDown() override;

        /**	Called for each iteration of the main loop. Called before any game objects or plugins are updated. */
        virtual void PreUpdate();

        /**	Called for each iteration of the main loop. Called after all game objects and plugins are updated. */
        virtual void PostUpdate();

        void TestStartUp();
        void TestRun();
        void TestShutDown();

    protected:
        typedef void(*UpdatePluginFunc)();

        SPtr<Renderer> _renderer;
        SPtr<RenderWindow> _window;
        START_UP_DESC _startUpDesc;

        // Frame limiting
        UINT64 _frameStep = 16666; // 60 times a second in microseconds
        UINT64 _lastFrameTime = 0; // Microseconds

        DynLib* _rendererPlugin;
        DynLib* _renderAPIPlugin;
        Map<DynLib*, UpdatePluginFunc> _pluginUpdateFunctions;

        bool _isFrameRenderingFinished;

        volatile bool _runMainLoop;
        volatile bool _pause;

        SPtr<GpuProgram> _textureVertexShader;
        SPtr<GpuProgram> _texturePixelShader;

        SPtr<VertexBuffer> _vertexBuffer;
        SPtr<IndexBuffer> _indexBuffer;
        SPtr<GpuParamBlockBuffer> _objectConstantBuffer;
        SPtr<GpuParamBlockBuffer> _frameConstantBuffer;
        SPtr<VertexDeclaration> _vertexDeclaration;
        FrameConstantBuffer _defFrameBuffer;
        ObjectConstantBuffer _defObjectBuffer;

        HCamera _sceneCamera;
        HMesh _loadedMesh;
        HTexture _loadedTexture;
        HTexture _loadedCubemapTexture;

        HRenderable _renderable;
        HLight _light;
        HSkybox _skybox;

        SPtr<GpuParams> _params;

        SPtr<Pass> _pass;
        SPtr<Technique> _technique;

        HShader _shader;
        HMaterial _material;

        HSceneObject _sceneCameraSO;
        HSceneObject _sceneRenderableSO;
        HSceneObject _sceneLightSO;
        HSceneObject _sceneSkyboxSO;
    };

    /**	Provides easy access to CoreApplication. */
    TE_CORE_EXPORT CoreApplication& gCoreApplication();
    TE_CORE_EXPORT CoreApplication* gCoreApplicationPtr();
}
