#pragma once

#include "TeCorePrerequisites.h"
#include "Renderer/TeRendererMeshData.h"

namespace te
{
    struct EvaluatedAnimationData;

    /** Contains various data evaluated by external systems on a per-frame basis that is to be used by the renderer. */
    struct PerFrameData
    {
        const EvaluatedAnimationData* Animation = nullptr;
    };

    /**	Set of options that can be used for controlling the renderer. */
    struct TE_CORE_EXPORT RendererOptions
    {
        virtual ~RendererOptions() = default;
    };

    /**
     * Contains information about the current state of a particular renderer view. This will be updated
     * during rendering of a single frame.
     */
    struct RendererViewContext
    {
        /** Current target the view is rendering to. */
        SPtr<RenderTarget> CurrentTarget;
    };

    class TE_CORE_EXPORT Renderer
    {
    public:
        Renderer();
        virtual ~Renderer() = default;

        /** Initializes the renderer. Must be called before using the renderer. */
        virtual void Initialize() {};

        /**	Cleans up the renderer. Must be called before the renderer is deleted. */
        virtual void Destroy() {}

        /** Called every frame. Triggers render task callbacks. */
        virtual void Update() = 0;

        /** Name of the renderer. Used by materials to find an appropriate technique for this renderer. */
        virtual const String& GetName() const = 0;

        /** Called in order to render all currently active cameras. */
        virtual void RenderAll(PerFrameData& perFrameData) = 0;

        /**	Sets options used for controlling the rendering. */
        virtual void SetOptions(const SPtr<RendererOptions>& options) { }

        /**	Returns current set of options used for controlling the rendering. */
        virtual SPtr<RendererOptions> GetOptions() const { return SPtr<RendererOptions>(); }

        /** Called whenever a new camera is created. */
        virtual void NotifyCameraAdded(Camera* camera) { }

        /**
         * Called whenever a camera's position or rotation is updated.
         *
         * @param[in]	camera		Camera that was updated.
         */
        virtual void NotifyCameraUpdated(Camera* camera, UINT32 updateFlag) { }

        /** Called whenever a camera is destroyed. */
        virtual void NotifyCameraRemoved(Camera* camera) { }

        /**
         * Called whenever a new renderable is created.
         */
        virtual void NotifyRenderableAdded(Renderable* renderable) { }

        /**
         * Called whenever a renderable is updated.
         */
        virtual void NotifyRenderableUpdated(Renderable* renderable) { }

        /**
         * Called whenever a renderable is destroyed.
         */
        virtual void NotifyRenderableRemoved(Renderable* renderable) { }

        /**
         * Called whenever a new light is created.
         */
        virtual void NotifyLightAdded(Light* light) { }

        /**
         * Called whenever a light is updated.
         */
        virtual void NotifyLightUpdated(Light* light) { }

        /**
         * Called whenever a light is destroyed.
         */
        virtual void NotifyLightRemoved(Light* light) { }

        /**
         * Called whenever a skybox is created.
         */
        virtual void NotifySkyboxAdded(Skybox* skybox) { }

        /**
         * Called whenever a skybox is destroyed.
         */
        virtual void NotifySkyboxRemoved(Skybox* skybox) { }

        /**
         * Call by the user when he went to batch several renderables into only one big renderable.
         */
        virtual void BatchRenderables() { }

        /**
         * Creates a new empty renderer mesh data.
         *
         * @see RendererMeshData
         */
        virtual SPtr<RendererMeshData> _createMeshData(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);

        /**
         * Creates a new renderer mesh data using an existing generic mesh data buffer.

         * @see RendererMeshData
         */
        virtual SPtr<RendererMeshData> _createMeshData(const SPtr<MeshData>& meshData);
    };

    /**	Provides easy access to Renderer. */
    SPtr<Renderer> TE_CORE_EXPORT gRenderer();
}
