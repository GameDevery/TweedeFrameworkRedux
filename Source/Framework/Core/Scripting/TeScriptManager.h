#pragma once

#include "TeCorePrerequisites.h"
#include "Utility/TeModule.h"
#include "Platform/TeFolderMonitor.h"

namespace te
{
    class NativeScript;

    struct UnloadedScript
    {
        Script* ScriptToReload;
        HSceneObject PreviousSceneObject;
    };

    /**	Handles initialization of a scripting system. */
    class TE_CORE_EXPORT ScriptManager : public Module<ScriptManager>
    {
    public:
        ScriptManager() = default;
        ~ScriptManager() = default;

        void OnStartUp() override;
        void OnShutDown() override;

        /** When a Script (so a CScript) is created, we need to register this script inside the manager */
        void RegisterScript(Script* script);

        /** When a Script is destroyed, we want to remove this script from the manager */
        void UnregisterScript(Script* script);

        /** When a script need a new instance of the given native script */
        NativeScript* CreateNativeScript(const String& name);

        /** Memory management for NativeScript, is manual, each script has an exported function for deleting a NativeScript */
        void DeleteNativeScript(NativeScript* script);

        /** Unloads all script libraries loaded in the engine */
        void UnloadAll();

    public: // #### EVENTS FOR SCRIPTS
        /** Called once per frame before scene update. */
        void PreUpdate();

        /** Called once per frame after scene update */
        void PostUpdate();

        /** Called once per frame after engine render */
        void PostRender();

        /** Update any script which has been modified */
        void Update();

    private: // #### EVENTS FOR SCRIPTS FOLRDER WATCHING
        /**	Triggered when the native folder monitor detects a file has been modified. */
        void OnMonitorFileModified(const String& path);

        /**	Triggered when the native folder monitor detects a file has been added. */
        void OnMonitorFileAdded(const String& path);

        /**	Triggered when the native folder monitor detects a file has been removed. */
        void OnMonitorFileRemoved(const String& path);

        /**	Triggered when the native folder monitor detects a file has been renamed. */
        void OnMonitorFileRenamed(const String& from, const String& to);

    private:
        /** Try to load a new script lib (.dll, .so) */
        DynLib* LoadScriptLibrary(const String& name);

        /** 
         * Try to unload a new script lib (.dll, .so), 
         * if second argument is not null, it's filled with pointer to script which were using this nativeScript 
         */
        void UnloadScriptLibrary(const String& name, Vector<UnloadedScript>* unloadedScripts = nullptr);

        /** Returns (and loads if not loaded yet) the given dynamic library */
        DynLib* GetScriptLibrary(const String& name);

        /** Compiles a library using provided name. All libraries will be located in the same directory as dlls and binaries */
        bool CompileLibrary(const String& name);

        /** Check if a library already exists. Usefull if we don't want to compile everything (time consuming) */
        bool LibraryExists(const String& name);

    public:
        /** Path where the debug configuration managed assemblies are located at, relative to the working directory. */
        static const String LIBRARIES_PATH;

    private:
        UnorderedMap<String, DynLib*> _scriptLibraries;
        Vector<Script*> _scripts;
        FolderMonitor _folderMonitor;

    };

    /** Provides easy access to the ScriptManager. */
    TE_CORE_EXPORT ScriptManager& gScriptManager();
}