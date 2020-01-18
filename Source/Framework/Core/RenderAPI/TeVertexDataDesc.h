#pragma once

#include "TeCorePrerequisites.h"
#include "RenderAPI/TeVertexDeclaration.h"

namespace te
{
	/**
	 * Contains information about layout of vertices in a buffer. This is very similar to VertexDeclaration but unlike
	 * VertexDeclaration it has no render API object to back it up and is very lightweight.
	 */
	class TE_CORE_EXPORT VertexDataDesc
	{
	public:
		VertexDataDesc() = default;
        
        /**
         * Informs the internal buffer that it needs to make room for the specified vertex element. If a vertex with same
         * stream and semantics already exists it will just be updated.
         *
         * @param[in]	type	   			Type of the vertex element. Determines size.
         * @param[in]	semantic			Semantic that allows the engine to connect the data to a shader input slot.
         * @param[in]	semanticIdx			(optional) If there are multiple semantics with the same name, use different
         *									index to differentiate between them.
         * @param[in]	streamIdx  			(optional) Zero-based index of the stream. Each stream will internally be
         *									represented as a single vertex buffer.
         * @param[in]	instanceStepRate	Determines at what rate does vertex element data advance. Zero means each vertex
         *									will advance the data pointer and receive new data (standard behaviour). Values
         *									larger than one are relevant for instanced rendering and determine how often do
         *									instances advance the vertex element (for example a value of 1 means each
         *									instance will retrieve a new value for this vertex element, a value of 2 means
         *									each second instance will, etc.).
         */
        void AddVertElem(VertexElementType type, VertexElementSemantic semantic, UINT32 semanticIdx = 0,
            UINT32 streamIdx = 0, UINT32 instanceStepRate = 0);

        /**	Query if we have vertex data for the specified semantic. */
        bool HasElement(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

        /**	Returns the size in bytes of the vertex element with the specified semantic. */
        UINT32 GetElementSize(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

        /**	Returns offset of the vertex from start of the stream in bytes. */
        UINT32 GetElementOffsetFromStream(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

        /**	Gets vertex stride in bytes (offset from one vertex to another) in the specified stream. */
        UINT32 GetVertexStride(UINT32 streamIdx) const;

        /**	Gets vertex stride in bytes (offset from one vertex to another) in all the streams. */
        UINT32 GetVertexStride() const;

        /**	Gets offset in bytes from the start of the internal buffer to the start of the specified stream. */
        UINT32 GetStreamOffset(UINT32 streamIdx) const;

        /**	Returns the number of vertex elements. */
        UINT32 GetNumElements() const { return (UINT32)_vertexElements.size(); }

        /**	Returns the vertex element at the specified index. */
        const VertexElement& GetElement(UINT32 idx) const { return _vertexElements[idx]; }

        /**	Returns the vertex element with the specified semantic. */
        const VertexElement* GetElement(VertexElementSemantic semantic, UINT32 semanticIdx = 0, UINT32 streamIdx = 0) const;

        /**	Creates a list of vertex elements from internal data. */
        Vector<VertexElement> CreateElements() const;

        /**	Creates a new empty vertex data descriptor. */
        static SPtr<VertexDataDesc> Create();

    private:
        friend class Mesh;

        /**	Returns the largest stream index of all the stored vertex elements. */
        UINT32 GetMaxStreamIdx() const;

        /**	Checks if any of the vertex elements use the specified stream index. */
        bool HasStream(UINT32 streamIdx) const;

        /**	Removes a vertex element of the specified type and semantics if it exists. */
        void ClearIfItExists(VertexElementType type, VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx);

    private:
        Vector<VertexElement> _vertexElements;
    };
}
