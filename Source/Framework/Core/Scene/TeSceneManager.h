#pragma once

#include "TeCorePrerequisites.h"
#include "Utility/TeEvent.h"
#include "TeSceneObject.h"

namespace te
{
    /** Information about a scene actor and the scene object it has been bound to. */
    struct BoundActorData
    {
        BoundActorData() = default;
        BoundActorData(const SPtr<SceneActor>& actor, const HSceneObject& so)
            : Actor(actor)
            , So(so)
        { }

        SPtr<SceneActor> Actor;
        HSceneObject So;
    };

    /** Contains information about an instantiated scene. */
    class TE_CORE_EXPORT SceneInstance
    {
    public:
        SceneInstance(const String& name, const HSceneObject& root);

        /** Name of the scene. */
        const String& GetName() const { return _name; }

        /** Root object of the scene. */
        const HSceneObject& GetRoot() const { return _root; }

    private:
        friend class SceneManager;

        String _name;
        HSceneObject _root;
    };

    /**
     * Keeps track of all active SceneObjects and their components. Keeps track of component state and triggers their
     * events. Updates the transforms of objects as SceneObject%s move.
     */
    class TE_CORE_EXPORT SceneManager : public Module<SceneManager>
    {
    public:
        SceneManager();
        ~SceneManager();

        /** Returns the object that represents the main scene. */
        const SPtr<SceneInstance>& GetMainScene() const { return _mainScene; }

        /**
         * Destroys all scene objects in the scene.
         *
         * @param[in]	forceAll	If true, then even the persistent objects will be unloaded.
         */
        void ClearScene();

        /**
         * Returns a list of all components of the specified type currently in the scene.
         *
         * @tparam		T			Type of the component to search for.

         * @return					A list of all matching components in the scene.
         */
        template<class T>
        Vector<GameObjectHandle<T>> FindComponents();

        /** Returns all cameras in the scene. */
        const UnorderedMap<Camera*, SPtr<Camera>>& GetAllCameras() const { return _cameras; }

        /**
         * Returns the camera in the scene marked as main. Main camera controls the final render surface that is displayed
         * to the user. If there are multiple main cameras, the first one found returned.
         */
        SPtr<Camera> GetMainCamera() const;

        /**
         * Sets the render target that the main camera in the scene (if any) will render its view to. This generally means
         * the main game window when running standalone, or the Game viewport when running in editor.
         */
        void SetMainRenderTarget(const SPtr<RenderTarget>& rt);

        /** Changes the root scene object. Any persistent objects will remain in the scene, now parented to the new root. */
        void _setRootNode(const HSceneObject& root);

        /**
         * Binds a scene actor with a scene object. Every frame the scene object's transform will be monitored for
         * changes and those changes will be automatically transfered to the actor.
         */
        void _bindActor(const SPtr<SceneActor>& actor, const HSceneObject& so);

        /** Unbinds an actor that was previously bound using bindActor(). */
        void _unbindActor(const SPtr<SceneActor>& actor);

        /** Returns a scene object bound to the provided actor, if any. */
        HSceneObject _getActorSO(const SPtr<SceneActor>& actor) const;

        /** Notifies the scene manager that a new camera was created. */
        void _registerCamera(const SPtr<Camera>& camera);

        /** Notifies the scene manager that a camera was removed. */
        void _unregisterCamera(const SPtr<Camera>& camera);

        /**	Notifies the scene manager that a camera either became the main camera, or has stopped being main camera. */
        void _notifyMainCameraStateChanged(const SPtr<Camera>& camera);

        /** Called every frame. Calls update methods on all scene objects and their components. */
        void _update();

        /** Updates dirty transforms on any core objects that may be tied with scene objects. */
        void _updateCoreObjectTransforms();

        /** Notifies the manager that a new component has just been created. The manager triggers necessary callbacks. */
        void _notifyComponentCreated(const HComponent& component);

        /** Notifies the manager that a component is about to be destroyed. The manager triggers necessary callbacks. */
        void _notifyComponentDestroyed(const HComponent& component);

    protected:
        friend class SceneObject;

        /**
         * Register a new node in the scene manager, on the top-most level of the hierarchy.
         *
         * @param[in]	node	Node you wish to add. It's your responsibility not to add duplicate or null nodes. This
         *						method won't check.
         *
         * @note
         * After you add a node in the scene manager, it takes ownership of its memory and is responsible for releasing it.
         * Do NOT add nodes that have already been added (if you just want to change their parent). Normally this
         * method will only be called by SceneObject.
         */
        void RegisterNewSO(const HSceneObject& node);

        /**	Callback that is triggered when the main render target size is changed. */
        void OnMainRenderTargetResized();

        /** Checks does the specified component type match the provided id. */
        static bool IsComponentOfType(const HComponent& component, UINT32 id);

    protected:
        SPtr<SceneInstance> _mainScene;

        UnorderedMap<SceneActor*, BoundActorData> _boundActors;
        UnorderedMap<Camera*, SPtr<Camera>> _cameras;
        Vector<SPtr<Camera>> _mainCameras;

        Vector<HComponent> _components;

        SPtr<RenderTarget> _mainRenderTarget;
        HEvent _mainRTResizedConn;
    };

    template<class T>
    Vector<GameObjectHandle<T>> SceneManager::FindComponents()
    {
        UINT32 typeId = T::GetComponentType();

        Vector<GameObjectHandle<T>> output;
        for (auto& entry : _components)
        {
            if (IsComponentOfType(entry, typeId))
                output.push_back(static_object_cast<T>(entry));
        }

        return output;
    }

    /** Provides easy access to the SceneManager. */
    TE_CORE_EXPORT SceneManager& gSceneManager();
}