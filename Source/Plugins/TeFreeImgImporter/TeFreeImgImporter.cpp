#include "TeFreeImgImporter.h"
#include "Importer/TeTextureImportOptions.h"
#include "Image/TeColor.h"
#include "Image/TeTexture.h"
#include "Image/TePixelData.h"
#include "Image/TePixelUtil.h"
#include "Utility/TeBitwise.h"
#include "Utility/TeFileStream.h"
#include "Utility/TeFileSystem.h"
#include "FreeImage.h"

namespace te
{
    void FreeImageLoadErrorHandler(FREE_IMAGE_FORMAT fif, const char* message)
    {
        // Callback method as required by FreeImage to report problems
        const char* typeName = FreeImage_GetFormatFromFIF(fif);
        if (typeName)
        {
            TE_DEBUG("FreeImage error: " + String(message) + " when loading format " + String(typeName));
        }
        else
        {
            TE_DEBUG("FreeImage error: " + String(message));
        }
    }

    FreeImgImporter::FreeImgImporter()
    {
        FreeImage_Initialise(false);

        // Register codecs
        StringStream strExt;
        strExt << "Supported formats: ";
        bool first = true;
        for (int i = 0; i < FreeImage_GetFIFCount(); ++i)
        {
            // Skip DDS codec since FreeImage does not have the option
            // to keep DXT data compressed, we'll use our own codec
            if ((FREE_IMAGE_FORMAT)i == FIF_DDS)
                continue;

            String exts = String(FreeImage_GetFIFExtensionList((FREE_IMAGE_FORMAT)i));
            if (!first)
                strExt << ",";

            first = false;
            strExt << exts;

            // Pull off individual formats (separated by comma by FI)
            Vector<String> extsVector = Util::Split(exts, u8",");
            for (auto v = extsVector.begin(); v != extsVector.end(); ++v)
            {
                auto findIter = std::find(_extensions.begin(), _extensions.end(), *v);

                if (findIter == _extensions.end())
                {
                    String ext = *v;
                    Util::ToLowerCase(ext);

                    _extensionToFID.insert(std::make_pair(ext, i));
                    _extensions.push_back(ext);
                }
            }
        }

        // Set error handler
        FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);
    }

    FreeImgImporter::~FreeImgImporter()
    {
        FreeImage_DeInitialise();
    }

    bool FreeImgImporter::IsExtensionSupported(const String& ext) const
    {
        String lowerCaseExt = ext;
        std::transform(lowerCaseExt.begin(), lowerCaseExt.end(), lowerCaseExt.begin(), tolower);

        return find(_extensions.begin(), _extensions.end(), lowerCaseExt) != _extensions.end();
    }

    String FreeImgImporter::MagicNumToExtension(const UINT8* magic, UINT32 maxBytes) const
    {
        // Set error handler
        FreeImage_SetOutputMessage(FreeImageLoadErrorHandler);

        FIMEMORY* fiMem =
            FreeImage_OpenMemory((BYTE*)magic, static_cast<DWORD>(maxBytes));

        FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fiMem, (int)maxBytes);
        FreeImage_CloseMemory(fiMem);

        if (fif != FIF_UNKNOWN)
        {
            String ext = String(FreeImage_GetFormatFromFIF(fif));
            Util::ToLowerCase(ext);
            return ext;
        }
        else
        {
            return "";
        }
    }

    SPtr<ImportOptions> FreeImgImporter::CreateImportOptions() const
    {
        return te_shared_ptr_new<TextureImportOptions>();
    }

    SPtr<Resource> FreeImgImporter::Import(const String& filePath, const SPtr<const ImportOptions> importOptions)
    {
        const TextureImportOptions* textureImportOptions = static_cast<const TextureImportOptions*>(importOptions.get());

        SPtr<PixelData> imgData = ImportRawImage(filePath);
        if (imgData == nullptr || imgData->GetData() == nullptr)
        {
            return nullptr;
        }

        Vector<SPtr<PixelData>> faceData;

        TextureType texType;
        if (textureImportOptions->IsCubemap)
        {
            texType = TEX_TYPE_CUBE_MAP;

            std::array<SPtr<PixelData>, 6> cubemapFaces;
            if (GenerateCubemap(imgData, textureImportOptions->CubemapType, cubemapFaces))
            {
                faceData.insert(faceData.begin(), cubemapFaces.begin(), cubemapFaces.end());
            }
            else // Fall-back to 2D texture
            {
                texType = TEX_TYPE_2D;
                faceData.push_back(imgData);
            }
        }
        else
        {
            texType = TEX_TYPE_2D;
            faceData.push_back(imgData);
        }

        UINT32 numMips = 0;
        if (textureImportOptions->GenerateMips)
        {
            if (Bitwise::IsPow2(faceData[0]->GetWidth()) && Bitwise::IsPow2(faceData[0]->GetHeight()))
            {
                UINT32 maxPossibleMip = PixelUtil::GetMaxMipmaps(faceData[0]->GetWidth(), faceData[0]->GetHeight(),
                    faceData[0]->GetDepth(), faceData[0]->GetFormat());

                if (textureImportOptions->MaxMip == 0)
                    numMips = maxPossibleMip;
                else
                    numMips = std::min(maxPossibleMip, textureImportOptions->MaxMip);
            }
            else
            {
                TE_DEBUG("Width and height of your image must be a power of 2");
            }
        }

        int usage = TU_DEFAULT;
        if (textureImportOptions->CpuCached)
            usage |= TU_CPUCACHED;

        bool sRGB = textureImportOptions->SRGB;

        TEXTURE_DESC texDesc;
        texDesc.Type = texType;
        texDesc.Width = faceData[0]->GetWidth();
        texDesc.Height = faceData[0]->GetHeight();
        texDesc.NumMips = numMips;
        texDesc.Format = textureImportOptions->Format;
        texDesc.Usage = usage;
        texDesc.HwGamma = sRGB;

        SPtr<Texture> texture = Texture::_createPtr(texDesc);

        UINT32 numFaces = (UINT32)faceData.size();
        for (UINT32 i = 0; i < numFaces; i++)
        {
            Vector<SPtr<PixelData>> mipLevels;
            if (numMips > 0)
            {
                MipMapGenOptions mipOptions;
                mipOptions.isSRGB = sRGB;

                mipLevels = PixelUtil::GenMipmaps(*faceData[i], mipOptions, numMips);
            }
            else
            {
                mipLevels.push_back(faceData[i]);
            }

            for (UINT32 mip = 0; mip < (UINT32)mipLevels.size(); ++mip)
            {
                SPtr<PixelData> dst = texture->GetProperties().AllocBuffer(0, mip);

                PixelUtil::BulkPixelConversion(*mipLevels[mip], *dst);
                texture->WriteData(*dst, mip, i); //BUG in original version
            }
        }

        texture->SetName(filePath);
        texture->SetPath(filePath);
        return texture;
    }

    SPtr<PixelData> FreeImgImporter::ImportRawImage(const String& filePath)
    {
        uint8_t* data = nullptr;
        size_t size = 0;
        FREE_IMAGE_FORMAT imageFormat;

        {
            Lock lock = FileScheduler::GetLock(filePath);
            FileStream file(filePath);

            if (file.Fail())
            {
                TE_DEBUG("Cannot open file: " + filePath);
                return nullptr;
            }

            size = file.Size();
            if (size > std::numeric_limits<UINT32>::max())
            {
                TE_DEBUG("File size larger than supported!");
                return nullptr;
            }

            UINT32 magicLen = std::min((UINT32)size, 32u);
            UINT8 magicBuf[32];
            file.Read(static_cast<char*>((void*)magicBuf), static_cast<std::streamsize>(magicLen));
            file.Seek(0);

            String fileExtension = MagicNumToExtension(magicBuf, magicLen);
            auto findFormat = _extensionToFID.find(fileExtension);
            if (findFormat == _extensionToFID.end())
            {
                TE_DEBUG("Type of the file provided is not supported by this importer. File type: " + fileExtension);
                return nullptr;
            }

            imageFormat = (FREE_IMAGE_FORMAT)findFormat->second;

            data = static_cast<uint8_t*>(te_allocate(static_cast<UINT32>(size)));
            memset(data, 0, size);
            file.Read(static_cast<char*>((void*)data), static_cast<std::streamsize>(size));

            file.Close();
        }

        FIMEMORY* fiMem = FreeImage_OpenMemory(data, static_cast<DWORD>(size));

        FIBITMAP* fiBitmap = FreeImage_LoadFromMemory((FREE_IMAGE_FORMAT)imageFormat, fiMem);
        if (!fiBitmap)
        {
            TE_DEBUG("Error decoding image");
        }

        UINT32 width = FreeImage_GetWidth(fiBitmap);
        UINT32 height = FreeImage_GetHeight(fiBitmap);
        PixelFormat format = PF_UNKNOWN;

        // Must derive format first, this may perform conversions

        FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(fiBitmap);
        FREE_IMAGE_COLOR_TYPE colourType = FreeImage_GetColorType(fiBitmap);
        unsigned bpp = FreeImage_GetBPP(fiBitmap);
        unsigned srcElemSize = 0;

        switch (imageType)
        {
        case FIT_UNKNOWN:
        case FIT_COMPLEX:
        case FIT_UINT32:
        case FIT_INT32:
        case FIT_DOUBLE:
        default:
            TE_DEBUG("Unknown or unsupported image format");
            break;
        case FIT_BITMAP:
            // Standard image type
            // Perform any colour conversions for greyscale
            if (colourType == FIC_MINISWHITE || colourType == FIC_MINISBLACK)
            {
                FIBITMAP* newBitmap = FreeImage_ConvertToGreyscale(fiBitmap);
                // free old bitmap and replace
                FreeImage_Unload(fiBitmap);
                fiBitmap = newBitmap;
                // get new formats
                bpp = FreeImage_GetBPP(fiBitmap);
                colourType = FreeImage_GetColorType(fiBitmap);
            }
            // Perform any colour conversions for RGB
            else if (bpp < 8 || colourType == FIC_PALETTE || colourType == FIC_CMYK)
            {
                FIBITMAP* newBitmap = FreeImage_ConvertTo24Bits(fiBitmap);
                // free old bitmap and replace
                FreeImage_Unload(fiBitmap);
                fiBitmap = newBitmap;
                // get new formats
                bpp = FreeImage_GetBPP(fiBitmap);
                colourType = FreeImage_GetColorType(fiBitmap);
            }

            // by this stage, 8-bit is greyscale, 16/24/32 bit are RGB[A]
            switch (bpp)
            {
            case 8:
                format = PF_R8;
                srcElemSize = 1;
                break;
            case 16:
                // Determine 555 or 565 from green mask
                // cannot be 16-bit greyscale since that's FIT_UINT16
                if (FreeImage_GetGreenMask(fiBitmap) == FI16_565_GREEN_MASK)
                {
                    TE_DEBUG("Format not supported by the engine.");
                    return nullptr;
                }
                else
                {
                    TE_DEBUG("Format not supported by the engine.");
                    return nullptr;
                    // FreeImage doesn't support 4444 format so must be 1555
                }
                srcElemSize = 2;
                break;
            case 24:
                // FreeImage differs per platform
                //     PF_BYTE_BGR[A] for little endian (== PF_ARGB native)
                //     PF_BYTE_RGB[A] for big endian (== PF_RGBA native)
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
                format = PF_RGB8;
#else
                format = PF_BGR8;
#endif
                srcElemSize = 3;
                break;
            case 32:
#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_RGB
                format = PF_RGBA8;
#else
                format = PF_BGRA8;
#endif
                srcElemSize = 4;
                break;
            };
            break;
        case FIT_UINT16:
        case FIT_INT16:
            // 16-bit greyscale
            TE_DEBUG("No INT pixel formats supported currently.");
            return nullptr;
            break;
        case FIT_FLOAT:
            // Single-component floating point data
            format = PF_R32F;
            srcElemSize = 4;
            break;
        case FIT_RGB16:
            format = PF_RGBA16;
            srcElemSize = 2 * 3;
            break;
        case FIT_RGBA16:
            format = PF_RGBA16;
            srcElemSize = 2 * 4;
            break;
        case FIT_RGBF:
            format = PF_RGB32F;
            srcElemSize = 4 * 3;
            break;
        case FIT_RGBAF:
            format = PF_RGBA32F;
            srcElemSize = 4 * 4;
            break;
        };

        unsigned char* srcData = FreeImage_GetBits(fiBitmap);
        unsigned srcPitch = FreeImage_GetPitch(fiBitmap);

        // Final data - invert image and trim pitch at the same time
        UINT32 dstElemSize = PixelUtil::GetNumElemBytes(format);
        UINT32 dstPitch = width * PixelUtil::GetNumElemBytes(format);

        // Bind output buffer
        SPtr<PixelData> texData = te_shared_ptr_new<PixelData>(width, height, 1, format);
        texData->AllocateInternalBuffer();
        UINT8* output = texData->GetData();

        UINT8* pSrc;
        UINT8* pDst = output;

        // Copy row by row, which is faster
        if (srcElemSize == dstElemSize)
        {
            for (UINT32 y = 0; y < height; ++y)
            {
                pSrc = srcData + (height - y - 1) * srcPitch;
                memcpy(pDst, pSrc, dstPitch);
                pDst += dstPitch;
            }
        }
        else
        {
            for (UINT32 y = 0; y < height; ++y)
            {
                pSrc = srcData + (height - y - 1) * srcPitch;

                for (UINT32 x = 0; x < width; ++x)
                    memcpy(pDst + x * dstElemSize, pSrc + x * srcElemSize, srcElemSize);

                pDst += dstPitch;
            }
        }

        FreeImage_Unload(fiBitmap);
        FreeImage_CloseMemory(fiMem);

        te_delete(data);

        return texData;
    }

    /**
     * Reads the source texture as a horizontal or vertical list of 6 cubemap faces.
     *
     * @param[in]	source		Source texture to read.
     * @param[out]	output		Output array that will contain individual cubemap faces.
     * @param[in]	faceSize	Size of a single face, in pixels. Both width & height must match.
     * @param[in]	vertical	True if the faces are laid out vertically, false if horizontally.
     */
    void ReadCubemapList(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, UINT32 faceSize, bool vertical)
    {
        Vector2I faceStart;
        for(UINT32 i = 0; i < 6; i++)
        {
            output[i] = PixelData::Create(faceSize, faceSize, 1, source->GetFormat());

            PixelVolume volume(faceStart.x, faceStart.y, faceStart.x + faceSize, faceStart.y + faceSize);
            PixelUtil::Copy(*source, *output[i], faceStart.x, faceStart.y);

            if (vertical)
                faceStart.y += faceSize;
            else
                faceStart.x += faceSize;
        }
    }

    /**
     * Reads the source texture as a horizontal or vertical "cross" of 6 cubemap faces.
     *
     * Vertical layout:
     *    +Y
     * -X +Z +X
     *    -Y
     *    -Z
     *
     * Horizontal layout:
     *    +Y
     * -X +Z +X -Z
     *    -Y
     *
     * @param[in]	source		Source texture to read.
     * @param[out]	output		Output array that will contain individual cubemap faces.
     * @param[in]	faceSize	Size of a single face, in pixels. Both width & height must match.
     * @param[in]	vertical	True if the faces are laid out vertically, false if horizontally.
     */
    void ReadCubemapCross(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, UINT32 faceSize,
        bool vertical)
    {
        const static UINT32 vertFaceIndices[] = { 5, 3, 1, 7, 4, 10 };
        const static UINT32 horzFaceIndices[] = { 6, 4, 1, 9, 5, 7 };

        const UINT32* faceIndices = vertical ? vertFaceIndices : horzFaceIndices;
        UINT32 numFacesInRow = vertical ? 3 : 4;

        for (UINT32 i = 0; i < 6; i++)
        {
            output[i] = PixelData::Create(faceSize, faceSize, 1, source->GetFormat());

            UINT32 faceX = (faceIndices[i] % numFacesInRow) * faceSize;
            UINT32 faceY = (faceIndices[i] / numFacesInRow) * faceSize;

            PixelVolume volume(faceX, faceY, faceX + faceSize, faceY + faceSize);
            PixelUtil::Copy(*source, *output[i], faceX, faceY);
        }

        // Flip -Z as it's upside down
        if (vertical)
        {
            INT32 mode = MirrorMode::X;
            mode |= MirrorMode::Y;
            PixelUtil::Mirror(*output[5], mode);
        }
    }

    /** Method that maps a direction to a point on a plane in range [0, 1] using spherical mapping. */
    Vector2 MapCubemapDirToSpherical(const Vector3& dir)
    {
        // Using the OpenGL spherical mapping formula
        Vector3 nrmDir = Vector3::Normalize(dir);
        nrmDir.z += 1.0f;

        float m = 2 * nrmDir.Length();

        float u = nrmDir.x / m + 0.5f;
        float v = nrmDir.y / m + 0.5f;

        return Vector2(u, v);
    }

    /**
     * Method that maps a direction to a point on a plane in range [0, 1] using cylindrical mapping. This mapping is also
     * know as longitude-latitude mapping, Blinn/Newell mapping or equirectangular cylindrical mapping.
     */
    Vector2 MapCubemapDirToCylindrical(const Vector3& dir)
    {
        Vector3 nrmDir = Vector3::Normalize(dir);

        float u = (atan2(nrmDir.x, nrmDir.z) + Math::PI) / Math::TWO_PI;
        float v = acos(nrmDir.y) / Math::PI;

        return Vector2(u, v);
    }

    /** Resizes the provided cubemap faces and outputs a new set of resized faces. */
    void DownsampleCubemap(const std::array<SPtr<PixelData>, 6>& input, std::array<SPtr<PixelData>, 6>& output, UINT32 size)
    {
        for(UINT32 i = 0; i < 6; i++)
        {
            output[i] = PixelData::Create(size, size, 1, input[i]->GetFormat());
            PixelUtil::Scale(*input[i], *output[i]);
        }
    }

    /**
     * Reads the source texture and remaps its data into six faces of a cubemap.
     *
     * @param[in]	source		Source texture to remap.
     * @param[out]	output		Remapped faces of the cubemap.
     * @param[in]	faceSize	Width/height of each individual face, in pixels.
     * @param[in]	remap		Function to use for remapping the cubemap direction to UV.
     */
    void ReadCubemapUVRemap(const SPtr<PixelData>& source, std::array<SPtr<PixelData>, 6>& output, UINT32 faceSize,
        const std::function<Vector2(Vector3)>& remap)
    {
        struct RemapInfo
        {
            int idx[3];
            Vector3 sign;
        };

        // Mapping from default (X, Y, 1.0f) plane to relevant cube face. Also flipping Y so it corresponds to how pixel
        // coordinates are mapped.
        static const RemapInfo remapLookup[] =
        {
            { {2, 1, 0}, { -1.0f, -1.0f,  1.0f }}, // X+
            { {2, 1, 0}, {  1.0f, -1.0f, -1.0f }}, // X-
            { {0, 2, 1}, {  1.0f,  1.0f,  1.0f }}, // Y+
            { {0, 2, 1}, {  1.0f, -1.0f, -1.0f }}, // Y-
            { {0, 1, 2}, {  1.0f, -1.0f,  1.0f }}, // Z+
            { {0, 1, 2}, { -1.0f, -1.0f, -1.0f }}  // Z-
        };

        float invSize = 1.0f / faceSize;
        for (UINT32 faceIdx = 0; faceIdx < 6; faceIdx++)
        {
            output[faceIdx] = PixelData::Create(faceSize, faceSize, 1, source->GetFormat());

            const RemapInfo& remapInfo = remapLookup[faceIdx];
            for (UINT32 y = 0; y < faceSize; y++)
            {
                for (UINT32 x = 0; x < faceSize; x++)
                {
                    // Map from pixel coordinates to [-1, 1] range.
                    Vector2 sourceCoord = (Vector2((float)x, (float)y) * invSize) * 2.0f - Vector2(1.0f, 1.0f);
                    Vector3 direction = Vector3(sourceCoord.x, sourceCoord.y, 1.0f);

                    direction *= remapInfo.sign;

                    // Rotate towards current face
                    Vector3 rotatedDir;
                    rotatedDir[remapInfo.idx[0]] = direction.x;
                    rotatedDir[remapInfo.idx[1]] = direction.y;
                    rotatedDir[remapInfo.idx[2]] = direction.z;

                    // Find location in the source to sample from
                    Vector2 sourceUV = remap(rotatedDir);
                    Color color = source->SampleColorAt(sourceUV);

                    // Write the sampled color
                    output[faceIdx]->SetColorAt(color, x, y);
                }
            }
        }
    }

    bool FreeImgImporter::GenerateCubemap(const SPtr<PixelData>& source, CubemapSourceType sourceType,
                         std::array<SPtr<PixelData>, 6>& output)
    {
        // Note: Expose this as a parameter if needed:
        UINT32 cubemapSupersampling = 1; // Set to amount of samples

        Vector2I faceSize;
        bool cross = false;
        bool vertical = false;

        switch(sourceType)
        {
        case CubemapSourceType::Faces:
            {
                float aspect = source->GetWidth() / (float)source->GetHeight();
                
                if(Math::ApproxEquals(aspect, 6.0f)) // Horizontal list
                {
                    faceSize.x = source->GetWidth() / 6;
                    faceSize.y = source->GetHeight();
                }
                else if(Math::ApproxEquals(aspect, 1.0f / 6.0f)) // Vertical list
                {
                    faceSize.x = source->GetWidth();
                    faceSize.y = source->GetHeight() / 6;
                    vertical = true;
                }
                else if(Math::ApproxEquals(aspect, 4.0f / 3.0f)) // Horizontal cross
                {
                    faceSize.x = source->GetWidth() / 4;
                    faceSize.y = source->GetHeight() / 3;
                    cross = true;
                }
                else if(Math::ApproxEquals(aspect, 3.0f / 4.0f)) // Vertical cross
                {
                    faceSize.x = source->GetWidth() / 3;
                    faceSize.y = source->GetHeight() / 4;
                    cross = true;
                    vertical = true;
                }
                else
                {
                    TE_DEBUG("Unable to generate a cubemap: unrecognized face configuration.");
                    return false;
                }
            }
            break;
        case CubemapSourceType::Cylindrical:
        case CubemapSourceType::Spherical:
            // Half of the source size will be used for each cube face, which should yield good enough quality
            faceSize.x = std::max(source->GetWidth(), source->GetHeight()) / 2;
            faceSize.x = Bitwise::ClosestPow2(faceSize.x);

            // Don't allow sizes larger than 4096
            faceSize.x = std::min(faceSize.x, 4096);

            faceSize.y = faceSize.x;

            break;
        default: // Assuming single-image
            faceSize.x = source->GetWidth();
            faceSize.y = source->GetHeight();
            break;
        }

        if (faceSize.x != faceSize.y)
        {
            TE_DEBUG("Unable to generate a cubemap: width & height must match.");
            return false;
        }

        if (!Bitwise::IsPow2(faceSize.x))
        {
            TE_DEBUG("Unable to generate a cubemap: width & height must be powers of 2.");
            return false;
        }

        switch (sourceType)
        {
        case CubemapSourceType::Faces:
        {
            if (cross)
                ReadCubemapCross(source, output, faceSize.x, vertical);
            else
                ReadCubemapList(source, output, faceSize.x, vertical);
        }
        break;
        case CubemapSourceType::Cylindrical:
        {			
            UINT32 superSampledFaceSize = faceSize.x * cubemapSupersampling;

            std::array<SPtr<PixelData>, 6> superSampledOutput;
            ReadCubemapUVRemap(source, superSampledOutput, superSampledFaceSize, &MapCubemapDirToCylindrical);

            if (faceSize.x != (INT32)superSampledFaceSize)
                DownsampleCubemap(superSampledOutput, output, faceSize.x);
            else
                output = superSampledOutput;
        }
        break;
        case CubemapSourceType::Spherical:
        {
            UINT32 superSampledFaceSize = faceSize.x * cubemapSupersampling;

            std::array<SPtr<PixelData>, 6> superSampledOutput;
            ReadCubemapUVRemap(source, superSampledOutput, superSampledFaceSize, &MapCubemapDirToSpherical);

            if (faceSize.x != (INT32)superSampledFaceSize)
                DownsampleCubemap(superSampledOutput, output, faceSize.x);
            else
                output = superSampledOutput;
        }
        break;
        default: // Single-image
            for (UINT32 i = 0; i < 6; i++)
                output[i] = source;

            break;
        }

        return true;
    }
}
