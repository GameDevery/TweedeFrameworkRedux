#pragma once

#include "TeCorePrerequisites.h"
#include "Importer/TeImportOptions.h"
#include "Importer/TeBaseImporter.h"
#include "Utility/TeModule.h"

namespace te
{
    /** Module responsible for importing various asset types and converting them to types usable by the engine. */
    class TE_CORE_EXPORT Importer : public Module<Importer>
    {
    public:
        Importer(); 
        ~Importer();

        TE_MODULE_STATIC_HEADER_MEMBER(Importer)

        /**
         * Imports a resource at the specified location, and returns the loaded data. If file contains more than one
         * resource only the primary resource is imported (for example an FBX a mesh would be imported, but animations
         * ignored).
         *
         * @param[in]	inputFilePath	Pathname of the input file.
         * @param[in]	importOptions	(optional) Options for controlling the import. Caller must ensure import options
         *								actually match the type of the importer used for the file type.
         * @param[in]	UUID			Specific UUID to assign to the resource. If not specified a randomly generated
         *								UUID will be assigned.
         * @return						Imported resource.
        */
        HResource Import(const String& inputFilePath, SPtr<const ImportOptions> importOptions, const UUID& UUID);

        template <class T>
        ResourceHandle<T> Import(const String& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr, const UUID& uuid = UUID::EMPTY)
        {
            return static_resource_cast<T>(Import(inputFilePath, importOptions, uuid));
        }

        /** Alternative to import() which doesn't create a resource handle, but instead returns a raw resource pointer. */
        SPtr<Resource> _import(const String& inputFilePath, SPtr<const ImportOptions> importOptions = nullptr);

        /**
         * Checks if we can import a file with the specified extension.
         *
         * @param[in]	extension	The extension without the leading dot.
         */
        bool SupportsFileType(const String& extension) const;

        /**
         * Registers a new asset importer for a specific set of extensions (as determined by the implementation). If an
         * asset importer for one or multiple extensions already exists, it is removed and replaced with this one.
         * @param[in]	importer	The importer that is able to handle import of certain type of files.
         *
         * @note	This method should only be called by asset importers themselves on startup. Importer takes ownership
         *			of the provided pointer and will release it. Assumes it is allocated using the general allocator.
         */
        void _registerAssetImporter(BaseImporter* importer);

    private:
        /**
         * Searches available importers and attempts to find one that can import the file of the provided type. Returns null
         * if one cannot be found.
         */
        BaseImporter* GetImporterForFile(const String& inputFilePath) const;

        /**
         * Prepares for import of a file at the specified path. Returns the type of importer the file can be imported with,
         * or null if the file isn't valid or is of unsupported type. Also creates the default set of import options unless
         * already provided.
         */
        BaseImporter* PrepareForImport(const String& filePath, SPtr<const ImportOptions>& importOptions) const;

    private:
        Vector<BaseImporter*> _assetImporters;
    };

    /** Provides easier access to Importer. */
    TE_CORE_EXPORT Importer& gImporter();
}