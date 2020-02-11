#pragma once

#include "TeCorePrerequisites.h"
#include "Utility/TeModule.h"
#include "Math/TeRect2.h"
#include "Math/TeVector2I.h"
#include "Math/TeRect2I.h"

namespace te
{
    /**
     * Contains various utility methods that make various common operations in the renderer easier.
     */
    class TE_CORE_EXPORT RendererUtility : public Module<RendererUtility>
    {
    public:
        RendererUtility();
        ~RendererUtility();

        /**
         * Activates the specified material pass for rendering. Any further draw calls will be executed using this pass.
         *
         * @param[in]	material		Material containing the pass.
         * @param[in]	passIdx			Index of the pass in the material.
         * @param[in]	techniqueIdx	Index of the technique the pass belongs to, if the material has multiple techniques.
         *
         * @note	Core thread.
         */
        void SetPass(const SPtr<Material>& material, UINT32 passIdx, UINT32 techniqueIdx);

        /**
         * Sets parameters (textures, samplers, buffers) for the currently active pass.
         *
         * @param[in]	params		        Object containing the parameters.
         * @param[in]	gpuParamsBindFlags	Specify which parameters are binded to GPU
         * @param[in]	isInstanced     	Check if current object is instanced or not (more param buffer to update)
         *					
         * @note	Core thread.
         */
        void SetPassParams(const SPtr<GpuParams> gpuParams, UINT32 gpuParamsBindFlags, bool isInstanced);

        /**
         * Draws the specified mesh.
         *
         * @param[in]	mesh			Mesh to draw.
         * @param[in]	numInstances	Number of times to draw the mesh using instanced rendering.
         *
         * @note	Core thread.
         */
        void Draw(const SPtr<Mesh>& mesh, UINT32 numInstances = 1);

        /**
         * Draws the specified mesh.
         *
         * @param[in]	mesh			Mesh to draw.
         * @param[in]	subMesh			Portion of the mesh to draw.
         * @param[in]	numInstances	Number of times to draw the mesh using instanced rendering.
         *
         * @note	Core thread.
         */
        void Draw(const SPtr<Mesh>& mesh, const SubMesh& subMesh, UINT32 numInstances = 1);

        /**
         * Draws a quad over the entire viewport in normalized device coordinates.
         *
         * @param[in]	uv				UV coordinates to assign to the corners of the quad.
         * @param[in]	textureSize		Size of the texture the UV coordinates are specified for. If the UV coordinates are
         *								already in normalized (0, 1) range then keep this value as is. If the UV coordinates
         *								are in texels then set this value to the texture size so they can be normalized
         *								internally.
         * @param[in]	numInstances	How many instances of the quad to draw (using instanced rendering). Useful when
         *								drawing to 3D textures.
         * @param[in]	flipUV			If true, vertical UV coordinate will be flipped upside down.
         */
        void DrawScreenQuad(const Rect2& uv, const Vector2I& textureSize = Vector2I(1, 1),
            UINT32 numInstances = 1, bool flipUV = false);

        /**
         * Draws a quad over the entire viewport in normalized device coordinates.
         *
         * @param[in]	numInstances	How many instances of the quad to draw (using instanced rendering). Useful when
         *								drawing to 3D textures.
         */
        void DrawScreenQuad(UINT32 numInstances = 1)
        {
            Rect2 uv(0.0f, 0.0f, 1.0f, 1.0f);
            Vector2I textureSize(1, 1);

            DrawScreenQuad(uv, textureSize, numInstances);
        }

    private:
        static constexpr UINT32 NUM_QUAD_VB_SLOTS = 1024;

        SPtr<IndexBuffer> _fullScreenQuadIB;
        SPtr<VertexBuffer> _fullScreenQuadVB;
        SPtr<VertexDataDesc> _fullscreenQuadVDesc;
        SPtr<VertexDeclaration> _fullscreenQuadVDecl;
        UINT32 _nextQuadVBSlot = 0;
    };

    /** Provides easy access to RendererUtility. */
    TE_CORE_EXPORT RendererUtility& gRendererUtility();
}