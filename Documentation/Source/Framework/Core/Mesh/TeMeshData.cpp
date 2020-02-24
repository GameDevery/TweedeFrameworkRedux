#include "Mesh/TeMeshData.h"
#include "Math/TeVector2.h"
#include "Math/TeVector3.h"
#include "Math/TeSphere.h"
#include "Math/TeAABox.h"
#include "RenderAPI/TeHardwareBufferManager.h"
#include "RenderAPI/TeVertexDeclaration.h"
#include "RenderAPI/TeVertexDataDesc.h"

namespace te
{
    MeshData::MeshData(UINT32 numVertices, UINT32 numIndexes, const SPtr<VertexDataDesc>& vertexData, IndexType indexType)
        : _numVertices(numVertices)
        , _numIndices(numIndexes)
        , _indexType(indexType)
        , _vertexData(vertexData)
    {
        AllocateInternalBuffer();
    }

    MeshData::~MeshData()
    { }

    UINT32 MeshData::GetNumIndices() const
    {
        return _numIndices;
    }

    UINT16* MeshData::GetIndices16() const
    {
        if(_indexType != IT_16BIT)
        {
            TE_ASSERT_ERROR(false, "Attempting to get 16bit index buffer, but internally allocated buffer is 32 bit.", __FILE__, __LINE__);
        }

        UINT32 indexBufferOffset = GetIndexBufferOffset();

        return (UINT16*)(GetData() + indexBufferOffset);
    }

    UINT32* MeshData::GetIndices32() const
    {
        if(_indexType != IT_32BIT)
            TE_ASSERT_ERROR(false, "Attempting to get 32bit index buffer, but internally allocated buffer is 16 bit.", __FILE__, __LINE__);

        UINT32 indexBufferOffset = GetIndexBufferOffset();

        return (UINT32*)(GetData() + indexBufferOffset);
    }

    UINT32 MeshData::GetInternalBufferSize() const
    {
        return GetIndexBufferSize() + GetStreamSize();
    }

    SPtr<MeshData> MeshData::Combine(const Vector<SPtr<MeshData>>& meshes, const Vector<Vector<SubMesh>>& allSubMeshes,
        Vector<SubMesh>& subMeshes)
    {
        UINT32 totalVertexCount = 0;
        UINT32 totalIndexCount = 0;
        for(auto& meshData : meshes)
        {
            totalVertexCount += meshData->GetNumVertices();
            totalIndexCount += meshData->GetNumIndices();
        }

        SPtr<VertexDataDesc> vertexData = te_shared_ptr_new<VertexDataDesc>();
        
        Vector<VertexElement> combinedVertexElements;
        for(auto& meshData : meshes)
        {
            for(UINT32 i = 0; i < meshData->GetVertexDesc()->GetNumElements(); i++)
            {
                const VertexElement& newElement = meshData->GetVertexDesc()->GetElement(i);

                INT32 alreadyExistsIdx = -1;
                UINT32 idx = 0;

                for(auto& existingElement : combinedVertexElements)
                {
                    if(newElement.GetSemantic() == existingElement.GetSemantic() && newElement.GetSemanticIdx() == existingElement.GetSemanticIdx()
                        && newElement.GetStreamIdx() == existingElement.GetStreamIdx())
                    {
                        if(newElement.GetType() != existingElement.GetType())
                        {
                            TE_ASSERT_ERROR(false, "Two elements have same semantics but different types. This is not supported.", __FILE__, __LINE__);
                        }

                        alreadyExistsIdx = idx;
                        break;
                    }

                    idx++;
                }

                if(alreadyExistsIdx == -1)
                {
                    combinedVertexElements.push_back(newElement);
                    vertexData->AddVertElem(newElement.GetType(), newElement.GetSemantic(), newElement.GetSemanticIdx(), newElement.GetStreamIdx());
                }
            }
        }

        SPtr<MeshData> combinedMeshData = te_shared_ptr_new<MeshData>(totalVertexCount, totalIndexCount, vertexData);

        // Copy indices
        UINT32 vertexOffset = 0;
        UINT32* idxPtr = combinedMeshData->GetIndices32();
        for(auto& meshData : meshes)
        {
            UINT32 numIndices = meshData->GetNumIndices();
            UINT32* srcData = meshData->GetIndices32();

            for(UINT32 j = 0; j < numIndices; j++)
                idxPtr[j] = srcData[j] + vertexOffset;

            idxPtr += numIndices;
            vertexOffset += meshData->GetNumVertices();
        }

        // Copy sub-meshes
        size_t meshCount = meshes.size();
        for(size_t meshIdx = 0; meshIdx < meshCount; meshIdx++)
        {
            const Vector<SubMesh> curSubMeshes = allSubMeshes[meshIdx];
            for(auto& subMesh : curSubMeshes)
            {
                subMeshes.push_back(SubMesh(subMesh.IndexOffset, subMesh.IndexCount, subMesh.DrawOp, subMesh.MaterialName));
            }
        }

        // Copy vertices
        vertexOffset = 0;
        for(auto& meshData : meshes)
        {
            for(auto& element : combinedVertexElements)
            {
                UINT32 dstVertexStride = vertexData->GetVertexStride(element.GetStreamIdx());
                UINT8* dstData = combinedMeshData->GetElementData(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx());
                dstData += vertexOffset * dstVertexStride;

                UINT32 numSrcVertices = meshData->GetNumVertices();
                UINT32 vertexSize = vertexData->GetElementSize(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx());

                if(meshData->GetVertexDesc()->HasElement(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx()))
                {
                    UINT32 srcVertexStride = meshData->GetVertexDesc()->GetVertexStride(element.GetStreamIdx());
                    UINT8* srcData = meshData->GetElementData(element.GetSemantic(), element.GetSemanticIdx(), element.GetStreamIdx());

                    for(UINT32 i = 0; i < numSrcVertices; i++)
                    {
                        memcpy(dstData, srcData, vertexSize);
                        dstData += dstVertexStride;
                        srcData += srcVertexStride;
                    }
                }
                else
                {
                    for(UINT32 i = 0; i < numSrcVertices; i++)
                    {
                        memset(dstData, 0, vertexSize);
                        dstData += dstVertexStride;
                    }
                }
            }

            vertexOffset += meshData->GetNumVertices();
        }

        return combinedMeshData;
    }

    void MeshData::SetVertexData(VertexElementSemantic semantic, void* data, UINT32 size, UINT32 semanticIdx, UINT32 streamIdx)
    {
        assert(data != nullptr);

        if(!_vertexData->HasElement(semantic, semanticIdx, streamIdx))
        {
            TE_DEBUG("MeshData doesn't contain an element of specified type: Semantic: " + ToString(semantic) +
                "Semantic index: " + ToString(semanticIdx) + ", Stream index: " + ToString(streamIdx), __FILE__, __LINE__);
            return;
        }

        UINT32 elementSize = _vertexData->GetElementSize(semantic, semanticIdx, streamIdx);
        UINT32 totalSize = elementSize * _numVertices;

        if(totalSize != size)
        {
            TE_ASSERT_ERROR(false, "Buffer sizes don't match. Expected: " + ToString(totalSize) + ". Got: " + ToString(size), __FILE__, __LINE__);
        }

        UINT32 indexBufferOffset = GetIndexBufferSize();

        UINT32 elementOffset = GetElementOffset(semantic, semanticIdx, streamIdx);
        UINT32 vertexStride = _vertexData->GetVertexStride(streamIdx);

        UINT8* dst = GetData() + indexBufferOffset + elementOffset;
        UINT8* src = (UINT8*)data;
        for(UINT32 i = 0; i < _numVertices; i++)
        {
            memcpy(dst, src, elementSize);
            dst += vertexStride;
            src += elementSize;
        }
    }

    void MeshData::GetVertexData(VertexElementSemantic semantic, void* data, UINT32 size, UINT32 semanticIdx, UINT32 streamIdx)
    {
        assert(data != nullptr);

        if (!_vertexData->HasElement(semantic, semanticIdx, streamIdx))
        {
            TE_DEBUG("MeshData doesn't contain an element of specified type: Semantic: " + ToString(semantic) + ", "
                "Semantic index: " + ToString(semanticIdx) + ", Stream index: " + ToString(streamIdx), __FILE__, __LINE__);
            return;
        }

        UINT32 elementSize = _vertexData->GetElementSize(semantic, semanticIdx, streamIdx);
        UINT32 totalSize = elementSize * _numVertices;

        if (totalSize != size)
        {
            TE_ASSERT_ERROR(false, "Buffer sizes don't match. Expected: " + ToString(totalSize) + ". Got: " + ToString(size), __FILE__, __LINE__);
        }

        UINT32 indexBufferOffset = GetIndexBufferSize();

        UINT32 elementOffset = GetElementOffset(semantic, semanticIdx, streamIdx);
        UINT32 vertexStride = _vertexData->GetVertexStride(streamIdx);

        UINT8* src = GetData() + indexBufferOffset + elementOffset;
        UINT8* dst = (UINT8*)data;
        for (UINT32 i = 0; i < _numVertices; i++)
        {
            memcpy(dst, src, elementSize);
            dst += elementSize;
            src += vertexStride;
        }
    }

    UINT32 MeshData::GetIndexBufferOffset() const
    {
        return 0;
    }

    UINT32 MeshData::GetStreamOffset(UINT32 streamIdx) const
    {
        UINT32 streamOffset = _vertexData->GetStreamOffset(streamIdx);

        return streamOffset * _numVertices;
    }

    UINT8* MeshData::GetElementData(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
    {
        return GetData() + GetIndexBufferSize() + GetElementOffset(semantic, semanticIdx, streamIdx);
    }

    UINT8* MeshData::GetStreamData(UINT32 streamIdx) const
    {
        return GetData() + GetIndexBufferSize() + GetStreamOffset(streamIdx);
    }

    UINT32 MeshData::GetIndexElementSize() const
    {
        return _indexType == IT_32BIT ? sizeof(UINT32) : sizeof(UINT16);
    }

    UINT32 MeshData::GetElementOffset(VertexElementSemantic semantic, UINT32 semanticIdx, UINT32 streamIdx) const
    {
        return GetStreamOffset(streamIdx) + _vertexData->GetElementOffsetFromStream(semantic, semanticIdx, streamIdx);
    }

    UINT32 MeshData::GetIndexBufferSize() const
    {
        return _numIndices * GetIndexElementSize();
    }

    UINT32 MeshData::GetStreamSize(UINT32 streamIdx) const
    {
        return _vertexData->GetVertexStride(streamIdx) * _numVertices;
    }

    UINT32 MeshData::GetStreamSize() const
    {
        return _vertexData->GetVertexStride() * _numVertices;
    }

    Bounds MeshData::CalculateBounds() const
    {
        Bounds bounds;

        SPtr<VertexDataDesc> vertexDesc = GetVertexDesc();
        for (UINT32 i = 0; i < vertexDesc->GetNumElements(); i++)
        {
            const VertexElement& curElement = vertexDesc->GetElement(i);

            if (curElement.GetSemantic() != VES_POSITION || (curElement.GetType() != VET_FLOAT3 && curElement.GetType() != VET_FLOAT4))
                continue;

            UINT8* data = GetElementData(curElement.GetSemantic(), curElement.GetSemanticIdx(), curElement.GetStreamIdx());
            UINT32 stride = vertexDesc->GetVertexStride(curElement.GetStreamIdx());

            if (GetNumVertices() > 0)
            {
                Vector3 curPosition = *(Vector3*)data;
                Vector3 accum = curPosition;
                Vector3 min = curPosition;
                Vector3 max = curPosition;

                for (UINT32 i = 1; i < GetNumVertices(); i++)
                {
                    curPosition = *(Vector3*)(data + stride * i);
                    accum += curPosition;
                    min = Vector3::Min(min, curPosition);
                    max = Vector3::Max(max, curPosition);
                }

                Vector3 center = accum / (float)GetNumVertices();
                float radiusSqrd = 0.0f;

                for (UINT32 i = 0; i < GetNumVertices(); i++)
                {
                    curPosition = *(Vector3*)(data + stride * i);
                    float dist = center.SquaredDistance(curPosition);

                    if (dist > radiusSqrd)
                        radiusSqrd = dist;
                }

                float radius = Math::Sqrt(radiusSqrd);

                bounds = Bounds(AABox(min, max), Sphere(center, radius));
                break;
            }
        }

        return bounds;
    }
}