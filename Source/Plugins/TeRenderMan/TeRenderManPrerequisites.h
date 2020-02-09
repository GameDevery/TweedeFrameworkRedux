#pragma once

#include "Prerequisites/TePrerequisitesUtility.h"
#include "Renderer/TeParamBlocks.h"
#include "Math/TeMatrix4.h"

#define STANDARD_FORWARD_MAX_INSTANCED_BLOCK_SIZE 128
#define STANDARD_FORWARD_MIN_INSTANCED_BLOCK_SIZE 16
#define STANDARD_FORWARD_MAX_INSTANCED_BLOCKS_NUMBER 128

#define STANDARD_FORWARD_MAX_VERTICES_COMBINED_MESH 4096

namespace te
{
    struct PerInstanceData
    {
        Matrix4 gMatWorld;
        Matrix4 gMatInvWorld;
        Matrix4 gMatWorldNoScale;
        Matrix4 gMatInvWorldNoScale;
        Matrix4 gMatPrevWorld;
        INT32   gLayer;
    };

    TE_PARAM_BLOCK_BEGIN(PerCameraParamDef)
        TE_PARAM_BLOCK_ENTRY(Vector3, gViewDir)
        TE_PARAM_BLOCK_ENTRY(Vector3, gViewOrigin)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatView)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatProj)
    TE_PARAM_BLOCK_END

    extern PerCameraParamDef gPerCameraParamDef;

    TE_PARAM_BLOCK_BEGIN(SkyboxParamDef)
        TE_PARAM_BLOCK_ENTRY(Color, gClearColor)
    TE_PARAM_BLOCK_END

    extern SkyboxParamDef gSkyboxParamDef;

    TE_PARAM_BLOCK_BEGIN(PerFrameParamDef)
        TE_PARAM_BLOCK_ENTRY(float, gTime)
    TE_PARAM_BLOCK_END

    extern PerFrameParamDef gPerFrameParamDef;

    TE_PARAM_BLOCK_BEGIN(PerObjectParamDef)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatWorld)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatInvWorld)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatWorldNoScale)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatInvWorldNoScale)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatPrevWorld)
        TE_PARAM_BLOCK_ENTRY(INT32, gLayer)
    TE_PARAM_BLOCK_END

    extern PerObjectParamDef gPerObjectParamDef;

    TE_PARAM_BLOCK_BEGIN(PerCallParamDef)
        TE_PARAM_BLOCK_ENTRY(Matrix4, gMatWorldViewProj)
    TE_PARAM_BLOCK_END

    extern PerCallParamDef gPerCallParamDef;

    TE_PARAM_BLOCK_BEGIN(PerInstanceParamDef)
        TE_PARAM_BLOCK_ENTRY_ARRAY(PerInstanceData, gInstances, STANDARD_FORWARD_MAX_INSTANCED_BLOCK_SIZE)
    TE_PARAM_BLOCK_END

    extern PerInstanceParamDef gPerInstanceParamDef;
    extern SPtr<GpuParamBlockBuffer> gPerInstanceParamBuffer[STANDARD_FORWARD_MAX_INSTANCED_BLOCKS_NUMBER];

    enum class RenderManCulling
    {
        Frustum = 1 << 0,
        Occlusion = 1 << 1
    };

    /** Instancing method for RenderMan */
    enum class RenderManInstancing
    {
        Automatic, /**< We loop each frame through all objects to find those who can be instanced. (CPU cost) */
        Manual, /**< User must set on its own the instancing property for each object */
        None /**< No instancing is used */
    };

    /** Texture filtering options for RenderMan. */
    enum class RenderManFiltering
    {
        Bilinear, /**< Sample linearly in X and Y directions within a texture mip level. */
        Trilinear, /**< Sample bilinearly and also between texture mip levels to hide the mip transitions. */
        Anisotropic /**< High quality dynamic filtering that improves quality of angled surfaces */
    };

    /** Available implementation of the RenderElement class. */
    enum class RenderElementType
    {
        /** See RenderableElement. */
        Renderable,
        /** See ParticlesRenderElement. */
        Particle,
        /** See DecalRenderElement. */
        Decal
    };

    /** Types of ways for shaders to handle MSAA. */
    enum class MSAAMode
    {
        /** No MSAA supported. */
        None,
        /** Single MSAA sample will be resolved. */
        Single,
        /** All MSAA samples will be resolved. */
        Full,
    };

    /** State used to controlling how are properties that need to maintain their previous frame state updated. */
    enum class PrevFrameDirtyState
    {
        /** Most recent version of the property was updated this frame, and its old data stored as prev. version. */
        Updated,
        /** No update has been done this frame, most recent version of the properties should be copied into prev. frame. */
        CopyMostRecent,
        /** Most recent and prev. frame versions are the same and require no updates. */
        Clean
    };

    /** Information about current time and frame index. */
    struct FrameTimings
    {
        float Time = 0.0f;
        float TimeDelta = 0.0f;
        UINT64 FrameIdx = 0;
    };

    struct RenderManOptions;
    class RenderMan;
    class RendererScene;
    class RendererView;
    struct LightData;
    class RendererLight;
    class RenderableElement;
    struct RendererRenderable;
}
