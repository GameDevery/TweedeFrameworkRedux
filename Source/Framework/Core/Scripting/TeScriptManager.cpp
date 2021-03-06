#include "TeScriptManager.h"

#include "Utility/TeDynLibManager.h"
#include "Utility/TeDynLib.h"
#include "Utility/TeFileSystem.h"
#include "TeScript.h"
#include "TeNativeScript.h"

#include <filesystem>

using namespace std::placeholders;

namespace te
{
    const String ScriptManager::LIBRARIES_PATH = "Data/Scripts/";

    void ScriptManager::OnStartUp()
    {
#ifdef TE_ENGINE_BUILD
        static String appRoot = RAW_APP_ROOT;
#else
        static String rawAppRoot = "";
#endif

        UINT32 folderChanges = 0;
        folderChanges |= (UINT32)FolderChangeFlag::FileName;
        folderChanges |= (UINT32)FolderChangeFlag::FileWrite;

        _folderMonitor.StartMonitor(appRoot + LIBRARIES_PATH, false, folderChanges);

        _folderMonitor.OnAdded.Connect(std::bind(&ScriptManager::OnMonitorFileAdded, this, _1));
        _folderMonitor.OnRemoved.Connect(std::bind(&ScriptManager::OnMonitorFileRemoved, this, _1));
        _folderMonitor.OnModified.Connect(std::bind(&ScriptManager::OnMonitorFileModified, this, _1));
        _folderMonitor.OnRenamed.Connect(std::bind(&ScriptManager::OnMonitorFileRenamed, this, _1, _2));
    }

    void ScriptManager::OnShutDown()
    {
#if TE_DEBUG_MODE
        if (_scripts.size() > 0)
            TE_ASSERT_ERROR(false, "Not all scripts have been unregistered from Script Manager");
#endif

        _folderMonitor.StopMonitorAll();
        _scripts.clear();
        UnloadAll();
    }

    void ScriptManager::PreUpdate()
    {
        for (auto& script : _scripts)
        {
            script->PreUpdate();
        }
    }

    void ScriptManager::PostUpdate()
    { 
        for (auto& script : _scripts)
        {
            script->PostUpdate();
        }
    }

    void ScriptManager::PostRender()
    {
        for (auto& script : _scripts)
        {
            script->PostRender();
        }
    }

    void ScriptManager::Update()
    {
        _folderMonitor.Update();

        for (auto& script : _scripts)
        {
            script->Update();
        }
    }

    void ScriptManager::RegisterScript(Script* script)
    {
        if (std::find(_scripts.begin(), _scripts.end(), script) == _scripts.end())
            _scripts.push_back(script);
    }

    void ScriptManager::UnregisterScript(Script* script)
    {
        auto iter = std::find(_scripts.begin(), _scripts.end(), script);

        if(iter != _scripts.end())
            _scripts.erase(iter);
    }

    NativeScript* ScriptManager::CreateNativeScript(const String& name)
    {
        NativeScript* script = nullptr;
        typedef NativeScript* (*LoadScriptFunc)();
        DynLib* library = GetScriptLibrary(name);

        if (library)
        {
            LoadScriptFunc loadScriptFunc = (LoadScriptFunc)library->GetSymbol("LoadScript");

            if (loadScriptFunc)
                script = (NativeScript*)loadScriptFunc();
            else
                TE_DEBUG("Script \"" + name + "\" has been built but LoadScript() function was not found");
        }

        return script;
    }

    void ScriptManager::DeleteNativeScript(NativeScript* script)
    {
        typedef void (*UnloadScriptFunc)(NativeScript*);
        DynLib* library = GetScriptLibrary(script->GetLibraryName());

        if (library)
        {
            UnloadScriptFunc unloadScriptFunc = (UnloadScriptFunc)library->GetSymbol("UnloadScript");
            unloadScriptFunc(script);
        }
    }

    DynLib* ScriptManager::LoadScriptLibrary(const String& name)
    {
        if (!LibraryExists(name))
        {
            if (!CompileLibrary(name))
                return nullptr;
        }

        if (LibraryExists(name))
        {
            DynLib* library = gDynLibManager().Load(name);
            if (library != nullptr)
                _scriptLibraries[name] = library;

            return library;
        }

        return nullptr;
    }

    void ScriptManager::UnloadScriptLibrary(const String& name, Vector<UnloadedScript>* unloadedScripts)
    {
        auto iter = _scriptLibraries.find(name);
        if (iter != _scriptLibraries.end())
        {
            // Remove instances of script which will be unloaded
            for (auto& script : _scripts)
            {
                auto nativeScript = script->GetNativeScript();

                if (nativeScript && unloadedScripts)
                {
                    UnloadedScript unloadedScript;
                    unloadedScript.ScriptToReload = script;
                    unloadedScript.PreviousSceneObject = nativeScript->GetParentSceneObject();

                    (*unloadedScripts).push_back(unloadedScript);
                }

                if (nativeScript && name == nativeScript->GetLibraryName())
                    script->SetNativeScript(String(), HSceneObject());
            }

            gDynLibManager().Unload(iter->second);
            _scriptLibraries.erase(iter);
        }
    }

    DynLib* ScriptManager::GetScriptLibrary(const String& name)
    {
        DynLib* library = nullptr;
        auto iter = _scriptLibraries.find(name);

        if (iter == _scriptLibraries.end())
            library = LoadScriptLibrary(name);
        else
            library = _scriptLibraries[name];

        return library;
    }

    void ScriptManager::UnloadAll()
    {
        for (auto& script : _scripts)
        {
            script->SetNativeScript(String(), HSceneObject());
        }

        for (auto it = _scriptLibraries.begin(); it != _scriptLibraries.end();)
        {
            it->second->Unload();
            it = _scriptLibraries.erase(it);
        }
    }

    bool ScriptManager::LibraryExists(const String& name)
    {
        String path = name + "." + DynLib::EXTENSION;
        return FileSystem::Exists(path);
    }

    void ScriptManager::OnMonitorFileModified(const String& path)
    {
        std::filesystem::path filePath(path);
        if (filePath.has_filename())
        {
            String fileName = filePath.filename().string();
            String fileExtension = filePath.extension().string();

            if (fileExtension == ".cpp")
            {
                fileName = ReplaceAll(fileName, fileExtension, "");
                Vector<UnloadedScript> unloadedScripts;
                UnloadScriptLibrary(fileName, &unloadedScripts);
                if (CompileLibrary(fileName))
                {
                    for (auto& unloadedScript : unloadedScripts)
                    {
                        unloadedScript.ScriptToReload->SetNativeScript(fileName, unloadedScript.PreviousSceneObject);
                    }
                }
            }
        }
    }

    void ScriptManager::OnMonitorFileAdded(const String& path)
    {
        // NOTHING
    }

    void ScriptManager::OnMonitorFileRemoved(const String& path)
    {
        std::filesystem::path filePath(path);
        if (filePath.has_filename())
        {
            String fileName = filePath.filename().string();
            String fileExtension = filePath.extension().string();

            if (fileExtension == ".cpp")
            {
                fileName = ReplaceAll(fileName, fileExtension, "");
                UnloadScriptLibrary(fileName);
            }
        }
    }

    void ScriptManager::OnMonitorFileRenamed(const String& from, const String& to) 
    {
        std::filesystem::path oldFilePath(from);
        std::filesystem::path newFilePath(to);

        if (oldFilePath.has_filename() && newFilePath.has_filename())
        {
            String oldFileName = oldFilePath.filename().string();
            String newFileName = newFilePath.filename().string();
            String oldFileExtension = oldFilePath.extension().string();
            String newFileExtension = newFilePath.extension().string();

            if (oldFileExtension == ".cpp")
            {
                oldFileName = ReplaceAll(oldFileName, oldFileExtension, "");

                Vector<UnloadedScript> unloadedScripts;
                UnloadScriptLibrary(oldFileName, &unloadedScripts);
                if (unloadedScripts.size() > 0 && CompileLibrary(newFileName))
                {
                    if (newFileExtension == ".cpp")
                    {
                        newFileName = ReplaceAll(newFileName, newFileExtension, "");
                        for (auto& unloadedScript : unloadedScripts)
                        {
                            unloadedScript.ScriptToReload->SetNativeScript(newFileName, unloadedScript.PreviousSceneObject);
                        }
                    }
                }
            }
        }
    }

    ScriptManager& gScriptManager()
    {
        return ScriptManager::Instance();
    }
}
